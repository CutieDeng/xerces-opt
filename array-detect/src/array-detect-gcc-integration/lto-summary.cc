#include "lto-summary.hh"

#include <cstring>

// GCC LTO streaming APIs (GCC 15).
#include "lto-streamer.h"
#include "data-streamer.h"

namespace array_detect_ns {

// Magic/version for forward compatibility.
static constexpr unsigned HOST_WIDE_INT kMagic = 0x41525F4445544543ULL; // "AR_DETEC"
static constexpr unsigned HOST_WIDE_INT kVersion = 2;  // v2: added template_args

// Module-owned pointers (GC-managed allocations).
static vec<LtoUnifiedResultSummary*, va_gc>* g_wpa_summaries = nullptr;
static vec<LtoUnifiedResultSummary*, va_gc>* g_ltrans_summaries = nullptr;

static inline char const* dup_cstr (char const* s) {
  if (!s) return nullptr;
  size_t n = strlen (s);
  char* out = (char*) ggc_alloc_atomic (n + 1);
  memcpy (out, s, n + 1);
  return out;
}

namespace {

// We avoid GCC's string table here to keep things self-contained:
// write: uleb128 length + raw bytes (no NUL); read: allocate + NUL-terminate.
static inline void write_raw_string (lto_output_stream* os, char const* s) {
  if (!s) s = "";
  unsigned HOST_WIDE_INT len = (unsigned HOST_WIDE_INT) strlen (s);
  streamer_write_uhwi_stream (os, len);
  if (len) streamer_write_data_stream (os, s, (size_t) len);
}

static inline char const* read_raw_string (lto_input_block* ib) {
  unsigned HOST_WIDE_INT len = streamer_read_uhwi (ib);
  char* out = (char*) ggc_alloc_atomic ((size_t) len + 1);
  if (len) lto_input_data_block (ib, out, (size_t) len);
  out[len] = '\0';
  return out;
}

static inline void write_uid_name_list (
  lto_output_stream* os,
  vec<unsigned int, va_gc>* uids,
  vec<char const*, va_gc>* names
) {
  unsigned HOST_WIDE_INT n = (unsigned HOST_WIDE_INT) vec_safe_length (uids);
  streamer_write_uhwi_stream (os, n);
  for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
    streamer_write_uhwi_stream (os, (unsigned HOST_WIDE_INT) (*uids)[(unsigned)i]);
    write_raw_string (os, names ? (*names)[(unsigned)i] : "");
  }
}

static inline void read_uid_name_list (
  lto_input_block* ib,
  vec<unsigned int, va_gc>** out_uids,
  vec<char const*, va_gc>** out_names
) {
  unsigned HOST_WIDE_INT n = streamer_read_uhwi (ib);
  vec<unsigned int, va_gc>* uids = nullptr;
  vec<char const*, va_gc>* names = nullptr;
  if (n) {
    vec_alloc (uids, (unsigned) n);
    vec_alloc (names, (unsigned) n);
    for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
      unsigned HOST_WIDE_INT uid = streamer_read_uhwi (ib);
      char const* name = read_raw_string (ib);
      vec_safe_push (uids, (unsigned int) uid);
      vec_safe_push (names, name);
    }
  }
  *out_uids = uids;
  *out_names = names;
}

static inline void write_related_fields (
  lto_output_stream* os,
  vec<LtoRelatedFieldsSummary*, va_gc>* accesses
) {
  unsigned HOST_WIDE_INT n = (unsigned HOST_WIDE_INT) vec_safe_length (accesses);
  streamer_write_uhwi_stream (os, n);
  for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
    LtoRelatedFieldsSummary* a = (*accesses)[(unsigned)i];
    vec<unsigned int, va_gc>* uids = a ? a->field_uids : nullptr;
    vec<char const*, va_gc>* names = a ? a->field_names : nullptr;
    write_uid_name_list (os, uids, names);
  }
}

