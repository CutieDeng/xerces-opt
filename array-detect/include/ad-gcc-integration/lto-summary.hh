#pragma once

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "owned-conclusion.hh"

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
void writeArrayDetectLtoSummarySection (AD_FUNC_ARGS);
void readArrayDetectLtoSummarySections (AD_FUNC_ARGS);

// ============================================================================
// Conversion from analysis results (已废弃)
// ============================================================================

// NOTE: convertToLtoSummary 和 convertAllToLtoSummaries 已移除
// 这些函数依赖已废弃的 result-aggregator 模块
// 如需 LTO 功能，请基于新的 ad-array-read-capacity / ad-array-write-capacity 模块重新实现

// ============================================================================
// LTRANS aggregation (已废弃)
// ============================================================================

// NOTE: aggregateLtransSummaries, writeLtransAggregatedResults, writeLtransResultsToRacketDatum 已移除

} // namespace array_detect_ns
