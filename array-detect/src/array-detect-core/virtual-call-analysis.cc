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

} // namespace array_detect_ns