static inline void read_related_fields (
  lto_input_block* ib,
  vec<LtoRelatedFieldsSummary*, va_gc>** out_accesses
) {
  unsigned HOST_WIDE_INT n = streamer_read_uhwi (ib);
  vec<LtoRelatedFieldsSummary*, va_gc>* accesses = nullptr;
  if (n) {
    vec_alloc (accesses, (unsigned) n);
    for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
      LtoRelatedFieldsSummary* a =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (a, 0, sizeof (*a));
      read_uid_name_list (ib, &a->field_uids, &a->field_names);
      vec_safe_push (accesses, a);
    }
  }
  *out_accesses = accesses;
}

// Serialize template_args (string list)
static inline void write_string_list (
  lto_output_stream* os,
  vec<char const*, va_gc>* strings
) {
  unsigned HOST_WIDE_INT n = (unsigned HOST_WIDE_INT) vec_safe_length (strings);
  streamer_write_uhwi_stream (os, n);
  for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
    write_raw_string (os, (*strings)[(unsigned)i]);
  }
}

static inline void read_string_list (
  lto_input_block* ib,
  vec<char const*, va_gc>** out_strings
) {
  unsigned HOST_WIDE_INT n = streamer_read_uhwi (ib);
  vec<char const*, va_gc>* strings = nullptr;
  if (n) {
    vec_alloc (strings, (unsigned) n);
    for (unsigned HOST_WIDE_INT i = 0; i < n; i++) {
      char const* s = read_raw_string (ib);
      vec_safe_push (strings, s);
    }
  }
  *out_strings = strings;
}

} // namespace

void clearWpaLtoSummaries () { g_wpa_summaries = nullptr; }
void clearLtransLtoSummaries () { g_ltrans_summaries = nullptr; }

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

vec<LtoUnifiedResultSummary*, va_gc>* getWpaLtoSummaries () { return g_wpa_summaries; }
vec<LtoUnifiedResultSummary*, va_gc>* getLtransLtoSummaries () { return g_ltrans_summaries; }

bool hasLtransLtoSummaries () { return g_ltrans_summaries && g_ltrans_summaries->length (); }

void writeArrayDetectLtoSummarySection () {
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = g_wpa_summaries;
  if (!summaries || summaries->is_empty ()) return;

  // Simple output block for our summary section.
  lto_simple_output_block* sob = lto_create_simple_output_block (LTO_section_lto);
  lto_output_stream* os = sob->main_stream;

  // Header.
  streamer_write_uhwi_stream (os, kMagic);
  streamer_write_uhwi_stream (os, kVersion);
  streamer_write_uhwi_stream (os, (unsigned HOST_WIDE_INT) summaries->length ());

  // Entries.
  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* e = (*summaries)[i];
    if (!e) continue;

    write_raw_string (os, e->tu_source_file ? e->tu_source_file : "");
    streamer_write_uhwi_stream (os, (unsigned HOST_WIDE_INT) e->type_uid);
    streamer_write_uhwi_stream (os, (unsigned HOST_WIDE_INT) e->ptr_field_uid);
    write_raw_string (os, e->type_name ? e->type_name : "");
    write_string_list (os, e->template_args);  // v2: template_args
    write_raw_string (os, e->ptr_field_name ? e->ptr_field_name : "");
    streamer_write_uhwi_stream (os, (unsigned HOST_WIDE_INT) e->owned_verdict);

    // malloc-size list
    write_uid_name_list (os, e->malloc_size_field_uids, e->malloc_size_field_names);

    // reads / writes
    write_related_fields (os, e->reads);
    write_related_fields (os, e->writes);
  }

  // Emit the section.
  char* section_name = lto_get_section_name (LTO_section_lto, nullptr, 0, nullptr);
  lto_begin_section (section_name, /*compress*/ true);
  lto_write_stream (os);
  lto_end_section ();

  free (section_name);
  lto_destroy_simple_output_block (sob);
}

