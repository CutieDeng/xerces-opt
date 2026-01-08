#pragma once

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
// Streaming (WPA write / LTRANS read)
// ============================================================================

// Called by ipa pass hooks (write_summary/read_summary).
void writeArrayDetectLtoSummarySection ();
void readArrayDetectLtoSummarySections ();

// ============================================================================
// Conversion from analysis results
// ============================================================================

struct UnifiedFieldAnalysisResult;  // Forward declaration

// Convert analysis result to LTO-serializable summary
LtoUnifiedResultSummary* convertToLtoSummary (
  UnifiedFieldAnalysisResult* result,
  char const* tu_source_file
);

// Convert all results
vec<LtoUnifiedResultSummary*, va_gc>* convertAllToLtoSummaries (
  vec<UnifiedFieldAnalysisResult*, va_gc>* results,
  char const* tu_source_file
);

// ============================================================================
// LTRANS aggregation
// ============================================================================

// Merge summaries from multiple TUs by (type_name, field_name)
// Returns aggregated results ready for final output
vec<LtoUnifiedResultSummary*, va_gc>* aggregateLtransSummaries ();

// Write final aggregated results to Racket datum (called in LTRANS)
// Uses OVERWRITE mode (not append)
void writeLtransAggregatedResults (char const* output_path);

// Legacy: append mode (deprecated, use writeLtransAggregatedResults instead)
void writeLtransResultsToRacketDatum (char const* output_path);

} // namespace array_detect_ns
