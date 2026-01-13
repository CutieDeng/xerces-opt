#include "lto-summary.hh"

#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <climits>

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

namespace array_detect_ns {

// ============================================================================
// Constants
// ============================================================================

// Magic number and version for format validation
static unsigned HOST_WIDE_INT const kSummaryMagic = 0x41444C544F32ULL; // "ADLTO2"
static unsigned HOST_WIDE_INT const kSummaryVersion = 2;

// Custom LTO section name for array-detect plugin
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
// Binary streaming helpers (using lto_output_stream directly)
// ============================================================================

// Write a length-prefixed string to output stream
static void stream_write_string (struct lto_output_stream* obs, char const* s) {
  if (!s) s = "";
  size_t len = strlen (s);
  streamer_write_uhwi_stream (obs, (unsigned HOST_WIDE_INT) len);
  if (len > 0) {
    streamer_write_data_stream (obs, s, len);
  }
}

// Read a length-prefixed string from input block
static char const* stream_read_string (class lto_input_block* ib) {
  unsigned HOST_WIDE_INT len = streamer_read_uhwi (ib);
  if (len == 0) {
    return dup_cstr ("");
  }
  char* buf = (char*) ggc_alloc_atomic ((size_t) len + 1);
  for (unsigned HOST_WIDE_INT i = 0; i < len; i++) {
    buf[i] = (char) streamer_read_uchar (ib);
  }
  buf[len] = '\0';
  return buf;
}

// ============================================================================
// Write a single LtoUnifiedResultSummary to output stream
// ============================================================================

static void write_summary_entry (struct lto_output_stream* obs,
                                  LtoUnifiedResultSummary* e) {
  if (!e) return;

  // Basic identity
  stream_write_string (obs, e->tu_source_file);
  streamer_write_uhwi_stream (obs, e->type_uid);
  streamer_write_uhwi_stream (obs, e->ptr_field_uid);
  stream_write_string (obs, e->type_name);
  stream_write_string (obs, e->ptr_field_name);
  streamer_write_uhwi_stream (obs, (unsigned HOST_WIDE_INT) e->owned_verdict);

  // Template args
  unsigned int tmpl_count = vec_safe_length (e->template_args);
  streamer_write_uhwi_stream (obs, tmpl_count);
  for (unsigned int i = 0; i < tmpl_count; i++) {
    stream_write_string (obs, (*e->template_args)[i]);
  }

  // Malloc size fields
  unsigned int malloc_count = vec_safe_length (e->malloc_size_field_names);
  streamer_write_uhwi_stream (obs, malloc_count);
  for (unsigned int i = 0; i < malloc_count; i++) {
    streamer_write_uhwi_stream (obs, (*e->malloc_size_field_uids)[i]);
    stream_write_string (obs, (*e->malloc_size_field_names)[i]);
  }

  // Reads
  unsigned int reads_count = vec_safe_length (e->reads);
  streamer_write_uhwi_stream (obs, reads_count);
  for (unsigned int i = 0; i < reads_count; i++) {
    LtoRelatedFieldsSummary* rf = (*e->reads)[i];
    unsigned int field_count = rf ? vec_safe_length (rf->field_names) : 0;
    streamer_write_uhwi_stream (obs, field_count);
    for (unsigned int j = 0; j < field_count; j++) {
      streamer_write_uhwi_stream (obs, (*rf->field_uids)[j]);
      stream_write_string (obs, (*rf->field_names)[j]);
    }
  }

  // Writes
  unsigned int writes_count = vec_safe_length (e->writes);
  streamer_write_uhwi_stream (obs, writes_count);
  for (unsigned int i = 0; i < writes_count; i++) {
    LtoRelatedFieldsSummary* wf = (*e->writes)[i];
    unsigned int field_count = wf ? vec_safe_length (wf->field_names) : 0;
    streamer_write_uhwi_stream (obs, field_count);
    for (unsigned int j = 0; j < field_count; j++) {
      streamer_write_uhwi_stream (obs, (*wf->field_uids)[j]);
      stream_write_string (obs, (*wf->field_names)[j]);
    }
  }
}

// ============================================================================
// Read a single LtoUnifiedResultSummary from input block
// ============================================================================

static LtoUnifiedResultSummary* read_summary_entry (class lto_input_block* ib) {
  LtoUnifiedResultSummary* e =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (e, 0, sizeof (*e));

  // Basic identity
  e->tu_source_file = stream_read_string (ib);
  e->type_uid = (unsigned int) streamer_read_uhwi (ib);
  e->ptr_field_uid = (unsigned int) streamer_read_uhwi (ib);
  e->type_name = stream_read_string (ib);
  e->ptr_field_name = stream_read_string (ib);
  e->owned_verdict = (OwnedConclusionVerdict) streamer_read_uhwi (ib);

  // Template args
  unsigned int tmpl_count = (unsigned int) streamer_read_uhwi (ib);
  if (tmpl_count > 0) {
    vec_alloc (e->template_args, tmpl_count);
    for (unsigned int i = 0; i < tmpl_count; i++) {
      vec_safe_push (e->template_args, stream_read_string (ib));
    }
  }

  // Malloc size fields
  unsigned int malloc_count = (unsigned int) streamer_read_uhwi (ib);
  if (malloc_count > 0) {
    vec_alloc (e->malloc_size_field_uids, malloc_count);
    vec_alloc (e->malloc_size_field_names, malloc_count);
    for (unsigned int i = 0; i < malloc_count; i++) {
      vec_safe_push (e->malloc_size_field_uids, (unsigned int) streamer_read_uhwi (ib));
      vec_safe_push (e->malloc_size_field_names, stream_read_string (ib));
    }
  }

  // Reads
  unsigned int reads_count = (unsigned int) streamer_read_uhwi (ib);
  if (reads_count > 0) {
    vec_alloc (e->reads, reads_count);
    for (unsigned int i = 0; i < reads_count; i++) {
      LtoRelatedFieldsSummary* rf =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (rf, 0, sizeof (*rf));
      unsigned int field_count = (unsigned int) streamer_read_uhwi (ib);
      if (field_count > 0) {
        vec_alloc (rf->field_uids, field_count);
        vec_alloc (rf->field_names, field_count);
        for (unsigned int j = 0; j < field_count; j++) {
          vec_safe_push (rf->field_uids, (unsigned int) streamer_read_uhwi (ib));
          vec_safe_push (rf->field_names, stream_read_string (ib));
        }
      }
      vec_safe_push (e->reads, rf);
    }
  }

  // Writes
  unsigned int writes_count = (unsigned int) streamer_read_uhwi (ib);
  if (writes_count > 0) {
    vec_alloc (e->writes, writes_count);
    for (unsigned int i = 0; i < writes_count; i++) {
      LtoRelatedFieldsSummary* wf =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (wf, 0, sizeof (*wf));
      unsigned int field_count = (unsigned int) streamer_read_uhwi (ib);
      if (field_count > 0) {
        vec_alloc (wf->field_uids, field_count);
        vec_alloc (wf->field_names, field_count);
        for (unsigned int j = 0; j < field_count; j++) {
          vec_safe_push (wf->field_uids, (unsigned int) streamer_read_uhwi (ib));
          vec_safe_push (wf->field_names, stream_read_string (ib));
        }
      }
      vec_safe_push (e->writes, wf);
    }
  }

  return e;
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

static void buf_write_summary_entry (vec<unsigned char, va_gc>*& buf, LtoUnifiedResultSummary* e) {
  if (!e) return;

  buf_write_string (buf, e->tu_source_file);
  buf_write_uhwi (buf, e->type_uid);
  buf_write_uhwi (buf, e->ptr_field_uid);
  buf_write_string (buf, e->type_name);
  buf_write_string (buf, e->ptr_field_name);
  buf_write_uhwi (buf, (unsigned HOST_WIDE_INT) e->owned_verdict);

  unsigned int tmpl_count = vec_safe_length (e->template_args);
  buf_write_uhwi (buf, tmpl_count);
  for (unsigned int i = 0; i < tmpl_count; i++) {
    buf_write_string (buf, (*e->template_args)[i]);
  }

  unsigned int malloc_count = vec_safe_length (e->malloc_size_field_names);
  buf_write_uhwi (buf, malloc_count);
  for (unsigned int i = 0; i < malloc_count; i++) {
    buf_write_uhwi (buf, (*e->malloc_size_field_uids)[i]);
    buf_write_string (buf, (*e->malloc_size_field_names)[i]);
  }

  unsigned int reads_count = vec_safe_length (e->reads);
  buf_write_uhwi (buf, reads_count);
  for (unsigned int i = 0; i < reads_count; i++) {
    LtoRelatedFieldsSummary* rf = (*e->reads)[i];
    unsigned int field_count = rf ? vec_safe_length (rf->field_names) : 0;
    buf_write_uhwi (buf, field_count);
    for (unsigned int j = 0; j < field_count; j++) {
      buf_write_uhwi (buf, (*rf->field_uids)[j]);
      buf_write_string (buf, (*rf->field_names)[j]);
    }
  }

  unsigned int writes_count = vec_safe_length (e->writes);
  buf_write_uhwi (buf, writes_count);
  for (unsigned int i = 0; i < writes_count; i++) {
    LtoRelatedFieldsSummary* wf = (*e->writes)[i];
    unsigned int field_count = wf ? vec_safe_length (wf->field_names) : 0;
    buf_write_uhwi (buf, field_count);
    for (unsigned int j = 0; j < field_count; j++) {
      buf_write_uhwi (buf, (*wf->field_uids)[j]);
      buf_write_string (buf, (*wf->field_names)[j]);
    }
  }
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

static LtoUnifiedResultSummary* buf_read_summary_entry (BufReader& r) {
  LtoUnifiedResultSummary* e =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (e, 0, sizeof (*e));

  e->tu_source_file = buf_read_string (r);
  e->type_uid = (unsigned int) buf_read_uhwi (r);
  e->ptr_field_uid = (unsigned int) buf_read_uhwi (r);
  e->type_name = buf_read_string (r);
  e->ptr_field_name = buf_read_string (r);
  e->owned_verdict = (OwnedConclusionVerdict) buf_read_uhwi (r);

  unsigned int tmpl_count = (unsigned int) buf_read_uhwi (r);
  if (tmpl_count > 0) {
    vec_alloc (e->template_args, tmpl_count);
    for (unsigned int i = 0; i < tmpl_count; i++) {
      vec_safe_push (e->template_args, buf_read_string (r));
    }
  }

  unsigned int malloc_count = (unsigned int) buf_read_uhwi (r);
  if (malloc_count > 0) {
    vec_alloc (e->malloc_size_field_uids, malloc_count);
    vec_alloc (e->malloc_size_field_names, malloc_count);
    for (unsigned int i = 0; i < malloc_count; i++) {
      vec_safe_push (e->malloc_size_field_uids, (unsigned int) buf_read_uhwi (r));
      vec_safe_push (e->malloc_size_field_names, buf_read_string (r));
    }
  }

  unsigned int reads_count = (unsigned int) buf_read_uhwi (r);
  if (reads_count > 0) {
    vec_alloc (e->reads, reads_count);
    for (unsigned int i = 0; i < reads_count; i++) {
      LtoRelatedFieldsSummary* rf =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (rf, 0, sizeof (*rf));
      unsigned int field_count = (unsigned int) buf_read_uhwi (r);
      if (field_count > 0) {
        vec_alloc (rf->field_uids, field_count);
        vec_alloc (rf->field_names, field_count);
        for (unsigned int j = 0; j < field_count; j++) {
          vec_safe_push (rf->field_uids, (unsigned int) buf_read_uhwi (r));
          vec_safe_push (rf->field_names, buf_read_string (r));
        }
      }
      vec_safe_push (e->reads, rf);
    }
  }

  unsigned int writes_count = (unsigned int) buf_read_uhwi (r);
  if (writes_count > 0) {
    vec_alloc (e->writes, writes_count);
    for (unsigned int i = 0; i < writes_count; i++) {
      LtoRelatedFieldsSummary* wf =
        (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
      memset (wf, 0, sizeof (*wf));
      unsigned int field_count = (unsigned int) buf_read_uhwi (r);
      if (field_count > 0) {
        vec_alloc (wf->field_uids, field_count);
        vec_alloc (wf->field_names, field_count);
        for (unsigned int j = 0; j < field_count; j++) {
          vec_safe_push (wf->field_uids, (unsigned int) buf_read_uhwi (r));
          vec_safe_push (wf->field_names, buf_read_string (r));
        }
      }
      vec_safe_push (e->writes, wf);
    }
  }

  return e;
}

// ============================================================================
// Public API: Write summary using raw LTO section API
// ============================================================================

ArrayDetectErrorCode writeArrayDetectLtoSummarySection (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = g_wpa_summaries;
  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] skip (no summaries)");
    AD_RETURNE (OK);
  }

  unsigned int count = summaries->length ();
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] writing %u summaries", count);

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
  lto_begin_section (kArrayDetectSectionName, false);
  if (buf && buf->length () > 0) {
    lto_write_data (buf->address (), buf->length ());
  }
  lto_end_section ();

  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] done writing %u summaries, %u bytes",
                  count, vec_safe_length (buf));
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// Public API: Read summary from raw data
// ============================================================================

