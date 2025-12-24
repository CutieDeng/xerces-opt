#include "virtual-call-analysis.hh"
#include "gcc-ext-util.hh"
#include "info-print.hh"

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
  
  // OBJ_TYPE_REF_EXPR 可能返回 SSA_NAME 而不是直接的 FUNCTION_DECL
  // 需要追踪 SSA_NAME 的定义
  tree method_decl_result = method;
  if (method && TREE_CODE (method) == SSA_NAME) {
    gimple * method_def = SSA_NAME_DEF_STMT (method);
    if (method_def && gimple_code (method_def) == GIMPLE_ASSIGN) {
      tree method_rhs = gimple_assign_rhs1 (method_def);
      if (method_rhs && TREE_CODE (method_rhs) == FUNCTION_DECL) {
        method_decl_result = method_rhs;
      } else if (method_rhs && TREE_CODE (method_rhs) == ADDR_EXPR) {
        tree addr_operand = TREE_OPERAND (method_rhs, 0);
        if (addr_operand && TREE_CODE (addr_operand) == FUNCTION_DECL) {
          method_decl_result = addr_operand;
        }
      } else if (method_rhs && TREE_CODE (method_rhs) == MEM_REF) {
        // mem_ref 表示从内存读取，可能是从虚表读取函数指针
        // 对于这种情况，我们无法直接获取函数名，但可以尝试从其他方式获取
        // 暂时返回 MATCH_ERROR，让调用者处理
        log_match_failure (AD_ARGS, call_expr);
        AD_RETURNE (MATCH_ERROR);
      }
    }
  }
  
  if (!method_decl_result || TREE_CODE (method_decl_result) != FUNCTION_DECL) {
    AD_DEBUG_PRINT ("[matchVirtualFunctionCall] method_decl unresolved (not FUNCTION_DECL); continuing and leaving method_decl NULL");
    if (ctx.debug_file && method) {
      AD_DEBUG_PRINT ("[matchVirtualFunctionCall] Dumping unresolved method expression for debugging:");
      print_generic_expr (ctx.debug_file, method, TDF_DETAILS);
      fprintf (ctx.debug_file, "\n");
    }
    method_decl = NULL_TREE;
  } else {
    method_decl = method_decl_result;
  }

  tree object = OBJ_TYPE_REF_OBJECT (call_expr);
  if (!object) {
    log_match_failure (AD_ARGS, call_expr);
      AD_DEBUG_PRINT ("[matchCallExpression] OBJ_TYPE_REF_OBJECT returned NULL");
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
      AD_DEBUG_PRINT ("[matchCallExpression] vtable_index is NULL for OBJ_TYPE_REF");
      if (ctx.debug_file) {
        AD_DEBUG_PRINT ("[matchCallExpression] Full OBJ_TYPE_REF dump:");
        print_generic_expr (ctx.debug_file, call_expr, TDF_DETAILS);
        fprintf (ctx.debug_file, "\n");
      }
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
      // Try to extract function name from OBJ_TYPE_REF/vtable info when method_decl is unresolved
      char const * extracted_name = NULL;
      if (extractVirtualCallFunctionName (AD_ARGS, call_stmt, extracted_name) == OK && extracted_name) {
        source_op.function_name = ggc_strdup (extracted_name);
        source_op.signature = ggc_strdup (extracted_name);
      } else {
        source_op.function_name = ggc_strdup ("<virtual>");
        source_op.signature = ggc_strdup ("<virtual>");
      }
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


// 比较两个函数声明的签名是否相同
ArrayDetectErrorCode compareFunctionSignatures (
  AD_FUNC_ARGS,
  tree decl1,
  tree decl2,
  bool &is_same_signature
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  is_same_signature = false;
  
  if (!decl1 || !decl2) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 必须都是函数声明
  if (TREE_CODE (decl1) != FUNCTION_DECL || TREE_CODE (decl2) != FUNCTION_DECL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 1. 比较函数名
  tree name1 = DECL_NAME (decl1);
  tree name2 = DECL_NAME (decl2);
  
  if (!name1 || !name2) {
    // 如果任一函数没有名称，无法比较
    AD_RETURNE (OK);
  }
  
  if (name1 != name2) {
    // 函数名不同，签名不同
    AD_RETURNE (OK);
  }
  
  // 2. 比较函数类型（包含返回类型和参数类型）
  tree type1 = TREE_TYPE (decl1);
  tree type2 = TREE_TYPE (decl2);
  
  if (!type1 || !type2) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 获取函数类型（METHOD_TYPE 或 FUNCTION_TYPE）
  if (TREE_CODE (type1) != TREE_CODE (type2)) {
    // 类型代码不同（如一个是 METHOD_TYPE，一个是 FUNCTION_TYPE）
    AD_RETURNE (OK);
  }
  
  // 3. 比较返回类型
  tree ret_type1 = TREE_TYPE (type1);
  tree ret_type2 = TREE_TYPE (type2);
  
  if (!ret_type1 || !ret_type2) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 获取主变体类型进行比较（去除 const/volatile 等修饰）
  ret_type1 = TYPE_MAIN_VARIANT (ret_type1);
  ret_type2 = TYPE_MAIN_VARIANT (ret_type2);
  
  if (ret_type1 != ret_type2) {
    // 返回类型不同
    AD_RETURNE (OK);
  }
  
  // 4. 比较参数类型
  tree args1 = TYPE_ARG_TYPES (type1);
  tree args2 = TYPE_ARG_TYPES (type2);
  
  // 遍历参数类型列表
  while (args1 && args2) {
    tree arg_type1 = TREE_VALUE (args1);
    tree arg_type2 = TREE_VALUE (args2);
    
    if (!arg_type1 || !arg_type2) {
      AD_RETURNE (INVALID_ARGUMENT);
    }
    
    // 获取主变体类型
    arg_type1 = TYPE_MAIN_VARIANT (arg_type1);
    arg_type2 = TYPE_MAIN_VARIANT (arg_type2);
    
    if (arg_type1 != arg_type2) {
      // 参数类型不同
      AD_RETURNE (OK);
    }
    
    args1 = TREE_CHAIN (args1);
    args2 = TREE_CHAIN (args2);
  }
  
  // 如果参数列表长度不同，签名不同
  if (args1 || args2) {
    AD_RETURNE (OK);
  }
  
  // 所有检查通过，签名相同
  is_same_signature = true;
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
    
    // OBJ_TYPE_REF_EXPR 可能返回 SSA_NAME 而不是直接的 FUNCTION_DECL
    // 需要追踪 SSA_NAME 的定义
    tree method_decl = method;
    if (method && TREE_CODE (method) == SSA_NAME) {
      gimple * method_def = SSA_NAME_DEF_STMT (method);
      if (method_def && gimple_code (method_def) == GIMPLE_ASSIGN) {
        tree method_rhs = gimple_assign_rhs1 (method_def);
        if (method_rhs && TREE_CODE (method_rhs) == FUNCTION_DECL) {
          method_decl = method_rhs;
        } else if (method_rhs && TREE_CODE (method_rhs) == ADDR_EXPR) {
          tree addr_operand = TREE_OPERAND (method_rhs, 0);
          if (addr_operand && TREE_CODE (addr_operand) == FUNCTION_DECL) {
            method_decl = addr_operand;
          }
        }
      }
    }
    
    if (!method_decl || TREE_CODE (method_decl) != FUNCTION_DECL) {
      AD_DEBUG_PRINT ("[matchCallExpression] method_decl unresolved; continuing as virtual call and leaving method_decl NULL");
      if (ctx.debug_file && method) {
        AD_DEBUG_PRINT ("[matchCallExpression] Dumping unresolved method expression for debugging:");
        print_generic_expr (ctx.debug_file, method, TDF_DETAILS);
        fprintf (ctx.debug_file, "\n");
      }
      result.info.virtual_.method_decl = NULL_TREE;
    } else {
      result.info.virtual_.method_decl = method_decl;
    }
    
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
