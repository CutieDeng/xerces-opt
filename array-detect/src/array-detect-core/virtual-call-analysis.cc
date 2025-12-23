#include "virtual-call-analysis.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

namespace {

// 匹配失败时的调试输出（受 ctx.match_debug_tracer 控制）
void log_match_failure (AD_FUNC_ARGS, tree call_expr) {
  if (!ctx.match_debug_tracer || !ctx.debug_file) {
    return;
  }

  AD_DEBUG_PRINT ("[virtual-call-analysis] matchVirtualFunctionCall failed");

  // 打印输入表达式
  if (call_expr) {
    AD_DEBUG_PRINT ("  call_expr tree (raw):");
    print_generic_expr (ctx.debug_file, call_expr, TDF_DETAILS);
    AD_DEBUG_PRINT ("  end call_expr tree");
  } else {
    AD_DEBUG_PRINT ("  call_expr tree: <null>");
  }

  // 打印源码位置与源码行
  location_t loc = UNKNOWN_LOCATION;
  if (call_expr && EXPR_P (call_expr)) {
    loc = EXPR_LOCATION (call_expr);
  }
  if (ctx.source_location_buffer && ctx.source_location_buffer_size > 0) {
    gcc_ext_util::get_source_location_string (AD_ARGS, loc,
      ctx.source_location_buffer, ctx.source_location_buffer_size);
    AD_DEBUG_PRINT ("  source location: %s", ctx.source_location_buffer);
  }
  if (ctx.source_line_buffer && ctx.source_line_buffer_size > 0) {
    gcc_ext_util::get_source_line_content (loc,
      ctx.source_line_buffer, ctx.source_line_buffer_size);
    AD_DEBUG_PRINT ("  source line: %s", ctx.source_line_buffer);
  }

  // 打印当前运行栈
  AD_GCC_DUMP_CALL_STACK ();
}

} // namespace

// 检查 gimple 语句是否为虚函数调用（使用 match-API 重构）
ArrayDetectErrorCode isVirtualFunctionCall (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  bool &is_virtual,
  CallType &call_type
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  is_virtual = false;
  call_type = CALL_UNKNOWN;
  
  if (!call_stmt || gimple_code (call_stmt) != GIMPLE_CALL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree fn = gimple_call_fn (call_stmt);
  if (!fn) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 使用 match-API 匹配调用表达式
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);
  
  if (match_ecode != OK) {
    // 匹配失败，保持默认值
    AD_RETURNE (OK);
  }
  
  call_type = match_result.call_type;
  is_virtual = (call_type == CALL_VIRTUAL);
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 匹配虚函数调用并提取细节（失败时直接返回 MATCH_ERROR）
ArrayDetectErrorCode matchVirtualFunctionCall (
  AD_FUNC_ARGS,
  tree call_expr,
  tree &object_type,
  tree &method_decl,
  tree &vtable_index
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;

  object_type = NULL_TREE;
  method_decl = NULL_TREE;
  vtable_index = NULL_TREE;

  if (!call_expr) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }

  // 允许 SSA_NAME/ADDR_EXPR 包裹，递归剥离 OBJ_TYPE_REF
  if (TREE_CODE (call_expr) == SSA_NAME) {
    gimple * def = SSA_NAME_DEF_STMT (call_expr);
    if (def && gimple_code (def) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def);
      return matchVirtualFunctionCall (AD_ARGS, rhs, object_type, method_decl, vtable_index);
    }
  }
  if (TREE_CODE (call_expr) == ADDR_EXPR) {
    tree inner = TREE_OPERAND (call_expr, 0);
    return matchVirtualFunctionCall (AD_ARGS, inner, object_type, method_decl, vtable_index);
  }

  if (TREE_CODE (call_expr) != OBJ_TYPE_REF) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }

  tree method = OBJ_TYPE_REF_EXPR (call_expr);
  if (!method || TREE_CODE (method) != FUNCTION_DECL) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }
  method_decl = method;

  tree object = OBJ_TYPE_REF_OBJECT (call_expr);
  if (!object) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }

  tree obj_type = TREE_TYPE (object);
  if (!obj_type) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }
  if (TREE_CODE (obj_type) == POINTER_TYPE || TREE_CODE (obj_type) == REFERENCE_TYPE) {
    obj_type = TREE_TYPE (obj_type);
  }
  if (obj_type) {
    obj_type = TYPE_MAIN_VARIANT (obj_type);
  }
  if (!obj_type) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }
  object_type = obj_type;

  // 虚表索引/偏移
