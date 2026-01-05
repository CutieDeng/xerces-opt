#include "field-owner-analysis.hh"
#include "virtual-call-analysis.hh"
#include "virtual-call-analysis.hh"

namespace array_detect_ns {

// 从源变量中提取源操作（函数调用）
ArrayDetectErrorCode extractSourceOperations (
  AD_FUNC_ARGS,
  vec<tree> const &source_vars,
  vec<SourceOperation*> &source_ops
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT ("Extracting source operations from source variables");
  
  for (unsigned int i = 0; i < source_vars.length (); i++) {
    tree source_var = source_vars[i];
    if (TREE_CODE (source_var) != SSA_NAME) {
      continue;
    }
    
    // 获取定义语句
    gimple * def_stmt = SSA_NAME_DEF_STMT (source_var);
    if (!def_stmt) {
      continue;
    }
    
    // 检查是否为函数调用
    gimple * call_stmt = NULL;
    if (gimple_code (def_stmt) == GIMPLE_CALL) {
      call_stmt = def_stmt;
    } else if (gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (TREE_CODE (rhs) == SSA_NAME) {
        // 继续追踪
        gimple * rhs_def = SSA_NAME_DEF_STMT (rhs);
        if (rhs_def && gimple_code (rhs_def) == GIMPLE_CALL) {
          call_stmt = rhs_def;
        }
      }
    }
    
    if (!call_stmt) {
      continue;
    }
    
    // 分析调用表达式
    SourceOperation * source_op = ggc_alloc<SourceOperation>();
    memset (source_op, 0, sizeof (SourceOperation));
    
    // analyzeCallExpression 现在接受 gimple * 而不是 tree
    AD_TRY (analyzeCallExpression (AD_ARGS, call_stmt, *source_op));
    source_op->return_value_ssa = source_var;
    source_ops.safe_push (source_op);
  }
  
  AD_DEBUG_PRINT ("  Extracted %u source operations", source_ops.length ());
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 判定字段是否为内存持有者
ArrayDetectErrorCode determineMemoryOwner (
  AD_FUNC_ARGS,
  tree field_decl,
  vec<FieldWriteCapture*> const &write_ops,
  vec<EscapeSite*> const &escape_sites,
  vec<SourceOperation*> const &source_ops,
  FieldAnalysisResult &result
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT ("Determining memory owner status for field");
  
  // 初始化结果
  memset (&result, 0, sizeof (FieldAnalysisResult));
  result.field_decl = field_decl;
  result.is_memory_owner = false;
  
  // 条件1：检查是否有写入操作
  if (write_ops.length () == 0) {
    result.reason = ggc_strdup ("no write operations found");
    AD_RETURNE (OK);
  }
  
  // 条件2：检查是否有逃逸
  if (escape_sites.length () > 0) {
    result.reason = ggc_strdup ("value escapes to other locations");
    AD_RETURNE (OK);
  }
  
  // 条件3：检查源操作
  if (source_ops.length () == 0) {
    result.reason = ggc_strdup ("no source operations found");
    AD_RETURNE (OK);
  }
  
  // 条件4：检查源操作是否唯一
  if (source_ops.length () > 1) {
    // 检查所有源操作是否等价（都是同一个虚函数调用）
    bool all_equivalent = true;
    SourceOperation * base_op = source_ops[0];
    
    if (base_op->call_type != CALL_VIRTUAL) {
      all_equivalent = false;
    } else {
      for (unsigned int i = 1; i < source_ops.length (); i++) {
        SourceOperation * op = source_ops[i];
        if (op->call_type != CALL_VIRTUAL) {
          all_equivalent = false;
          break;
        }
        
        // 比较签名
        if (!base_op->signature || !op->signature ||
            strcmp (base_op->signature, op->signature) != 0) {
          all_equivalent = false;
          break;
        }
        
        // 注意：更精确的比较需要使用 FunctionCallSource（write-operation-trace 模块的二级信息）
        // 这里只比较签名字符串，如果需要更精确的比较，应该使用 function-call-comparator 模块
      }
    }
    
    if (!all_equivalent) {
      result.reason = ggc_strdup ("multiple different source operations");
      AD_RETURNE (OK);
    }
  }
  
  // 条件5：检查是否为虚函数调用
  SourceOperation * source_op = source_ops[0];
  if (source_op->call_type != CALL_VIRTUAL) {
    result.reason = ggc_strdup ("source operation is not a virtual function call");
    AD_DEBUG_PRINT ("  Field is not memory owner: %s", result.reason);
    AD_RETURNE (OK);
  }
  
  // 所有条件满足
  result.is_memory_owner = true;
  result.reason = ggc_strdup ("all conditions satisfied: single virtual call, no escape");
  
  AD_DEBUG_PRINT ("  Field is memory owner: %s", result.reason);
  AD_DEBUG_PRINT ("    Write operations: %u", write_ops.length ());
  AD_DEBUG_PRINT ("    Escape sites: %u", escape_sites.length ());
  AD_DEBUG_PRINT ("    Source operations: %u", source_ops.length ());
  if (source_op->signature) {
    AD_DEBUG_PRINT ("    Call signature: %s", source_op->signature);
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
