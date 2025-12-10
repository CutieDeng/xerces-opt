#include "field-write-analysis.hh"
#include "gcc-ext-util.hh"
#include "array-detector.hh"

namespace array_detect_ns {

// 辅助函数：在平行 vec 中查找字段索引
static int findFieldIndex(vec<tree> const &field_decls, tree field_decl) {
  for (unsigned int i = 0; i < field_decls.length(); i++) {
    if (field_decls[i] == field_decl) {
      return (int)i;
    }
  }
  return -1;
}

// 分析函数中的字段写入操作
ArrayDetectErrorCode analyzeFunctionFieldWrites(
  AD_FUNC_ARGS,
  function* fn,
  const char* function_name,
  tree function_decl,
  vec<tree> &field_decls,
  vec<vec<WriteOperation*>*> &field_writes
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  if (!fn || !function_name || !function_decl) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("Analyzing field writes in function: %s", function_name);
  
  // 初始化输出向量
  field_decls.create(0);
  field_writes.create(0);
  
  // 遍历函数的所有基本块
  basic_block bb;
  FOR_EACH_BB_FN(bb, fn) {
    gimple_stmt_iterator gsi;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
      gimple* stmt = gsi_stmt(gsi);
      
      // 只处理赋值语句
      if (gimple_code(stmt) != GIMPLE_ASSIGN) {
        continue;
      }
      
      tree lhs = gimple_assign_lhs(stmt);
      tree rhs = gimple_assign_rhs1(stmt);
      
      // 检查左值是否为字段访问
      tree field_decl = NULL_TREE;
      tree object = NULL_TREE;
      bool is_field_access0;
      AD_TRY(gcc_ext_util::is_field_access(AD_ARGS, lhs, &field_decl, &object, is_field_access0));
      
      if (!is_field_access0 || !field_decl) {
        continue;
      }
      
      // 查找或创建该字段的写入操作列表
      int field_index = findFieldIndex(field_decls, field_decl);
      vec<WriteOperation*>* write_list = NULL;
      
      if (field_index < 0) {
        // 新字段，添加到列表
        field_decls.safe_push(field_decl);
        write_list = ggc_alloc<vec<WriteOperation*>>();
        write_list->create(0);
        field_writes.safe_push(write_list);
        field_index = (int)(field_decls.length() - 1);
      } else {
        write_list = field_writes[field_index];
      }
      
      // 创建写入操作记录
      WriteOperation* write_op = ggc_alloc<WriteOperation>();
      memset(write_op, 0, sizeof(WriteOperation));
      
      write_op->stmt = stmt;
      write_op->rhs_value = rhs;
      write_op->function_name = ggc_strdup(function_name);
      write_op->function_decl = function_decl;
      write_op->location = gimple_location(stmt);
      
      // 分析右值来源
      enum tree_code rhs_code = gimple_assign_rhs_code(stmt);
      bool is_call = false;
      tree call_expr = NULL_TREE;
      const char* rhs_desc = "";
      
      if (rhs_code == CALL_EXPR) {
        is_call = true;
        call_expr = rhs;
        rhs_desc = "function call";
      } else if (TREE_CODE(rhs) == SSA_NAME) {
        // 追踪 SSA_NAME 的定义
        gimple* def_stmt = SSA_NAME_DEF_STMT(rhs);
        if (def_stmt && is_gimple_call(def_stmt)) {
          is_call = true;
          call_expr = gimple_call_fn(def_stmt);
          rhs_desc = "call result";
        } else {
          rhs_desc = "variable";
        }
      } else if (rhs_code == INTEGER_CST) {
        rhs_desc = "constant";
      } else {
        rhs_desc = "expression";
      }
      
      write_op->is_from_call = is_call;
      write_op->call_stmt = NULL;  // 如果是调用，需要找到实际的 call_stmt
      // 注意：这里简化处理，call_stmt 需要从 def_stmt 中获取
      if (is_call && TREE_CODE(rhs) == SSA_NAME) {
        gimple* def_stmt = SSA_NAME_DEF_STMT(rhs);
        if (def_stmt && gimple_code(def_stmt) == GIMPLE_CALL) {
          write_op->call_stmt = def_stmt;
        }
      }
      write_op->rhs_description = ggc_strdup(rhs_desc);
      
      // 添加到列表
      write_list->safe_push(write_op);
      
      AD_DEBUG_PRINT("  Found write operation for field: %s", rhs_desc);
    }
  }
  
  AD_DEBUG_PRINT("  Total fields with writes: %u", field_decls.length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
