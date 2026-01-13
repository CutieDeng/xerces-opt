#include "lto-summary.hh"

#include <cstring>
#include <cstdio>

#include "context.hh"
#include "prelude.hh"
#include "stor-layout.h"
#include "own-conclude.hh"
#include "array-detector.hh"
#include "string-utils.hh"
#include "gcc-ext-util.hh"

// LTO streaming API
#include "lto-streamer.h"
#include "data-streamer.h"
#include "lto-section-names.h"

namespace array_detect_ns {

// ============================================================================
// Constants
// ============================================================================

// Magic number and version for format validation
static unsigned HOST_WIDE_INT const kSummaryMagic = 0x41444C544F33ULL; // "ADLTO3" (v3)
static unsigned HOST_WIDE_INT const kSummaryVersion = 3;

// Custom LTO section name for array-detect plugin
// Note: lto_begin_section takes the base name only, not the full section name
static char const* const kArrayDetectSectionName = "array_detect";

// ============================================================================
// Module state
// ============================================================================

static vec<LtoUnifiedResultSummary*, va_gc>* g_wpa_summaries = nullptr;
static vec<LtoUnifiedResultSummary*, va_gc>* g_ltrans_summaries = nullptr;
static bool g_ltrans_summaries_loaded = false;

// ============================================================================
// Helper: duplicate C-string into GC-managed memory
// ============================================================================

static inline char const* dup_cstr (char const* s) {
  if (!s) return nullptr;
  size_t n = strlen (s);
  char* out = (char*) ggc_alloc_atomic (n + 1);
  memcpy (out, s, n + 1);
  return out;
}

// ============================================================================
// Public API: Lifecycle & accessors
// ============================================================================

void clearWpaLtoSummaries () {
  g_wpa_summaries = nullptr;
}

void clearLtransLtoSummaries () {
  g_ltrans_summaries = nullptr;
  g_ltrans_summaries_loaded = false;
}

void setWpaLtoSummaries (vec<LtoUnifiedResultSummary*, va_gc>* summaries) {
  g_wpa_summaries = summaries;
}

void appendLtransLtoSummaries (vec<LtoUnifiedResultSummary*, va_gc>* summaries) {
  if (!summaries) return;
  if (!g_ltrans_summaries) vec_alloc (g_ltrans_summaries, vec_safe_length (summaries));
  for (unsigned i = 0; i < vec_safe_length (summaries); i++) {
    vec_safe_push (g_ltrans_summaries, (*summaries)[i]);
  }
}

static void appendLtransLtoSummaries_single (LtoUnifiedResultSummary* s) {
  if (!s) return;
  if (!g_ltrans_summaries) vec_alloc (g_ltrans_summaries, 16);
  vec_safe_push (g_ltrans_summaries, s);
}

vec<LtoUnifiedResultSummary*, va_gc>* getWpaLtoSummaries () { return g_wpa_summaries; }
vec<LtoUnifiedResultSummary*, va_gc>* getLtransLtoSummaries () { return g_ltrans_summaries; }

bool hasLtransLtoSummaries () { return g_ltrans_summaries && g_ltrans_summaries->length (); }

// ============================================================================
// Buffer encoding helpers (for raw section write)
// ============================================================================

static void buf_write_uhwi (vec<unsigned char, va_gc>*& buf, unsigned HOST_WIDE_INT val) {
  do {
    unsigned char byte = val & 0x7f;
    val >>= 7;
    if (val) byte |= 0x80;
    vec_safe_push (buf, byte);
  } while (val);
}

static void buf_write_string (vec<unsigned char, va_gc>*& buf, char const* s) {
  if (!s) s = "";
  size_t len = strlen (s);
  buf_write_uhwi (buf, (unsigned HOST_WIDE_INT) len);
  for (size_t i = 0; i < len; i++) {
    vec_safe_push (buf, (unsigned char) s[i]);
  }
}

static void buf_write_field_counts (vec<unsigned char, va_gc>*& buf, vec<LtoFieldCount, va_gc>* counts) {
  unsigned int n = vec_safe_length (counts);
  buf_write_uhwi (buf, n);
  for (unsigned int i = 0; i < n; i++) {
    LtoFieldCount& fc = (*counts)[i];
    buf_write_string (buf, fc.field_name);
    buf_write_uhwi (buf, fc.count);
  }
}

static void buf_write_summary_entry (vec<unsigned char, va_gc>*& buf, LtoUnifiedResultSummary* e) {
  if (!e) return;

  // Basic info
  buf_write_string (buf, e->tu_source_file);
  buf_write_string (buf, e->type_name);

  // Template args
  unsigned int tmpl_count = vec_safe_length (e->template_args);
  buf_write_uhwi (buf, tmpl_count);
  for (unsigned int i = 0; i < tmpl_count; i++) {
    buf_write_string (buf, (*e->template_args)[i]);
  }

  buf_write_string (buf, e->ptr_field_name);
  buf_write_uhwi (buf, (unsigned HOST_WIDE_INT) e->owned_verdict);

  // malloc-size: total + field counts
  buf_write_uhwi (buf, e->malloc_total);
  buf_write_field_counts (buf, e->malloc_field_counts);

  // reads: total + field counts
  buf_write_uhwi (buf, e->reads_total);
  buf_write_field_counts (buf, e->read_field_counts);

  // writes: total + field counts
  buf_write_uhwi (buf, e->writes_total);
  buf_write_field_counts (buf, e->write_field_counts);
}

// ============================================================================
// Buffer decoding helpers (for raw section read)
// ============================================================================

struct BufReader {
  char const* data;
  size_t len;
  size_t pos;
};

static unsigned HOST_WIDE_INT buf_read_uhwi (BufReader& r) {
  unsigned HOST_WIDE_INT result = 0;
  unsigned shift = 0;
  while (r.pos < r.len) {
    unsigned char byte = (unsigned char) r.data[r.pos++];
    result |= ((unsigned HOST_WIDE_INT)(byte & 0x7f)) << shift;
    if (!(byte & 0x80)) break;
    shift += 7;
  }
  return result;
}

static char const* buf_read_string (BufReader& r) {
  unsigned HOST_WIDE_INT len = buf_read_uhwi (r);
  if (len == 0) return dup_cstr ("");
  if (r.pos + len > r.len) return dup_cstr ("");
  char* buf = (char*) ggc_alloc_atomic ((size_t) len + 1);
  memcpy (buf, r.data + r.pos, (size_t) len);
  buf[len] = '\0';
  r.pos += (size_t) len;
  return buf;
}

static vec<LtoFieldCount, va_gc>* buf_read_field_counts (BufReader& r) {
  unsigned int n = (unsigned int) buf_read_uhwi (r);
  if (n == 0) return nullptr;

  vec<LtoFieldCount, va_gc>* counts = nullptr;
  vec_alloc (counts, n);

  for (unsigned int i = 0; i < n; i++) {
    LtoFieldCount fc;
    fc.field_name = buf_read_string (r);
    fc.count = (unsigned int) buf_read_uhwi (r);
    vec_safe_push (counts, fc);
  }

  return counts;
}

static LtoUnifiedResultSummary* buf_read_summary_entry (BufReader& r) {
  LtoUnifiedResultSummary* e =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (e, 0, sizeof (*e));

  // Basic info
  e->tu_source_file = buf_read_string (r);
  e->type_name = buf_read_string (r);

  // Template args
  unsigned int tmpl_count = (unsigned int) buf_read_uhwi (r);
  if (tmpl_count > 0) {
    vec_alloc (e->template_args, tmpl_count);
    for (unsigned int i = 0; i < tmpl_count; i++) {
      vec_safe_push (e->template_args, buf_read_string (r));
    }
  }

  e->ptr_field_name = buf_read_string (r);
  e->owned_verdict = (OwnedConclusionVerdict) buf_read_uhwi (r);

  // malloc-size: total + field counts
  e->malloc_total = (unsigned int) buf_read_uhwi (r);
  e->malloc_field_counts = buf_read_field_counts (r);

  // reads: total + field counts
  e->reads_total = (unsigned int) buf_read_uhwi (r);
  e->read_field_counts = buf_read_field_counts (r);

  // writes: total + field counts
  e->writes_total = (unsigned int) buf_read_uhwi (r);
  e->write_field_counts = buf_read_field_counts (r);

  return e;
}

// ============================================================================
// Public API: Write summary using raw LTO section API
// ============================================================================

ArrayDetectErrorCode writeArrayDetectLtoSummarySection (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = g_wpa_summaries;
  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("skip (no summaries)");
    AD_RETURNE (OK);
  }