#ifdef OBJ_TYPE_REF_TOKEN
  vtable_index = OBJ_TYPE_REF_TOKEN (call_expr);
#else
  vtable_index = NULL_TREE;
#endif
  if (!vtable_index) {
    log_match_failure (AD_ARGS, call_expr);
    AD_RETURNE (MATCH_ERROR);
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// 提取调用签名（用于等价性判定，使用 match-API 重构）
ArrayDetectErrorCode extractCallSignature (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  char const * &signature
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  signature = NULL;
  
  if (!call_stmt || gimple_code (call_stmt) != GIMPLE_CALL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree fn = gimple_call_fn (call_stmt);
  if (!fn) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 使用 match-API 匹配调用表达式
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);
  
  if (match_ecode != OK) {
    // 匹配失败，使用默认签名
    signature = ggc_strdup ("<unknown>");
    AD_RETURNE (OK);
  }
  
  // 根据匹配结果提取签名
  AD_MATCH_DIRECT_CALL (match_result, direct_info) {
    if (direct_info.function_decl && DECL_NAME (direct_info.function_decl)) {
      signature = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (direct_info.function_decl)));
    } else {
      signature = ggc_strdup ("<direct>");
    }
  } AD_MATCH_END ()
  
  AD_MATCH_VIRTUAL_CALL (match_result, virtual_info) {
    if (virtual_info.method_decl && DECL_NAME (virtual_info.method_decl)) {
      signature = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
    } else {
      signature = ggc_strdup ("<virtual>");
    }
  } AD_MATCH_END ()
  
  if (match_result.call_type == CALL_INDIRECT) {
    signature = ggc_strdup ("<indirect>");
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 分析函数调用，提取详细信息（使用 match-API 重构）
ArrayDetectErrorCode analyzeCallExpression (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  SourceOperation &source_op
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  memset (&source_op, 0, sizeof (SourceOperation));
  
  if (!call_stmt || gimple_code (call_stmt) != GIMPLE_CALL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  source_op.call_stmt = call_stmt;
  source_op.location = gimple_location (call_stmt);
  
  tree fn = gimple_call_fn (call_stmt);
  if (!fn) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 使用 match-API 匹配调用表达式
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);
  
  if (match_ecode != OK) {
    // 匹配失败，设置默认值
    source_op.call_type = CALL_UNKNOWN;
    source_op.function_name = ggc_strdup ("<unknown>");
    source_op.signature = ggc_strdup ("<unknown>");
    AD_RETURNE (OK);  // 仍然返回 OK，因为这是分析函数，匹配失败不算错误
  }
  
  source_op.call_type = match_result.call_type;
  
  // 根据匹配结果提取信息
  AD_MATCH_DIRECT_CALL (match_result, direct_info) {
    if (direct_info.function_decl && DECL_NAME (direct_info.function_decl)) {
      source_op.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (direct_info.function_decl)));
      source_op.signature = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (direct_info.function_decl)));
    } else {
      source_op.function_name = ggc_strdup ("<direct>");
      source_op.signature = ggc_strdup ("<direct>");
    }
  } AD_MATCH_END ()
  
  AD_MATCH_VIRTUAL_CALL (match_result, virtual_info) {
    if (virtual_info.method_decl && DECL_NAME (virtual_info.method_decl)) {
      source_op.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
      source_op.signature = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
    } else {
      source_op.function_name = ggc_strdup ("<virtual>");
      source_op.signature = ggc_strdup ("<virtual>");
    }
    source_op.vtable_ref = virtual_info.object;
  } AD_MATCH_END ()
  
  if (match_result.call_type == CALL_INDIRECT) {
    source_op.function_name = ggc_strdup ("<indirect>");
    source_op.signature = ggc_strdup ("<indirect>");
  }
  
  // 提取返回值 SSA
  source_op.return_value_ssa = gimple_call_lhs (call_stmt);
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 检查两个虚函数调用是否等价（使用 match-API 重构）
ArrayDetectErrorCode areVirtualCallsEquivalent (
  AD_FUNC_ARGS,
  gimple * call1,
  gimple * call2,
  bool &is_equivalent
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  is_equivalent = false;
  
  if (!call1 || !call2) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree fn1 = gimple_call_fn (call1);
  tree fn2 = gimple_call_fn (call2);
  
  if (!fn1 || !fn2) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 使用 match-API 匹配两个调用
  CallMatchResult match1, match2;
  ArrayDetectErrorCode ecode1 = matchCallExpression (AD_ARGS, fn1, match1);
  ArrayDetectErrorCode ecode2 = matchCallExpression (AD_ARGS, fn2, match2);
  
  // 如果任一匹配失败，则不等价
  if (ecode1 != OK || ecode2 != OK) {
    AD_RETURNE (OK);
  }
  
  // 类型必须相同
  if (match1.call_type != match2.call_type) {
    AD_RETURNE (OK);
  }
  
  // 根据类型比较
  if (match1.call_type == CALL_DIRECT && match2.call_type == CALL_DIRECT) {
    DirectCallInfo const &direct1 = match1.info.direct;
    DirectCallInfo const &direct2 = match2.info.direct;
    is_equivalent = (direct1.function_decl == direct2.function_decl);
  } else if (match1.call_type == CALL_VIRTUAL && match2.call_type == CALL_VIRTUAL) {
    VirtualCallInfo const &virtual1 = match1.info.virtual_;
    VirtualCallInfo const &virtual2 = match2.info.virtual_;
    // 比较方法声明
    is_equivalent = (virtual1.method_decl == virtual2.method_decl);
  } else if (match1.call_type == CALL_INDIRECT && match2.call_type == CALL_INDIRECT) {
    IndirectCallInfo const &indirect1 = match1.info.indirect;
    IndirectCallInfo const &indirect2 = match2.info.indirect;
    // 间接调用比较表达式
    is_equivalent = (indirect1.function_expr == indirect2.function_expr);
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 函数调用匹配与解构实现
// ============================================================================

// 匹配函数调用表达式
ArrayDetectErrorCode matchCallExpression (
  AD_FUNC_ARGS,
  tree call_fn_expr,
  CallMatchResult &result
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  // 初始化结果
  memset (&result, 0, sizeof (CallMatchResult));
  result.call_type = CALL_UNKNOWN;
  
  // 检查输入有效性
  if (!call_fn_expr) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree fn = call_fn_expr;
  
  // 处理 OBJ_TYPE_REF（虚函数调用）
  if (TREE_CODE (fn) == OBJ_TYPE_REF) {
    result.call_type = CALL_VIRTUAL;
    
    // 提取方法声明
    tree method = OBJ_TYPE_REF_EXPR (fn);
    if (!method || TREE_CODE (method) != FUNCTION_DECL) {
      AD_RETURNE (MATCH_ERROR);  // 匹配失败：无效的方法声明
    }
    result.info.virtual_.method_decl = method;
    
    // 提取对象表达式（用于访问虚表，可能是局部变量 SSA_NAME）
    tree object = OBJ_TYPE_REF_OBJECT (fn);
    if (!object) {
      AD_RETURNE (MATCH_ERROR);  // 匹配失败：无效的对象表达式
    }
    result.info.virtual_.object = object;
    
    // 提取对象类型（需要小心处理局部变量）
    tree object_type = NULL_TREE;
    
    // 如果对象是 SSA_NAME（局部变量），需要获取其类型
    if (TREE_CODE (object) == SSA_NAME) {
      object_type = TREE_TYPE (object);
    } else {
      // 其他情况直接获取类型
      object_type = TREE_TYPE (object);
    }
    
    if (!object_type) {
      AD_RETURNE (MATCH_ERROR);  // 匹配失败：无法获取对象类型
    }
    
    // 处理指针和引用类型，获取实际的对象类型
    tree actual_object_type = object_type;
    if (TREE_CODE (actual_object_type) == POINTER_TYPE) {
      actual_object_type = TREE_TYPE (actual_object_type);
    } else if (TREE_CODE (actual_object_type) == REFERENCE_TYPE) {
      actual_object_type = TREE_TYPE (actual_object_type);
    }
    
    // 获取主变体类型（去除 const/volatile 等修饰）
    if (actual_object_type) {
      actual_object_type = TYPE_MAIN_VARIANT (actual_object_type);
    }
    
    result.info.virtual_.object_type = actual_object_type ? actual_object_type : object_type;
    
    // 提取虚表类型
    // obj_type_ref_class () 返回的是类的类型，用于确定虚表
    // 如果函数不存在，则从对象类型推导
    tree class_type = NULL_TREE;
    
    // 尝试使用 GCC 的 obj_type_ref_class () 函数
    // 注意：这个函数可能在某些 GCC 版本中不存在，需要检查
    // 如果不存在，我们从对象类型推导
    if (result.info.virtual_.object_type) {
      class_type = result.info.virtual_.object_type;
    }
    
    // 如果对象类型是 RECORD_TYPE 或 UNION_TYPE，直接使用
    // 否则尝试从对象表达式的类型推导
    if (!class_type || (TREE_CODE (class_type) != RECORD_TYPE && TREE_CODE (class_type) != UNION_TYPE)) {
      // 从对象表达式推导类型
      tree obj = result.info.virtual_.object;
      if (obj) {
        tree obj_type = TREE_TYPE (obj);
        if (obj_type) {
          // 处理指针和引用类型
          if (TREE_CODE (obj_type) == POINTER_TYPE) {
            obj_type = TREE_TYPE (obj_type);
          } else if (TREE_CODE (obj_type) == REFERENCE_TYPE) {
            obj_type = TREE_TYPE (obj_type);
          }
          if (obj_type) {
            obj_type = TYPE_MAIN_VARIANT (obj_type);
            if (TREE_CODE (obj_type) == RECORD_TYPE || TREE_CODE (obj_type) == UNION_TYPE) {
              class_type = obj_type;
            }
          }
        }
      }
    }
    
    if (class_type) {
      // 获取主变体类型
      tree main_variant = TYPE_MAIN_VARIANT (class_type);
      result.info.virtual_.vtable_type = main_variant ? main_variant : class_type;
    } else {
      // 如果无法获取，使用对象类型
      result.info.virtual_.vtable_type = result.info.virtual_.object_type;
    }
    
    AD_RETURNE (OK);
  }
  
  // 处理 SSA_NAME（可能是间接调用或虚函数调用的间接形式）
  if (TREE_CODE (fn) == SSA_NAME) {
    // 检查定义语句，看是否来自 OBJ_TYPE_REF
    gimple * def_stmt = SSA_NAME_DEF_STMT (fn);
    if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (TREE_CODE (rhs) == OBJ_TYPE_REF) {
        // 递归处理 OBJ_TYPE_REF
        return matchCallExpression (AD_ARGS, rhs, result);
      }
    }
    
    // 否则是间接调用
    result.call_type = CALL_INDIRECT;
    result.info.indirect.function_expr = fn;
    AD_RETURNE (OK);
  }
  
  // 处理直接函数调用
  if (TREE_CODE (fn) == FUNCTION_DECL) {
    result.call_type = CALL_DIRECT;
    result.info.direct.function_decl = fn;
    AD_RETURNE (OK);
  }
  
  // 处理 ADDR_EXPR（函数地址）
  if (TREE_CODE (fn) == ADDR_EXPR) {
    tree addr_expr = TREE_OPERAND (fn, 0);
    if (addr_expr && TREE_CODE (addr_expr) == FUNCTION_DECL) {
      result.call_type = CALL_DIRECT;
      result.info.direct.function_decl = addr_expr;
      AD_RETURNE (OK);
    }
  }
  
  // 匹配失败
  AD_RETURNE (MATCH_ERROR);
} AD_FUNCTION_END

} // namespace array_detect_ns