ArrayDetectErrorCode readArrayDetectLtoSummarySections (AD_FUNC_ARGS, char const* data, size_t len) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;

  if (!data || len == 0) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] ERROR: no data");
    AD_RETURNE (INVALID_ARGUMENT);
  }

  AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] ENTRY, len=%zu", len);

  BufReader r = { data, len, 0 };

  unsigned HOST_WIDE_INT magic = buf_read_uhwi (r);
  if (magic != kSummaryMagic) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] bad magic 0x%llx",
                    (unsigned long long) magic);
    AD_RETURNE (INVALID_ARGUMENT);
  }

  unsigned HOST_WIDE_INT version = buf_read_uhwi (r);
  if (version != kSummaryVersion) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] unknown version %llu",
                    (unsigned long long) version);
    AD_RETURNE (INVALID_ARGUMENT);
  }

  unsigned HOST_WIDE_INT count = buf_read_uhwi (r);
  AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] %llu entries",
                  (unsigned long long) count);

  unsigned int total_summaries = 0;
  for (unsigned HOST_WIDE_INT i = 0; i < count && r.pos < r.len; i++) {
    LtoUnifiedResultSummary* e = buf_read_summary_entry (r);
    if (e) {
      appendLtransLtoSummaries_single (e);
      total_summaries++;
    }
  }

  AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] EXIT summaries=%u", total_summaries);
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
  summary->type_uid = conclusion->type ? TYPE_UID (conclusion->type) : 0;
  summary->ptr_field_uid = conclusion->field_decl ? DECL_UID (conclusion->field_decl) : 0;
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
    // Malloc capacity fields
    if (tfad->malloc_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->malloc_evidences_map->begin ();
           iter != tfad->malloc_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->malloc_size_field_uids, count);
        vec_alloc (summary->malloc_size_field_names, count);
        for (auto iter = tfad->malloc_evidences_map->begin ();
             iter != tfad->malloc_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          vec_safe_push (summary->malloc_size_field_uids,
                         integer_field ? DECL_UID (integer_field) : 0u);
          char const* name = safeGetFieldName (AD_ARGS, integer_field);
          vec_safe_push (summary->malloc_size_field_names, dup_cstr (name));
        }
      }
    }

    // Read evidence (grouped by integer_field)
    if (tfad->read_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->read_evidences_map->begin ();
           iter != tfad->read_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->reads, count);
        for (auto iter = tfad->read_evidences_map->begin ();
             iter != tfad->read_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          LtoRelatedFieldsSummary* rf =
            (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
          memset (rf, 0, sizeof (*rf));
          vec_alloc (rf->field_uids, 1);
          vec_alloc (rf->field_names, 1);
          vec_safe_push (rf->field_uids, integer_field ? DECL_UID (integer_field) : 0u);
          char const* name = safeGetFieldName (AD_ARGS, integer_field);
          vec_safe_push (rf->field_names, dup_cstr (name));
          vec_safe_push (summary->reads, rf);
        }
      }
    }

    // Write evidence (grouped by integer_field)
    if (tfad->write_evidences_map) {
      unsigned int count = 0;
      for (auto iter = tfad->write_evidences_map->begin ();
           iter != tfad->write_evidences_map->end ();
           ++iter) {
        count++;
      }
      if (count) {
        vec_alloc (summary->writes, count);
        for (auto iter = tfad->write_evidences_map->begin ();
             iter != tfad->write_evidences_map->end ();
             ++iter) {
          tree integer_field = (*iter).first;
          LtoRelatedFieldsSummary* wf =
            (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
          memset (wf, 0, sizeof (*wf));
          vec_alloc (wf->field_uids, 1);
          vec_alloc (wf->field_names, 1);
          vec_safe_push (wf->field_uids, integer_field ? DECL_UID (integer_field) : 0u);
          char const* name = safeGetFieldName (AD_ARGS, integer_field);
          vec_safe_push (wf->field_names, dup_cstr (name));
          vec_safe_push (summary->writes, wf);
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

  AD_DEBUG_PRINT ("[convertAllFieldOwnedConclusionsToLtoSummaries] converted %u conclusions",
                  vec_safe_length (summaries));
  return summaries;
}

} // namespace array_detect_ns