  unsigned int count = summaries->length ();
  AD_DEBUG_PRINT ("writing %u summaries", count);

  // Encode all data to a buffer
  vec<unsigned char, va_gc>* buf = nullptr;
  vec_alloc (buf, 4096);

  buf_write_uhwi (buf, kSummaryMagic);
  buf_write_uhwi (buf, kSummaryVersion);
  buf_write_uhwi (buf, count);

  for (unsigned int i = 0; i < count; i++) {
    LtoUnifiedResultSummary* e = (*summaries)[i];
    if (e) {
      buf_write_summary_entry (buf, e);
    }
  }

  // Write to LTO section using raw API
  // Note: lto_begin_section takes just the base name, not the full "decls.name.order" format
  lto_begin_section (kArrayDetectSectionName, false);
  if (buf && buf->length () > 0) {
    lto_write_data (buf->address (), buf->length ());
  }
  lto_end_section ();

  AD_DEBUG_PRINT ("done writing %u summaries, %u bytes to section '%s'",
                  count, vec_safe_length (buf), kArrayDetectSectionName);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// Public API: Read summary from raw data
// ============================================================================

ArrayDetectErrorCode readArrayDetectLtoSummarySections (AD_FUNC_ARGS, char const* data, size_t len) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;

  if (!data || len == 0) {
    AD_DEBUG_PRINT ("ERROR: no data");
    AD_RETURNE (INVALID_ARGUMENT);
  }