void readArrayDetectLtoSummarySections () {
  // Read from all input files.
  lto_file_decl_data** files = lto_get_file_decl_data ();
  if (!files) return;

  for (unsigned fi = 0; files[fi]; fi++) {
    lto_file_decl_data* file_data = files[fi];
    char const* data = nullptr;
    size_t len = 0;

    lto_input_block* ib = lto_create_simple_input_block (file_data, LTO_section_lto, &data, &len);
    if (!ib || !data || !len) {
      if (ib) lto_destroy_simple_input_block (file_data, LTO_section_lto, ib, data, len);
      continue;
    }

    // Validate header.
    unsigned HOST_WIDE_INT magic = streamer_read_uhwi (ib);
    unsigned HOST_WIDE_INT version = streamer_read_uhwi (ib);
    if (magic != kMagic || version != kVersion) {
      lto_destroy_simple_input_block (file_data, LTO_section_lto, ib, data, len);
      continue;
    }

    unsigned HOST_WIDE_INT count = streamer_read_uhwi (ib);
    vec<LtoUnifiedResultSummary*, va_gc>* file_summaries = nullptr;
    if (count) vec_alloc (file_summaries, (unsigned) count);

    for (unsigned HOST_WIDE_INT i = 0; i < count; i++) {
      LtoUnifiedResultSummary* e =
        (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
      memset (e, 0, sizeof (*e));

      e->tu_source_file = read_raw_string (ib);
      e->type_uid = (unsigned int) streamer_read_uhwi (ib);
      e->ptr_field_uid = (unsigned int) streamer_read_uhwi (ib);
      e->type_name = read_raw_string (ib);
      read_string_list (ib, &e->template_args);  // v2: template_args
      e->ptr_field_name = read_raw_string (ib);
      e->owned_verdict = (OwnedConclusionVerdict) streamer_read_uhwi (ib);

      read_uid_name_list (ib, &e->malloc_size_field_uids, &e->malloc_size_field_names);
      read_related_fields (ib, &e->reads);
      read_related_fields (ib, &e->writes);

      vec_safe_push (file_summaries, e);
    }

    appendLtransLtoSummaries (file_summaries);
    lto_destroy_simple_input_block (file_data, LTO_section_lto, ib, data, len);
  }
}

} // namespace array_detect_ns

// ============================================================================
// Include result-aggregator for UnifiedFieldAnalysisResult
// ============================================================================

#include "result-aggregator.hh"
#include "array-access-collector.hh"
#include "bound-condition-analyzer.hh"

