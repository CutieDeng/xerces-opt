#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-escape-collection.hh"
#include "field-wrapper.hh"

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
// 输入：all_uses (所有使用点)
// 输出：直接写入 wrapper->escape_uses 和 wrapper->escape_count
ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  vec<field_analysis::FieldUsePoint>* all_uses,
  vec<field_analysis::FieldUsePoint const*>** out_escape_uses,
  unsigned int* out_escape_count,
  bool* out_has_escape
);

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
