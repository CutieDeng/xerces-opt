#pragma once

// ============================================================================
// ad-result-output: 结果输出模块
// ============================================================================
// 管理分析结果输出到 AD_RESULT_FILE
// 输出格式: Racket datum, 单行, append 模式
//
// 普通编译: pipeline 结束后直接输出
// LTO 模式: 仅在 LTRANS 阶段输出
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "lto-summary.hh"

namespace array_detect_ns {

// 前向声明
struct FieldOwnedConclusion;

namespace field_analysis {
  struct Wrapper_FieldEscapeConclude_OwnershipConclude;
}

namespace array_detector {
  class ArrayDetector;
}

// ============================================================================
// 输出函数
// ============================================================================

// 输出单个 owned 字段的 Racket datum (单行)
// 格式: (owned "TypeName" "field_name" (malloc-size "field1" ...) (reads "field2" ...) (writes "field3" ...))
// 仅当 verdict == OWNED_YES 时输出
void writeOwnedFieldDatum (
  AD_FUNC_ARGS,
  FILE* out,
  FieldOwnedConclusion* conclusion,
  field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 从 LtoUnifiedResultSummary 输出 (用于 LTRANS 阶段)
void writeOwnedFieldDatumFromSummary (
  AD_FUNC_ARGS,
  FILE* out,
  LtoUnifiedResultSummary* summary
);

// ============================================================================
// Pipeline 集成
// ============================================================================

// 普通编译: 输出所有 owned 字段到 AD_RESULT_FILE
// 内部检查 flag_generate_lto, 如果是 LTO 模式则跳过
ArrayDetectErrorCode writeResultsToFile (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions,
  ::array_detector::ArrayDetector& detector
);

// LTRANS 阶段: 输出所有 owned 字段到 AD_RESULT_FILE
// 从 LTO summaries 中读取数据
ArrayDetectErrorCode writeLtransResultsToFile (AD_FUNC_ARGS);

} // namespace array_detect_ns
