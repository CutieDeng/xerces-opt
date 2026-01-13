#pragma once

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "own-conclude.hh"

namespace array_detect_ns {

// ============================================================================
// LTO summary payload (serializable)
// ============================================================================

// 字段计数条目
struct LtoFieldCount {
  char const* field_name;
  unsigned int count;
};

// 统一的 LTO 结果摘要
// 与普通阶段的输出格式保持一致：
//   (owned "Type" (template-args ...) "field"
//     (malloc-size (total . N) ("f1" . c1) ...)
//     (reads (total . N) ("f1" . c1) ...)
//     (writes (total . N) ("f1" . c1) ...))
struct LtoUnifiedResultSummary {
  // 来源文件（诊断用）
  char const* tu_source_file;

  // 类型和字段标识
  char const* type_name;
  vec<char const*, va_gc>* template_args;
  char const* ptr_field_name;

  // 所有权判定结果
  OwnedConclusionVerdict owned_verdict;

  // malloc-size: total + per-field counts
  unsigned int malloc_total;
  vec<LtoFieldCount, va_gc>* malloc_field_counts;

  // reads: total + per-field counts
  unsigned int reads_total;
  vec<LtoFieldCount, va_gc>* read_field_counts;

  // writes: total + per-field counts
  unsigned int writes_total;
  vec<LtoFieldCount, va_gc>* write_field_counts;
};

// ============================================================================
// Lifecycle & accessors
// ============================================================================

void clearWpaLtoSummaries ();
void clearLtransLtoSummaries ();

// Owned by module (GC allocated). Callers should not free.
void setWpaLtoSummaries (vec<LtoUnifiedResultSummary*, va_gc>* summaries);
void appendLtransLtoSummaries (vec<LtoUnifiedResultSummary*, va_gc>* summaries);

vec<LtoUnifiedResultSummary*, va_gc>* getWpaLtoSummaries ();
vec<LtoUnifiedResultSummary*, va_gc>* getLtransLtoSummaries ();

bool hasLtransLtoSummaries ();

// ============================================================================
// Streaming via LTO decls (WPA write / LTRANS read)
// ============================================================================

// Called by ipa pass hooks (write_summary/read_summary).
// Write uses lto_begin_section/lto_write_data with section name "decls.array_detect.0".
// Read uses lto_get_raw_section_data with LTO_section_decls type.
ArrayDetectErrorCode writeArrayDetectLtoSummarySection (AD_FUNC_ARGS);
ArrayDetectErrorCode readArrayDetectLtoSummarySections (AD_FUNC_ARGS, char const* data, size_t len);

// ============================================================================
// Conversion from analysis results
// ============================================================================

// 前向声明
struct FieldOwnedConclusion;

namespace field_analysis {
  struct Wrapper_FieldEscapeConclude_TransferStats;
}

namespace array_detector {
  class ArrayDetector;
}

// 从 FieldOwnedConclusion 转换为 LtoUnifiedResultSummary
// conclusion: 字段 owned 结论
// tfad: 对应的 TypeFieldAnalysisData (用于获取 capacity 证据)
LtoUnifiedResultSummary* convertFieldOwnedConclusionToLtoSummary (
  AD_FUNC_ARGS,
  FieldOwnedConclusion* conclusion,
  field_analysis::Wrapper_FieldEscapeConclude_TransferStats* tfad
);

// 批量转换所有 FieldOwnedConclusion 为 LtoUnifiedResultSummary
// conclusions: 字段 owned 结论列表
// detector: ArrayDetector (用于查找对应的 TypeFieldAnalysisData)
vec<LtoUnifiedResultSummary*, va_gc>* convertAllFieldOwnedConclusionsToLtoSummaries (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions,
  ::array_detector::ArrayDetector& detector
);

} // namespace array_detect_ns
