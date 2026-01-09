#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-escape-collection.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸使用结果 (EscapedUseResult)
// ============================================================================
// 数据流位置：(listof source-use) -> (listof escaped-use)
// 从 SourceUseResult.all_uses 中提取所有逃逸使用
//
// (escaped-use-result
//   source-operand : tree
//   escapes        : (listof use-info*)  ; 指向 all-uses 中逃逸元素
//   escape-count   : nat)
// ============================================================================

struct EscapedUseResult {
  tree source_operand;              // 源操作数
  gimple * source_stmt;             // 源语句

  vec<SourceUseInfo const *> * escapes;  // 所有逃逸使用
  unsigned int escape_count;        // 逃逸数量

  void * aux;
  void * original_write_info;       // 原始 FieldWriteInfo*
};

// 向后兼容别名
typedef EscapedUseResult EscapeExtractionResult;

// ============================================================================
// 接口函数
// ============================================================================

// 提取逃逸使用
// SourceUseResult -> EscapedUseResult
ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  SourceUseResult * use_result,
  EscapedUseResult * &result
);

// 向后兼容别名
inline ArrayDetectErrorCode extractEscapes (
  AD_FUNC_ARGS_DECL,
  SourceUseResult * use_result,
  EscapedUseResult * &result
) { return extractEscapedUses(AD_FUNC_ARGS_CALL, use_result, result); }

// ============================================================================
// 调试输出
// ============================================================================

void printEscapedUseResult (
  EscapedUseResult const * result,
  FILE * output
);

// 向后兼容别名
inline void printEscapeExtractionResult (
  EscapedUseResult const * result,
  FILE * output
) { printEscapedUseResult(result, output); }

} // namespace array_detect_ns
