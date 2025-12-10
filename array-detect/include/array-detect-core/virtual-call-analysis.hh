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

} // namespace array_detect_ns