  AD_DEBUG_PRINT ("ENTRY, len=%zu", len);

  BufReader r = { data, len, 0 };

  unsigned HOST_WIDE_INT magic = buf_read_uhwi (r);
  if (magic != kSummaryMagic) {
    AD_DEBUG_PRINT ("bad magic 0x%llx (expected 0x%llx)",
                    (unsigned long long) magic, (unsigned long long) kSummaryMagic);
    AD_RETURNE (INVALID_ARGUMENT);
  }

  unsigned HOST_WIDE_INT version = buf_read_uhwi (r);
  if (version != kSummaryVersion) {
    AD_DEBUG_PRINT ("unknown version %llu (expected %llu)",
                    (unsigned long long) version, (unsigned long long) kSummaryVersion);
    AD_RETURNE (INVALID_ARGUMENT);
  }

  unsigned HOST_WIDE_INT count = buf_read_uhwi (r);
  AD_DEBUG_PRINT ("%llu entries", (unsigned long long) count);

  unsigned int total_summaries = 0;
  for (unsigned HOST_WIDE_INT i = 0; i < count && r.pos < r.len; i++) {
    LtoUnifiedResultSummary* e = buf_read_summary_entry (r);
    if (e) {
      appendLtransLtoSummaries_single (e);
      total_summaries++;
    }
  }

  AD_DEBUG_PRINT ("EXIT summaries=%u", total_summaries);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// Conversion from FieldOwnedConclusion to LtoUnifiedResultSummary
// ============================================================================

LtoUnifiedResultSummary* convertFieldOwnedConclusionToLtoSummary (
  AD_FUNC_ARGS,
  FieldOwnedConclusion* conclusion,
  ::field_analysis::Wrapper_FieldEscapeConclude_TransferStats* tfad
) {
  if (!conclusion) return nullptr;

  LtoUnifiedResultSummary* summary =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (summary, 0, sizeof (*summary));

  // Basic identity
  summary->tu_source_file = dup_cstr (main_input_filename);
  summary->type_name = dup_cstr (conclusion->type_name ? conclusion->type_name : "<anon>");
  summary->ptr_field_name = dup_cstr (conclusion->field_name ? conclusion->field_name : "<anon>");
  summary->owned_verdict = conclusion->verdict;

  // Template args (if type is a template instance)
  summary->template_args = nullptr;
  if (conclusion->type) {
    char const* base_name = nullptr;
    vec<char const*, va_gc>* tmpl_args = nullptr;
    gcc_ext_util::extractTemplateArgsFromType (AD_ARGS, conclusion->type, &base_name, &tmpl_args);
    if (tmpl_args && tmpl_args->length () > 0) {
      vec_alloc (summary->template_args, tmpl_args->length ());
      for (unsigned i = 0; i < tmpl_args->length (); i++) {
        vec_safe_push (summary->template_args, dup_cstr ((*tmpl_args)[i]));
      }
    }
  }

  // Extract capacity evidence from tfad
  if (tfad) {
    // malloc-size: count SOURCE_FUNCTION_CALL writes as total
    summary->malloc_total = 0;
    if (tfad->writes) {
      for (unsigned i = 0; i < tfad->writes->length (); i++) {
        auto* wrapper = (*tfad->writes)[i];
        if (!wrapper || !wrapper->write_source) continue;
        if (wrapper->write_source->source_type == ::array_detector::SOURCE_FUNCTION_CALL) {
          summary->malloc_total++;
        }
      }
    }

    // malloc field counts
    if (tfad->malloc_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->malloc_evidences_map->begin ();
           iter != tfad->malloc_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->malloc_field_counts, count);
        for (auto iter = tfad->malloc_evidences_map->begin ();
             iter != tfad->malloc_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          vec<::array_detector::MallocCapacityEvidence*, va_gc>* evidences = (*iter).second;
          LtoFieldCount fc;
          fc.field_name = dup_cstr (safeGetFieldName (AD_ARGS, integer_field));
          fc.count = evidences ? evidences->length () : 0;
          vec_safe_push (summary->malloc_field_counts, fc);
        }
      }
    }

