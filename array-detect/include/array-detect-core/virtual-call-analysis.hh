#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detect_ns {

// ============================================================================
// 虚函数调用分析
// ============================================================================
// 识别和分析 GIMPLE 中的虚函数调用
// ============================================================================

// 检查是否为虚函数调用
// 返回值：ArrayDetectErrorCode
// 输入：call_stmt - GIMPLE_CALL 语句
// 输出：is_virtual - 是否为虚函数调用
// 输出：call_type - 调用类型
ArrayDetectErrorCode isVirtualFunctionCall(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  bool &is_virtual,
  CallType &call_type
);

// 分析函数调用，提取详细信息
// 返回值：ArrayDetectErrorCode
// 输入：call_stmt - GIMPLE_CALL 语句
// 输出：source_op - 源操作信息
ArrayDetectErrorCode analyzeCallExpression(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  SourceOperation &source_op
);

// 提取调用签名（用于等价性判定）
// 返回值：ArrayDetectErrorCode
// 输入：call_stmt - GIMPLE_CALL 语句
// 输出：signature - 调用签名字符串
ArrayDetectErrorCode extractCallSignature(
  AD_FUNC_ARGS,
  gimple* call_stmt,
  const char* &signature
);

// 匹配虚函数调用并提取细节（直接返回 MATCH_ERROR 代表匹配失败）
// 输入：call_expr - OBJ_TYPE_REF 或可解构出虚调用的表达式
// 输出：object_type - 对象类型（去除引用/指针后的主变体）
// 输出：method_decl - 虚函数方法声明（FUNCTION_DECL）
// 输出：vtable_index - 虚表偏移量/标识（来自 OBJ_TYPE_REF_TOKEN）
ArrayDetectErrorCode matchVirtualFunctionCall(
  AD_FUNC_ARGS,
  tree call_expr,
  tree &object_type,
  tree &method_decl,
  tree &vtable_index
);

// 检查两个虚函数调用是否等价
// 返回值：ArrayDetectErrorCode
// 输入：call1, call2 - 两个 GIMPLE_CALL 语句
// 输出：is_equivalent - 是否等价
ArrayDetectErrorCode areVirtualCallsEquivalent(
  AD_FUNC_ARGS,
  gimple* call1,
  gimple* call2,
  bool &is_equivalent
);

// ============================================================================
// 函数调用匹配与解构
// ============================================================================
// 匹配函数调用并解构其详细信息
// ============================================================================

// 直接调用解构信息
struct DirectCallInfo {
  tree function_decl;  // FUNCTION_DECL（GCC 内部管理）
};

// 虚函数调用解构信息
struct VirtualCallInfo {
  tree object;         // 对象表达式（用于访问虚表，GCC 内部管理）
  tree object_type;   // 对象类型（GCC 内部管理）
  tree vtable_type;   // 虚表类型（GCC 内部管理）
  tree method_decl;    // 方法声明（FUNCTION_DECL，GCC 内部管理）
};

// 间接调用解构信息
struct IndirectCallInfo {
  tree function_expr;  // 函数表达式（SSA_NAME 或其他，GCC 内部管理）
};

// 调用匹配结果（联合体，根据 call_type 使用对应字段）
struct CallMatchResult {
  CallType call_type;
  union {
    DirectCallInfo direct;
    VirtualCallInfo virtual_;
    IndirectCallInfo indirect;
  } info;
};

// 匹配函数调用表达式
// 返回值：ArrayDetectErrorCode（OK 表示匹配成功，MATCH_ERROR 表示匹配失败）
// 输入：call_fn_expr - 函数调用表达式（tree），通常来自 gimple_call_fn(call_stmt)
//      可以是 OBJ_TYPE_REF（虚函数）、FUNCTION_DECL（直接调用）、SSA_NAME（间接调用）等
// 输出：result - 匹配结果和解构信息
// 注意：匹配失败时，result 中的字段可能未初始化，应使用宏安全访问
// 使用示例：
//   tree fn = gimple_call_fn(call_stmt);
//   CallMatchResult result;
//   if (matchCallExpression(AD_ARGS, fn, result) == OK) { ... }
ArrayDetectErrorCode matchCallExpression(
  AD_FUNC_ARGS,
  tree call_fn_expr,  // 函数表达式（tree），来自 gimple_call_fn() 或类似函数
  CallMatchResult &result
);

// ============================================================================
// 安全访问宏
// ============================================================================
// 这些宏确保只在正确的 call_type 下才访问对应的字段
// ============================================================================

// 安全访问直接调用信息
// 用法：AD_MATCH_DIRECT_CALL(result, var_name) { ... } AD_MATCH_END()
// 只有在 call_type == CALL_DIRECT 时才会执行代码块
#define AD_MATCH_DIRECT_CALL(result_var, var_name) \
  if ((result_var).call_type == CALL_DIRECT) { \
    DirectCallInfo const &var_name = (result_var).info.direct;

// 安全访问虚函数调用信息
// 用法：AD_MATCH_VIRTUAL_CALL(result, var_name) { ... } AD_MATCH_END()
// 只有在 call_type == CALL_VIRTUAL 时才会执行代码块
#define AD_MATCH_VIRTUAL_CALL(result_var, var_name) \
  if ((result_var).call_type == CALL_VIRTUAL) { \
    VirtualCallInfo const &var_name = (result_var).info.virtual_;

// 安全访问间接调用信息
// 用法：AD_MATCH_INDIRECT_CALL(result, var_name) { ... } AD_MATCH_END()
// 只有在 call_type == CALL_INDIRECT 时才会执行代码块
#define AD_MATCH_INDIRECT_CALL(result_var, var_name) \
  if ((result_var).call_type == CALL_INDIRECT) { \
    IndirectCallInfo const &var_name = (result_var).info.indirect;

// 结束匹配块（必须与上述宏配对使用）
#define AD_MATCH_END() \
  }

} // namespace array_detect_ns
