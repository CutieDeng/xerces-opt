#pragma once

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "own-conclude.hh"

// Forward declarations for GCC LTO types (global namespace)
class lto_input_block;

namespace array_detect_ns {

// ============================================================================
// LTO summary payload (serializable)
// ============================================================================

// A single (read/write) access instance in the output format.
// We intentionally store only stable primitives (uids + strings), so we can
// replay the existing Racket datum output in LTRANS without needing gimple/tree.
struct LtoRelatedFieldsSummary {
  vec<unsigned int, va_gc>* field_uids;   // DECL_UID(field)
  vec<char const*, va_gc>* field_names;   // DECL_NAME(field) or "<anon>"
};

struct LtoUnifiedResultSummary {
  // Origin (TU) identity; used to populate (file "...") in the datum output.
  char const* tu_source_file;

  // Primary identity.
  unsigned int type_uid;              // TYPE_UID(type)
  unsigned int ptr_field_uid;         // DECL_UID(pointer_field_decl)

  // Cached names (used for output; avoids needing tree reconstruction).
  char const* type_name;
  vec<char const*, va_gc>* template_args; // 模板参数列表
  char const* ptr_field_name;

  // Owned verdict (matches existing datum semantics).
  OwnedConclusionVerdict owned_verdict;

  // (malloc-size (...)) list: capacity field candidates with malloc evidence.
  vec<unsigned int, va_gc>* malloc_size_field_uids;
  vec<char const*, va_gc>* malloc_size_field_names;

  // (reads (...)) / (writes (...)) lists: one entry per access instance,
  // each entry is a list of related bound fields.
  vec<LtoRelatedFieldsSummary*, va_gc>* reads;
  vec<LtoRelatedFieldsSummary*, va_gc>* writes;
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
// Write uses lto_begin_section/lto_write_data/lto_end_section with custom section name.
// Read uses lto_input_block from lto_get_section_data.
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
