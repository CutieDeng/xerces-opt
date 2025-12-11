#include "virtual-call-analysis.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// 检查 gimple 语句是否为虚函数调用
ArrayDetectErrorCode isVirtualFunctionCall(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  bool &is_virtual,
  CallType &call_type
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  is_virtual = false;
  call_type = CALL_UNKNOWN;
  
  if (!call_stmt || gimple_code(call_stmt) != GIMPLE_CALL) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  tree fn = gimple_call_fn(call_stmt);
  if (!fn) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 检查是否为 OBJ_TYPE_REF（C++ 虚函数调用）
  if (TREE_CODE(fn) == OBJ_TYPE_REF) {
    is_virtual = true;
    call_type = CALL_VIRTUAL;
    AD_RETURNE(OK);
  }
  
  // 检查是否为 SSA_NAME（可能是间接调用）
  if (TREE_CODE(fn) == SSA_NAME) {
    gimple* def_stmt = SSA_NAME_DEF_STMT(fn);
    if (def_stmt && gimple_code(def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1(def_stmt);
      if (TREE_CODE(rhs) == OBJ_TYPE_REF) {
        is_virtual = true;
        call_type = CALL_VIRTUAL;
        AD_RETURNE(OK);
      }
    }
    call_type = CALL_INDIRECT;
    AD_RETURNE(OK);
  }
  
  // 直接函数调用
  if (TREE_CODE(fn) == FUNCTION_DECL || TREE_CODE(fn) == ADDR_EXPR) {
    call_type = CALL_DIRECT;
    AD_RETURNE(OK);
  }
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 提取调用签名（用于等价性判定）
ArrayDetectErrorCode extractCallSignature(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  const char* &signature
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  signature = NULL;
  
  if (!call_stmt || gimple_code(call_stmt) != GIMPLE_CALL) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  tree fn = gimple_call_fn(call_stmt);
  if (!fn) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 对于虚函数调用，提取方法签名
  if (TREE_CODE(fn) == OBJ_TYPE_REF) {
    tree method = OBJ_TYPE_REF_EXPR(fn);
    if (method && DECL_NAME(method)) {
      signature = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(method)));
      AD_RETURNE(OK);
    }
  }
  
  // 对于直接调用，提取函数名
  if (TREE_CODE(fn) == FUNCTION_DECL && DECL_NAME(fn)) {
    signature = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(fn)));
    AD_RETURNE(OK);
  }
  
  // 对于间接调用，使用特殊标记
  signature = ggc_strdup("<indirect>");
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 分析函数调用，提取详细信息
ArrayDetectErrorCode analyzeCallExpression(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  SourceOperation &source_op
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  memset(&source_op, 0, sizeof(SourceOperation));
  
  if (!call_stmt || gimple_code(call_stmt) != GIMPLE_CALL) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  source_op.call_stmt = call_stmt;
  source_op.location = gimple_location(call_stmt);
  
  tree fn = gimple_call_fn(call_stmt);
  
  // 检查调用类型
  bool is_virtual;
  CallType call_type;
  AD_TRY(isVirtualFunctionCall(AD_ARGS, call_stmt, is_virtual, call_type));
  source_op.call_type = call_type;
  
  // 提取调用签名
  const char* signature;
  AD_TRY(extractCallSignature(AD_ARGS, call_stmt, signature));
  source_op.signature = signature;
  
  // 提取函数名
  if (TREE_CODE(fn) == OBJ_TYPE_REF) {
    tree method = OBJ_TYPE_REF_EXPR(fn);
    if (method && DECL_NAME(method)) {
      source_op.function_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(method)));
    } else {
      source_op.function_name = ggc_strdup("<virtual>");
    }
    source_op.vtable_ref = OBJ_TYPE_REF_OBJECT(fn);
  } else if (TREE_CODE(fn) == FUNCTION_DECL && DECL_NAME(fn)) {
    source_op.function_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(fn)));
  } else {
    source_op.function_name = ggc_strdup("<unknown>");
  }
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 检查两个虚函数调用是否等价
ArrayDetectErrorCode areVirtualCallsEquivalent(
  AD_FUNC_ARGS,
  gimple* call1,
  gimple* call2,
  bool &is_equivalent
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  is_equivalent = false;
  
  if (!call1 || !call2) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 提取两个调用的签名
  const char* sig1;
  const char* sig2;
  AD_TRY(extractCallSignature(AD_ARGS, call1, sig1));
  AD_TRY(extractCallSignature(AD_ARGS, call2, sig2));
  
  // 比较签名
  if (sig1 && sig2 && strcmp(sig1, sig2) == 0) {
    is_equivalent = true;
  }
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// 函数调用匹配与解构实现
// ============================================================================

// 匹配函数调用表达式
ArrayDetectErrorCode matchCallExpression(
  AD_FUNC_ARGS,
  tree call_fn_expr,
  CallMatchResult &result
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  // 初始化结果
  memset(&result, 0, sizeof(CallMatchResult));
  result.call_type = CALL_UNKNOWN;
  
  // 检查输入有效性
  if (!call_fn_expr) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  tree fn = call_fn_expr;
  
  // 处理 OBJ_TYPE_REF（虚函数调用）
  if (TREE_CODE(fn) == OBJ_TYPE_REF) {
    result.call_type = CALL_VIRTUAL;
    
    // 提取方法声明
    tree method = OBJ_TYPE_REF_EXPR(fn);
    if (!method || TREE_CODE(method) != FUNCTION_DECL) {
      AD_RETURNE(MATCH_ERROR);  // 匹配失败：无效的方法声明
    }
    result.info.virtual_.method_decl = method;
    
    // 提取对象表达式（用于访问虚表，可能是局部变量 SSA_NAME）
    tree object = OBJ_TYPE_REF_OBJECT(fn);
    if (!object) {
      AD_RETURNE(MATCH_ERROR);  // 匹配失败：无效的对象表达式
    }
    result.info.virtual_.object = object;
    
    // 提取对象类型（需要小心处理局部变量）
    tree object_type = NULL_TREE;
    
    // 如果对象是 SSA_NAME（局部变量），需要获取其类型
    if (TREE_CODE(object) == SSA_NAME) {
      object_type = TREE_TYPE(object);
    } else {
      // 其他情况直接获取类型
      object_type = TREE_TYPE(object);
    }
    
    if (!object_type) {
      AD_RETURNE(MATCH_ERROR);  // 匹配失败：无法获取对象类型
    }
    
    // 处理指针和引用类型，获取实际的对象类型
    tree actual_object_type = object_type;
    if (TREE_CODE(actual_object_type) == POINTER_TYPE) {
      actual_object_type = TREE_TYPE(actual_object_type);
    } else if (TREE_CODE(actual_object_type) == REFERENCE_TYPE) {
      actual_object_type = TREE_TYPE(actual_object_type);
    }
    
    // 获取主变体类型（去除 const/volatile 等修饰）
    if (actual_object_type) {
      actual_object_type = TYPE_MAIN_VARIANT(actual_object_type);
    }
    
    result.info.virtual_.object_type = actual_object_type ? actual_object_type : object_type;
    
    // 提取虚表类型
    // obj_type_ref_class() 返回的是类的类型，用于确定虚表
    // 如果函数不存在，则从对象类型推导
    tree class_type = NULL_TREE;
    
    // 尝试使用 GCC 的 obj_type_ref_class() 函数
    // 注意：这个函数可能在某些 GCC 版本中不存在，需要检查
    // 如果不存在，我们从对象类型推导
    if (result.info.virtual_.object_type) {
      class_type = result.info.virtual_.object_type;
    }
    
    // 如果对象类型是 RECORD_TYPE 或 UNION_TYPE，直接使用
    // 否则尝试从对象表达式的类型推导
    if (!class_type || (TREE_CODE(class_type) != RECORD_TYPE && TREE_CODE(class_type) != UNION_TYPE)) {
      // 从对象表达式推导类型
      tree obj = result.info.virtual_.object;
      if (obj) {
        tree obj_type = TREE_TYPE(obj);
        if (obj_type) {
          // 处理指针和引用类型
          if (TREE_CODE(obj_type) == POINTER_TYPE) {
            obj_type = TREE_TYPE(obj_type);
          } else if (TREE_CODE(obj_type) == REFERENCE_TYPE) {
            obj_type = TREE_TYPE(obj_type);
          }
          if (obj_type) {
            obj_type = TYPE_MAIN_VARIANT(obj_type);
            if (TREE_CODE(obj_type) == RECORD_TYPE || TREE_CODE(obj_type) == UNION_TYPE) {
              class_type = obj_type;
            }
          }
        }
      }
    }
    
    if (class_type) {
      // 获取主变体类型
      tree main_variant = TYPE_MAIN_VARIANT(class_type);
      result.info.virtual_.vtable_type = main_variant ? main_variant : class_type;
    } else {
      // 如果无法获取，使用对象类型
      result.info.virtual_.vtable_type = result.info.virtual_.object_type;
    }
    
    AD_RETURNE(OK);
  }
  
  // 处理 SSA_NAME（可能是间接调用或虚函数调用的间接形式）
  if (TREE_CODE(fn) == SSA_NAME) {
    // 检查定义语句，看是否来自 OBJ_TYPE_REF
    gimple* def_stmt = SSA_NAME_DEF_STMT(fn);
    if (def_stmt && gimple_code(def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1(def_stmt);
      if (TREE_CODE(rhs) == OBJ_TYPE_REF) {
        // 递归处理 OBJ_TYPE_REF
        return matchCallExpression(AD_ARGS, rhs, result);
      }
    }
    
    // 否则是间接调用
    result.call_type = CALL_INDIRECT;
    result.info.indirect.function_expr = fn;
    AD_RETURNE(OK);
  }
  
  // 处理直接函数调用
  if (TREE_CODE(fn) == FUNCTION_DECL) {
    result.call_type = CALL_DIRECT;
    result.info.direct.function_decl = fn;
    AD_RETURNE(OK);
  }
  
  // 处理 ADDR_EXPR（函数地址）
  if (TREE_CODE(fn) == ADDR_EXPR) {
    tree addr_expr = TREE_OPERAND(fn, 0);
    if (addr_expr && TREE_CODE(addr_expr) == FUNCTION_DECL) {
      result.call_type = CALL_DIRECT;
      result.info.direct.function_decl = addr_expr;
      AD_RETURNE(OK);
    }
  }
  
  // 匹配失败
  AD_RETURNE(MATCH_ERROR);
} AD_FUNCTION_END

} // namespace array_detect_ns
