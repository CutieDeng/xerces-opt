#include "field-escape-analysis.hh"
#include "gcc-ext-util.hh"
#include "gcc-common.hh"

namespace array_detect_ns {

// 辅助函数：检查语句是否使用了指定的 SSA_NAME
static bool stmt_uses_ssa (gimple * stmt, tree ssa_name) {
  if (!stmt || TREE_CODE (ssa_name) != SSA_NAME) {
    return false;
  }
  
  // 检查赋值语句的右值
  if (gimple_code (stmt) == GIMPLE_ASSIGN) {
    tree rhs1 = gimple_assign_rhs1 (stmt);
    tree rhs2 = gimple_assign_rhs2 (stmt);
    tree rhs3 = gimple_assign_rhs3 (stmt);
    
    if (rhs1 == ssa_name || rhs2 == ssa_name || rhs3 == ssa_name) {
      return true;
    }
  }
  
  // 检查函数调用参数
  if (gimple_code (stmt) == GIMPLE_CALL) {
    unsigned int num_args = gimple_call_num_args (stmt);
    for (unsigned int i = 0; i < num_args; i++) {
      tree arg = gimple_call_arg (stmt, i);
      if (arg == ssa_name) {
        return true;
      }
    }
  }
  
  // 检查返回语句
  if (gimple_code (stmt) == GIMPLE_RETURN) {
    greturn * ret_stmt = as_a<greturn*>(stmt);
    tree ret_val = gimple_return_retval (ret_stmt);
    if (ret_val == ssa_name) {
      return true;
    }
  }
  
  return false;
}

// 追踪单个 SSA_NAME 的所有使用位置
// 注意：使用遍历所有语句的方法，因为 ssa-iterators.h 在插件环境中不可用
ArrayDetectErrorCode trackSSAUses (
  AD_FUNC_ARGS,
  tree ssa_name,
  tree target_field,
  vec<EscapeSite*> &escape_sites
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  if (TREE_CODE (ssa_name) != SSA_NAME) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 使用 hash_set 避免重复访问
  hash_set<tree> visited;
  visited.create_ggc (0);
  
  // DFS 追踪所有使用
  vec<tree> worklist;
  worklist.create (0);
  worklist.safe_push (ssa_name);
  
  while (!worklist.is_empty ()) {
    tree current = worklist.pop ();
    
    if (visited.contains (current)) {
      continue;
    }
    visited.add (current);
    
    // 遍历所有函数查找使用该 SSA_NAME 的语句
    struct cgraph_node * node;
    FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
      function * fn = node->get_fun ();
      if (!fn) {
        continue;
      }
      
      basic_block bb;
      FOR_EACH_BB_FN (bb, fn) {
        gimple_stmt_iterator gsi;
        for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
          gimple * use_stmt = gsi_stmt (gsi);
          if (!use_stmt) {
            continue;
          }
          
          // 检查语句是否使用了当前的 SSA_NAME
          if (!stmt_uses_ssa (use_stmt, current)) {
            continue;
          }
          
          enum gimple_code code = gimple_code (use_stmt);
          
          // 检查使用类型
          if (code == GIMPLE_ASSIGN) {
            tree lhs = gimple_assign_lhs (use_stmt);
            
            // 检查是否写入目标字段
            tree field_decl = NULL_TREE;
            tree object = NULL_TREE;
            bool is_field_access0;
            AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));
            
            if (is_field_access0 && field_decl == target_field) {
              // 写入目标字段，不是逃逸
              continue;
            } else {
              // 写入其他位置，记录逃逸
              EscapeSite * escape = ggc_alloc<EscapeSite>();
              memset (escape, 0, sizeof (EscapeSite));
              escape->stmt = use_stmt;
              escape->escape_type = ESCAPE_WRITE;
              escape->location = gimple_location (use_stmt);
              escape->function_name = ggc_strdup ("<function>");
              escape->description = ggc_strdup ("written to other location");
              
              escape_sites.safe_push (escape);
              
              // 继续追踪写入的变量
              if (TREE_CODE (lhs) == SSA_NAME && !visited.contains (lhs)) {
                worklist.safe_push (lhs);
              }
            }
          } else if (code == GIMPLE_CALL) {
            // 作为函数参数传递，记录逃逸
            EscapeSite * escape = ggc_alloc<EscapeSite>();
            memset (escape, 0, sizeof (EscapeSite));
            escape->stmt = use_stmt;
            escape->escape_type = ESCAPE_ARGUMENT;
            escape->location = gimple_location (use_stmt);
            escape->function_name = ggc_strdup ("<call>");
            escape->description = ggc_strdup ("passed as function argument");
            
            escape_sites.safe_push (escape);
          } else if (code == GIMPLE_RETURN) {
            // 作为返回值，记录逃逸
            EscapeSite * escape = ggc_alloc<EscapeSite>();
            memset (escape, 0, sizeof (EscapeSite));
            escape->stmt = use_stmt;
            escape->escape_type = ESCAPE_RETURN;
            escape->location = gimple_location (use_stmt);
            escape->function_name = ggc_strdup ("<return>");
            escape->description = ggc_strdup ("returned from function");
            
            escape_sites.safe_push (escape);
          } else {
            // 参与其他计算，记录逃逸
            EscapeSite * escape = ggc_alloc<EscapeSite>();
            memset (escape, 0, sizeof (EscapeSite));
            escape->stmt = use_stmt;
            escape->escape_type = ESCAPE_COMPUTE;
            escape->location = gimple_location (use_stmt);
            escape->function_name = ggc_strdup ("<compute>");
            escape->description = ggc_strdup ("used in computation");
            
            escape_sites.safe_push (escape);
          }
        }
      }
    }
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 分析字段源变量的逃逸情况
ArrayDetectErrorCode analyzeFieldEscapes (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<vec<tree>*> const &field_sources,
  unsigned int target_field_index,
  vec<EscapeSite*> &escape_sites
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT ("Analyzing escapes for field at index %u", target_field_index);
  
  if (target_field_index >= field_decls.length () || 
      target_field_index >= field_sources.length ()) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree target_field = field_decls[target_field_index];
  vec<tree>* source_vars = field_sources[target_field_index];
  
  if (!source_vars) {
    AD_RETURNE (OK);
  }
  
  // 初始化逃逸列表
  escape_sites.create (0);
  
  // 追踪每个源变量的使用
  for (unsigned int i = 0; i < source_vars->length (); i++) {
    tree source_var = (*source_vars)[i];
    if (TREE_CODE (source_var) != SSA_NAME) {
      continue;
    }
    
    AD_TRY (trackSSAUses (AD_ARGS, source_var, target_field, escape_sites));
  }
  
  AD_DEBUG_PRINT ("  Found %u escape sites", escape_sites.length ());
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
