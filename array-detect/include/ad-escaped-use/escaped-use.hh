#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-use.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸使用结果 (EscapedUseResult)
// ============================================================================
// 数据流：(listof write-original-source-use) -> (listof escaped-use)
// 从 all_uses 中提取所有逃逸使用
//
// (escaped-use-result
//   escapes        : (listof write-original-source-use*)  ; 指向 all-uses 中逃逸元素
//   escape-count   : nat)
// ============================================================================

struct EscapedUseResult {
  vec<SourceUseInfo const *> * escapes;  // 所有逃逸使用（指向原 all_uses 元素）
  unsigned int escape_count;             // 逃逸数量
};

// ============================================================================
// 接口函数
// ============================================================================

// 提取逃逸使用
// 输入：(listof SourceUseInfo)
// 输出：EscapedUseResult 指针
ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  vec<SourceUseInfo>* all_uses,
  EscapedUseResult** out_result
);

// ============================================================================
// 调试输出
// ============================================================================

void printEscapedUseResult (
  EscapedUseResult const * result,
  FILE * output
);

} // namespace array_detect_ns