namespace array_detect_ns {

// ============================================================================
// Conversion: UnifiedFieldAnalysisResult -> LtoUnifiedResultSummary
// ============================================================================

LtoUnifiedResultSummary* convertToLtoSummary (
  UnifiedFieldAnalysisResult* result,
  char const* tu_source_file
) {
  if (!result) return nullptr;

  LtoUnifiedResultSummary* summary =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (summary, 0, sizeof (*summary));

  // TU source file
  summary->tu_source_file = dup_cstr (tu_source_file);

  // Primary identity (use UIDs for potential cross-TU matching, names for output)
  summary->type_uid = result->type ? TYPE_UID (result->type) : 0;
  summary->ptr_field_uid = result->pointer_field_decl ? DECL_UID (result->pointer_field_decl) : 0;

  // Cached names
  summary->type_name = dup_cstr (result->type_name);
  summary->ptr_field_name = dup_cstr (result->pointer_field_name);

  // Copy template_args
  if (result->template_args && result->template_args->length () > 0) {
    vec_alloc (summary->template_args, result->template_args->length ());
    for (unsigned i = 0; i < result->template_args->length (); i++) {
      vec_safe_push (summary->template_args, dup_cstr ((*result->template_args)[i]));
    }
  }

  // Owned verdict
  summary->owned_verdict = result->owned_verdict;

  // malloc-size fields
  if (result->capacity_relations && result->capacity_relations->length () > 0) {
    vec_alloc (summary->malloc_size_field_uids, result->capacity_relations->length ());
    vec_alloc (summary->malloc_size_field_names, result->capacity_relations->length ());

    for (unsigned i = 0; i < result->capacity_relations->length (); i++) {
      CapacityFieldRelation* rel = (*result->capacity_relations)[i];
      if (rel && (rel->evidence_bitmap & EVID_MALLOC_SIZE_ARG)) {
        unsigned int uid = rel->capacity_field_decl ? DECL_UID (rel->capacity_field_decl) : 0;
        vec_safe_push (summary->malloc_size_field_uids, uid);
        vec_safe_push (summary->malloc_size_field_names, dup_cstr (rel->capacity_field_name));
      }
    }
  }

  // reads / writes - convert array accesses with their bound fields
  if (result->array_accesses) {
    vec_alloc (summary->reads, 8);
    vec_alloc (summary->writes, 8);

    for (unsigned i = 0; i < result->array_accesses->length (); i++) {
      ArrayAccessCapture* access = (*result->array_accesses)[i];
      if (!access) continue;

      LtoRelatedFieldsSummary* rf =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (rf, 0, sizeof (*rf));

      // Get bound analysis for this access
      ArrayAccessBoundAnalysis* ba = (ArrayAccessBoundAnalysis*) access->bound_analysis;
      if (ba && ba->related_fields) {
        vec_alloc (rf->field_uids, ba->related_fields->length ());
        vec_alloc (rf->field_names, ba->related_fields->length ());

        for (unsigned j = 0; j < ba->related_fields->length (); j++) {
          tree field = (*ba->related_fields)[j];
          unsigned int uid = field ? DECL_UID (field) : 0;
          char const* name = "<anon>";
          if (field && DECL_NAME (field)) {
            name = IDENTIFIER_POINTER (DECL_NAME (field));
          }
          vec_safe_push (rf->field_uids, uid);
          vec_safe_push (rf->field_names, dup_cstr (name));
        }
      }

      if (access->direction == ACCESS_READ) {
        vec_safe_push (summary->reads, rf);
      } else {
        vec_safe_push (summary->writes, rf);
      }
    }
  }

  return summary;
}

vec<LtoUnifiedResultSummary*, va_gc>* convertAllToLtoSummaries (
  vec<UnifiedFieldAnalysisResult*, va_gc>* results,
  char const* tu_source_file
) {
  if (!results || results->is_empty ()) return nullptr;

  vec<LtoUnifiedResultSummary*, va_gc>* summaries = nullptr;
  vec_alloc (summaries, results->length ());

  for (unsigned i = 0; i < results->length (); i++) {
    LtoUnifiedResultSummary* s = convertToLtoSummary ((*results)[i], tu_source_file);
    if (s) vec_safe_push (summaries, s);
  }

  return summaries;
}

// ============================================================================
// LTRANS aggregation: merge summaries from multiple TUs
// ============================================================================

vec<LtoUnifiedResultSummary*, va_gc>* aggregateLtransSummaries () {
  vec<LtoUnifiedResultSummary*, va_gc>* all = g_ltrans_summaries;
  if (!all || all->is_empty ()) return nullptr;

  // Group by (type_name, field_name)
  // For now, simple approach: keep first occurrence, merge accesses
  // A more sophisticated approach would merge owned verdicts, etc.

  vec<LtoUnifiedResultSummary*, va_gc>* result = nullptr;
  vec_alloc (result, all->length ());

  // Use a simple O(n^2) dedup for now (can optimize with hash_map later)
  for (unsigned i = 0; i < all->length (); i++) {
    LtoUnifiedResultSummary* s = (*all)[i];
    if (!s) continue;

    // Check if we already have this (type, field)
    bool found = false;
    for (unsigned j = 0; j < vec_safe_length (result); j++) {
      LtoUnifiedResultSummary* existing = (*result)[j];
      if (!existing) continue;

      bool type_match = (s->type_name && existing->type_name &&
                         strcmp (s->type_name, existing->type_name) == 0);
      bool field_match = (s->ptr_field_name && existing->ptr_field_name &&
                          strcmp (s->ptr_field_name, existing->ptr_field_name) == 0);

      if (type_match && field_match) {
        found = true;

        // Merge: upgrade owned verdict if needed
        if (s->owned_verdict == OWNED_YES && existing->owned_verdict != OWNED_YES) {
          existing->owned_verdict = OWNED_YES;
        }

        // Merge malloc-size fields
        if (s->malloc_size_field_names) {
          for (unsigned k = 0; k < s->malloc_size_field_names->length (); k++) {
            char const* name = (*s->malloc_size_field_names)[k];
            // Check if already present
            bool has = false;
            if (existing->malloc_size_field_names) {
              for (unsigned m = 0; m < existing->malloc_size_field_names->length (); m++) {
                if (strcmp ((*existing->malloc_size_field_names)[m], name) == 0) {
                  has = true;
                  break;
                }
              }
            }
            if (!has) {
              if (!existing->malloc_size_field_names) {
                vec_alloc (existing->malloc_size_field_names, 4);
                vec_alloc (existing->malloc_size_field_uids, 4);
              }
              vec_safe_push (existing->malloc_size_field_names, name);
              vec_safe_push (existing->malloc_size_field_uids, (*s->malloc_size_field_uids)[k]);
            }
          }
        }

        // Merge reads/writes (append)
        if (s->reads) {
          if (!existing->reads) vec_alloc (existing->reads, 8);
          for (unsigned k = 0; k < s->reads->length (); k++) {
            vec_safe_push (existing->reads, (*s->reads)[k]);
          }
        }
        if (s->writes) {
          if (!existing->writes) vec_alloc (existing->writes, 8);
          for (unsigned k = 0; k < s->writes->length (); k++) {
            vec_safe_push (existing->writes, (*s->writes)[k]);
          }
        }

        break;
      }
    }

    if (!found) {
      vec_safe_push (result, s);
    }
  }

  return result;
}

// ============================================================================
// Write final LTRANS results to Racket datum
// ============================================================================

void writeLtransResultsToRacketDatum (char const* output_path) {
  vec<LtoUnifiedResultSummary*, va_gc>* aggregated = aggregateLtransSummaries ();
  if (!aggregated || aggregated->is_empty ()) return;

  FILE* f = fopen (output_path, "a");
  if (!f) return;

  for (unsigned i = 0; i < aggregated->length (); i++) {
    LtoUnifiedResultSummary* s = (*aggregated)[i];
    if (!s) continue;

    // Format: ((file "...")(type "TypeName" ("arg1" ...))(field "...")(owned yes|no|undetermined)
    //          (malloc-size (...))(reads (...))(writes (...)))
    fprintf (f, "((file \"%s\")", s->tu_source_file ? s->tu_source_file : "");

    // type with template_args
    fprintf (f, "(type \"%s\" (", s->type_name ? s->type_name : "");
    if (s->template_args) {
      for (unsigned j = 0; j < s->template_args->length (); j++) {
        if (j > 0) fprintf (f, " ");
        fprintf (f, "\"%s\"", (*s->template_args)[j] ? (*s->template_args)[j] : "");
      }
    }
    fprintf (f, "))");

    fprintf (f, "(field \"%s\")", s->ptr_field_name ? s->ptr_field_name : "");

    // owned
    char const* owned_str = "undetermined";
    if (s->owned_verdict == OWNED_YES) owned_str = "yes";
    else if (s->owned_verdict == OWNED_NO) owned_str = "no";
    fprintf (f, "(owned %s)", owned_str);

    // malloc-size
    fprintf (f, "(malloc-size (");
    if (s->malloc_size_field_names) {
      for (unsigned j = 0; j < s->malloc_size_field_names->length (); j++) {
        if (j > 0) fprintf (f, " ");
        fprintf (f, "\"%s\"", (*s->malloc_size_field_names)[j]);
      }
    }
    fprintf (f, "))");

    // reads
    fprintf (f, "(reads (");
    if (s->reads) {
      for (unsigned j = 0; j < s->reads->length (); j++) {
        LtoRelatedFieldsSummary* rf = (*s->reads)[j];
        fprintf (f, "(");
        if (rf && rf->field_names) {
          for (unsigned k = 0; k < rf->field_names->length (); k++) {
            if (k > 0) fprintf (f, " ");
            fprintf (f, "\"%s\"", (*rf->field_names)[k]);
          }
        }
        fprintf (f, ")");
      }
    }
    fprintf (f, "))");

    // writes
    fprintf (f, "(writes (");
    if (s->writes) {
      for (unsigned j = 0; j < s->writes->length (); j++) {
        LtoRelatedFieldsSummary* rf = (*s->writes)[j];
        fprintf (f, "(");
        if (rf && rf->field_names) {
          for (unsigned k = 0; k < rf->field_names->length (); k++) {
            if (k > 0) fprintf (f, " ");
            fprintf (f, "\"%s\"", (*rf->field_names)[k]);
          }
        }
        fprintf (f, ")");
      }
    }
    fprintf (f, "))");

    fprintf (f, ")\n");
  }

  fclose (f);
}

} // namespace array_detect_ns