    // reads: total + field counts
    summary->reads_total = tfad->array_reads ? tfad->array_reads->length () : 0;
    if (tfad->read_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->read_evidences_map->begin ();
           iter != tfad->read_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->read_field_counts, count);
        for (auto iter = tfad->read_evidences_map->begin ();
             iter != tfad->read_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          vec<::array_detector::ReadCapacityEvidence*, va_gc>* evidences = (*iter).second;
          LtoFieldCount fc;
          fc.field_name = dup_cstr (safeGetFieldName (AD_ARGS, integer_field));
          fc.count = evidences ? evidences->length () : 0;
          vec_safe_push (summary->read_field_counts, fc);
        }
      }
    }

    // writes: total + field counts
    summary->writes_total = tfad->array_writes ? tfad->array_writes->length () : 0;
    if (tfad->write_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->write_evidences_map->begin ();
           iter != tfad->write_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->write_field_counts, count);
        for (auto iter = tfad->write_evidences_map->begin ();
             iter != tfad->write_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          vec<::array_detector::WriteCapacityEvidence*, va_gc>* evidences = (*iter).second;
          LtoFieldCount fc;
          fc.field_name = dup_cstr (safeGetFieldName (AD_ARGS, integer_field));
          fc.count = evidences ? evidences->length () : 0;
          vec_safe_push (summary->write_field_counts, fc);
        }
      }
    }
  }

  return summary;
}

vec<LtoUnifiedResultSummary*, va_gc>* convertAllFieldOwnedConclusionsToLtoSummaries (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions,
  ::array_detector::ArrayDetector& detector
) {
  if (!conclusions || conclusions->is_empty ()) {
    return nullptr;
  }

  vec<LtoUnifiedResultSummary*, va_gc>* summaries = nullptr;
  vec_alloc (summaries, conclusions->length ());

  for (unsigned i = 0; i < conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*conclusions)[i];
    if (!conclusion) continue;

    // Find corresponding TypeFieldAnalysisData
    ::field_analysis::Wrapper_FieldEscapeConclude_TransferStats* tfad = nullptr;
    if (detector.m_type_field_writes && conclusion->type && conclusion->field_decl) {
      ::array_detector::TypeFieldKey key = {
        TYPE_MAIN_VARIANT (conclusion->type),
        conclusion->field_decl
      };
      ::field_analysis::Wrapper_FieldEscapeConclude_TransferStats** slot =
        detector.m_type_field_writes->get (key);
      if (slot) {
        tfad = *slot;
      }
    }

    LtoUnifiedResultSummary* summary =
      convertFieldOwnedConclusionToLtoSummary (AD_ARGS, conclusion, tfad);
    if (summary) {
      vec_safe_push (summaries, summary);
    }
  }

  AD_DEBUG_PRINT ("converted %u conclusions",
                  vec_safe_length (summaries));
  return summaries;
}

} // namespace array_detect_ns
