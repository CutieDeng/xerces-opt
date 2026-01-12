#include "lto-summary.hh"

#include <cstring>

#include "context.hh"
#include "prelude.hh"
#include "stor-layout.h"
#include "owned-conclusion.hh"
#include "array-detector.hh"
#include "string-utils.hh"

namespace array_detect_ns {

static char const kSummaryMagic[] = "ADLTO2";
static char const kSummaryVarPrefix[] = "__array_detect_lto_summary_";
static unsigned int g_summary_var_seq = 0;
static bool g_wpa_summary_emitted = false;

// Module-owned pointers (GC-managed allocations).
static vec<LtoUnifiedResultSummary*, va_gc>* g_wpa_summaries = nullptr;
static vec<LtoUnifiedResultSummary*, va_gc>* g_ltrans_summaries = nullptr;
static bool g_ltrans_summaries_loaded = false;

static inline char const* dup_cstr (char const* s) {
  if (!s) return nullptr;
  size_t n = strlen (s);
  char* out = (char*) ggc_alloc_atomic (n + 1);
  memcpy (out, s, n + 1);
  return out;
}


namespace {

struct AdBuffer {
  char* data;
  size_t len;
  size_t cap;
};

static void ad_buffer_init (AdBuffer* buf) {
  buf->data = nullptr;
  buf->len = 0;
  buf->cap = 0;
}

static void ad_buffer_free (AdBuffer* buf) {
  if (!buf) return;
  buf->data = nullptr;
  buf->len = 0;
  buf->cap = 0;
}

static bool ad_buffer_reserve (AdBuffer* buf, size_t extra) {
  if (!buf) return false;
  if (extra == 0) return true;
  size_t needed = buf->len + extra;
  if (needed <= buf->cap) return true;

  size_t new_cap = buf->cap ? buf->cap : 256;
  while (new_cap < needed) {
    size_t next = new_cap * 2;
    if (next <= new_cap) return false;
    new_cap = next;
  }

  char* new_data = buf->data
    ? (char*) ggc_realloc (buf->data, new_cap)
    : (char*) ggc_alloc_atomic (new_cap);
  if (!new_data) return false;
  buf->data = new_data;
  buf->cap = new_cap;
  return true;
}

static bool ad_buffer_append (AdBuffer* buf, char const* data, size_t len) {
  if (!ad_buffer_reserve (buf, len)) return false;
  if (len) memcpy (buf->data + buf->len, data, len);
  buf->len += len;
  return true;
}

static bool ad_buffer_append_cstr (AdBuffer* buf, char const* s) {
  if (!s) return true;
  return ad_buffer_append (buf, s, strlen (s));
}

static bool ad_buffer_append_space (AdBuffer* buf) {
  return ad_buffer_append (buf, " ", 1);
}

static bool ad_buffer_append_newline (AdBuffer* buf) {
  return ad_buffer_append (buf, "\n", 1);
}

static bool ad_buffer_append_uint (AdBuffer* buf, unsigned HOST_WIDE_INT val) {
  char tmp[32];
  int n = snprintf (tmp, sizeof (tmp), "%llu", (unsigned long long) val);
  if (n <= 0) return false;
  return ad_buffer_append (buf, tmp, (size_t) n);
}

static bool ad_buffer_append_len_string (AdBuffer* buf, char const* s) {
  if (!s) s = "";
  size_t len = strlen (s);
  if (!ad_buffer_append_uint (buf, (unsigned HOST_WIDE_INT) len)) return false;
  if (!ad_buffer_append_space (buf)) return false;
  if (len && !ad_buffer_append (buf, s, len)) return false;
  return true;
}

static unsigned int ad_hash_string (char const* s) {
  unsigned int h = 2166136261u;
  if (!s) return h;
  for (; *s; ++s) {
    h ^= (unsigned char) *s;
    h *= 16777619u;
  }
  return h;
}

static bool ad_uint_to_u32 (unsigned HOST_WIDE_INT v, unsigned int* out) {
  if (!out) return false;
  if (v > (unsigned HOST_WIDE_INT) ~0u) return false;
  *out = (unsigned int) v;
  return true;
}

static bool ad_is_ws (char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

struct AdReader {
  char const* data;
  size_t len;
  size_t pos;
};

static void ad_reader_skip_ws (AdReader* r) {
  if (!r) return;
  while (r->pos < r->len && ad_is_ws (r->data[r->pos])) r->pos++;
}

static bool ad_reader_expect_token (AdReader* r, char const* token) {
  if (!r || !token) return false;
  size_t tok_len = strlen (token);
  ad_reader_skip_ws (r);
  if (r->pos + tok_len > r->len) return false;
  if (memcmp (r->data + r->pos, token, tok_len) != 0) return false;
  if (r->pos + tok_len < r->len) {
    char next = r->data[r->pos + tok_len];
    if (!ad_is_ws (next)) return false;
  }
  r->pos += tok_len;
  return true;
}

static bool ad_reader_read_uint (AdReader* r, unsigned HOST_WIDE_INT* out) {
  if (!r || !out) return false;
  ad_reader_skip_ws (r);
  unsigned HOST_WIDE_INT val = 0;
  size_t start = r->pos;
  while (r->pos < r->len) {
    char c = r->data[r->pos];
    if (c < '0' || c > '9') break;
    val = (val * 10) + (unsigned HOST_WIDE_INT) (c - '0');
    r->pos++;
  }
  if (r->pos == start) return false;
  *out = val;
  return true;
}

static char const* ad_reader_read_len_string (AdReader* r) {
  unsigned HOST_WIDE_INT len = 0;
  if (!ad_reader_read_uint (r, &len)) return nullptr;
  ad_reader_skip_ws (r);
  if (len > (r->len - r->pos)) return nullptr;
  char* out = (char*) ggc_alloc_atomic ((size_t) len + 1);
  if (len) memcpy (out, r->data + r->pos, (size_t) len);
  out[len] = '\0';
  r->pos += (size_t) len;
  return out;
}

static bool encode_summary_blob (
  AD_FUNC_ARGS,
  vec<LtoUnifiedResultSummary*, va_gc>* summaries,
  char** out_data,
  size_t* out_len
) {
  (void) ctx;
  (void) gcc_ctx;
  if (!out_data || !out_len) return false;
  *out_data = nullptr;
  *out_len = 0;
  if (!summaries || summaries->is_empty ()) return false;

  char const* tu_file = "";
  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* s = (*summaries)[i];
    if (s && s->tu_source_file) {
      tu_file = s->tu_source_file;
      break;
    }
  }

  unsigned HOST_WIDE_INT entry_count = 0;
  for (unsigned i = 0; i < summaries->length (); i++) {
    if ((*summaries)[i]) entry_count++;
  }

  AdBuffer buf;
  ad_buffer_init (&buf);

  if (!ad_buffer_append_cstr (&buf, kSummaryMagic) ||
      !ad_buffer_append_newline (&buf)) {
    ad_buffer_free (&buf);
    return false;
  }

  if (!ad_buffer_append_cstr (&buf, "TU ") ||
      !ad_buffer_append_len_string (&buf, tu_file) ||
      !ad_buffer_append_newline (&buf)) {
    ad_buffer_free (&buf);
    return false;
  }

  if (!ad_buffer_append_cstr (&buf, "N ") ||
      !ad_buffer_append_uint (&buf, entry_count) ||
      !ad_buffer_append_newline (&buf)) {
    ad_buffer_free (&buf);
    return false;
  }

  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* e = (*summaries)[i];
    if (!e) continue;

    char const* type_name = e->type_name ? e->type_name : "";
    char const* field_name = e->ptr_field_name ? e->ptr_field_name : "";

    if (!ad_buffer_append_cstr (&buf, "E ") ||
        !ad_buffer_append_len_string (&buf, type_name) ||
        !ad_buffer_append_cstr (&buf, " T ") ||
        !ad_buffer_append_uint (&buf, vec_safe_length (e->template_args))) {
      ad_buffer_free (&buf);
      return false;
    }

    if (e->template_args) {
      for (unsigned j = 0; j < e->template_args->length (); j++) {
        if (!ad_buffer_append_space (&buf) ||
            !ad_buffer_append_len_string (&buf, (*e->template_args)[j])) {
          ad_buffer_free (&buf);
          return false;
        }
      }
    }

    if (!ad_buffer_append_cstr (&buf, " F ") ||
        !ad_buffer_append_len_string (&buf, field_name) ||
        !ad_buffer_append_cstr (&buf, " V ") ||
        !ad_buffer_append_uint (&buf, (unsigned HOST_WIDE_INT) e->owned_verdict) ||
        !ad_buffer_append_cstr (&buf, " M ") ||
        !ad_buffer_append_uint (&buf, vec_safe_length (e->malloc_size_field_names))) {
      ad_buffer_free (&buf);
      return false;
    }

    if (e->malloc_size_field_names) {
      for (unsigned j = 0; j < e->malloc_size_field_names->length (); j++) {
        if (!ad_buffer_append_space (&buf) ||
            !ad_buffer_append_len_string (&buf, (*e->malloc_size_field_names)[j])) {
          ad_buffer_free (&buf);
          return false;
        }
      }
    }

    if (!ad_buffer_append_cstr (&buf, " R ") ||
        !ad_buffer_append_uint (&buf, vec_safe_length (e->reads))) {
      ad_buffer_free (&buf);
      return false;
    }

    if (e->reads) {
      for (unsigned j = 0; j < e->reads->length (); j++) {
        LtoRelatedFieldsSummary* rf = (*e->reads)[j];
        unsigned int field_count = rf && rf->field_names ? rf->field_names->length () : 0;
        if (!ad_buffer_append_space (&buf) ||
            !ad_buffer_append_uint (&buf, field_count)) {
          ad_buffer_free (&buf);
          return false;
        }
        if (field_count && rf && rf->field_names) {
          for (unsigned k = 0; k < rf->field_names->length (); k++) {
            if (!ad_buffer_append_space (&buf) ||
                !ad_buffer_append_len_string (&buf, (*rf->field_names)[k])) {
              ad_buffer_free (&buf);
              return false;
            }
          }
        }
      }
    }

    if (!ad_buffer_append_cstr (&buf, " W ") ||
        !ad_buffer_append_uint (&buf, vec_safe_length (e->writes))) {
      ad_buffer_free (&buf);
      return false;
    }

    if (e->writes) {
      for (unsigned j = 0; j < e->writes->length (); j++) {
        LtoRelatedFieldsSummary* wf = (*e->writes)[j];
        unsigned int field_count = wf && wf->field_names ? wf->field_names->length () : 0;
        if (!ad_buffer_append_space (&buf) ||
            !ad_buffer_append_uint (&buf, field_count)) {
          ad_buffer_free (&buf);
          return false;
        }
        if (field_count && wf && wf->field_names) {
          for (unsigned k = 0; k < wf->field_names->length (); k++) {
            if (!ad_buffer_append_space (&buf) ||
                !ad_buffer_append_len_string (&buf, (*wf->field_names)[k])) {
              ad_buffer_free (&buf);
              return false;
            }
          }
        }
      }
    }

    if (!ad_buffer_append_newline (&buf)) {
      ad_buffer_free (&buf);
      return false;
    }
  }

  *out_data = buf.data;
  *out_len = buf.len;
  return true;
}

static bool decode_summary_blob (
  AD_FUNC_ARGS,
  char const* data,
  size_t len,
  vec<LtoUnifiedResultSummary*, va_gc>** out_summaries
) {
  (void) ctx;
  (void) gcc_ctx;
  if (!out_summaries) return false;
  *out_summaries = nullptr;
  if (!data || !len) return false;

  AdReader r = { data, len, 0 };
  if (!ad_reader_expect_token (&r, kSummaryMagic)) return false;
  if (!ad_reader_expect_token (&r, "TU")) return false;
  char const* tu_file = ad_reader_read_len_string (&r);
  if (!tu_file) return false;
  if (!ad_reader_expect_token (&r, "N")) return false;

  unsigned HOST_WIDE_INT count_hw = 0;
  if (!ad_reader_read_uint (&r, &count_hw)) return false;
  unsigned int count = 0;
  if (!ad_uint_to_u32 (count_hw, &count)) return false;

  vec<LtoUnifiedResultSummary*, va_gc>* summaries = nullptr;
  if (count) vec_alloc (summaries, count);

  for (unsigned int i = 0; i < count; i++) {
    if (!ad_reader_expect_token (&r, "E")) return false;
    char const* type_name = ad_reader_read_len_string (&r);
    if (!type_name) return false;

    if (!ad_reader_expect_token (&r, "T")) return false;
    unsigned HOST_WIDE_INT tmpl_hw = 0;
    if (!ad_reader_read_uint (&r, &tmpl_hw)) return false;
    unsigned int tmpl_count = 0;
    if (!ad_uint_to_u32 (tmpl_hw, &tmpl_count)) return false;
    vec<char const*, va_gc>* template_args = nullptr;
    if (tmpl_count) {
      vec_alloc (template_args, tmpl_count);
      for (unsigned int j = 0; j < tmpl_count; j++) {
        char const* arg = ad_reader_read_len_string (&r);
        if (!arg) return false;
        vec_safe_push (template_args, arg);
      }
    }

    if (!ad_reader_expect_token (&r, "F")) return false;
    char const* field_name = ad_reader_read_len_string (&r);
    if (!field_name) return false;

    if (!ad_reader_expect_token (&r, "V")) return false;
    unsigned HOST_WIDE_INT verdict_hw = 0;
    if (!ad_reader_read_uint (&r, &verdict_hw)) return false;
    unsigned int verdict = 0;
    if (!ad_uint_to_u32 (verdict_hw, &verdict)) return false;

    if (!ad_reader_expect_token (&r, "M")) return false;
    unsigned HOST_WIDE_INT m_hw = 0;
    if (!ad_reader_read_uint (&r, &m_hw)) return false;
    unsigned int mcount = 0;
    if (!ad_uint_to_u32 (m_hw, &mcount)) return false;
    vec<char const*, va_gc>* malloc_names = nullptr;
    vec<unsigned int, va_gc>* malloc_uids = nullptr;
    if (mcount) {
      vec_alloc (malloc_names, mcount);
      vec_alloc (malloc_uids, mcount);
      for (unsigned int j = 0; j < mcount; j++) {
        char const* name = ad_reader_read_len_string (&r);
        if (!name) return false;
        vec_safe_push (malloc_names, name);
        vec_safe_push (malloc_uids, 0u);
      }
    }

    if (!ad_reader_expect_token (&r, "R")) return false;
    unsigned HOST_WIDE_INT r_hw = 0;
    if (!ad_reader_read_uint (&r, &r_hw)) return false;
    unsigned int rcount = 0;
    if (!ad_uint_to_u32 (r_hw, &rcount)) return false;
    vec<LtoRelatedFieldsSummary*, va_gc>* reads = nullptr;
    if (rcount) {
      vec_alloc (reads, rcount);
      for (unsigned int j = 0; j < rcount; j++) {
        unsigned HOST_WIDE_INT rf_hw = 0;
        if (!ad_reader_read_uint (&r, &rf_hw)) return false;
        unsigned int fcount = 0;
        if (!ad_uint_to_u32 (rf_hw, &fcount)) return false;
        LtoRelatedFieldsSummary* rf =
          (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
        memset (rf, 0, sizeof (*rf));
        if (fcount) {
          vec_alloc (rf->field_names, fcount);
          vec_alloc (rf->field_uids, fcount);
          for (unsigned int k = 0; k < fcount; k++) {
            char const* name = ad_reader_read_len_string (&r);
            if (!name) return false;
            vec_safe_push (rf->field_names, name);
            vec_safe_push (rf->field_uids, 0u);
          }
        }
        vec_safe_push (reads, rf);
      }
    }

    if (!ad_reader_expect_token (&r, "W")) return false;
    unsigned HOST_WIDE_INT w_hw = 0;
    if (!ad_reader_read_uint (&r, &w_hw)) return false;
    unsigned int wcount = 0;
    if (!ad_uint_to_u32 (w_hw, &wcount)) return false;
    vec<LtoRelatedFieldsSummary*, va_gc>* writes = nullptr;
    if (wcount) {
      vec_alloc (writes, wcount);
      for (unsigned int j = 0; j < wcount; j++) {
        unsigned HOST_WIDE_INT wf_hw = 0;
        if (!ad_reader_read_uint (&r, &wf_hw)) return false;
        unsigned int fcount = 0;
        if (!ad_uint_to_u32 (wf_hw, &fcount)) return false;
        LtoRelatedFieldsSummary* wf =
          (LtoRelatedFieldsSummary*) ggc_alloc_atomic (sizeof (LtoRelatedFieldsSummary));
        memset (wf, 0, sizeof (*wf));
        if (fcount) {
          vec_alloc (wf->field_names, fcount);
          vec_alloc (wf->field_uids, fcount);
          for (unsigned int k = 0; k < fcount; k++) {
            char const* name = ad_reader_read_len_string (&r);
            if (!name) return false;
            vec_safe_push (wf->field_names, name);
            vec_safe_push (wf->field_uids, 0u);
          }
        }
        vec_safe_push (writes, wf);
      }
    }

    LtoUnifiedResultSummary* summary =
      (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
    memset (summary, 0, sizeof (*summary));
    summary->tu_source_file = tu_file;
    summary->type_uid = 0;
    summary->ptr_field_uid = 0;
    summary->type_name = type_name;
    summary->template_args = template_args;
    summary->ptr_field_name = field_name;
    summary->owned_verdict = (OwnedConclusionVerdict) verdict;
    summary->malloc_size_field_uids = malloc_uids;
    summary->malloc_size_field_names = malloc_names;
    summary->reads = reads;
    summary->writes = writes;

    vec_safe_push (summaries, summary);
  }

  *out_summaries = summaries;
  return true;
}

static bool is_summary_var_decl (tree decl) {
  if (!decl || TREE_CODE (decl) != VAR_DECL) return false;
  tree name = DECL_NAME (decl);
  if (!name) return false;
  char const* ident = IDENTIFIER_POINTER (name);
  if (!ident) return false;
  size_t prefix_len = strlen (kSummaryVarPrefix);
  return strncmp (ident, kSummaryVarPrefix, prefix_len) == 0;
}

static void emit_summary_var (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  if (g_wpa_summary_emitted) return;

  vec<LtoUnifiedResultSummary*, va_gc>* summaries = g_wpa_summaries;
  if (!summaries || summaries->is_empty ()) return;

  char const* tu_file = "";
  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* s = (*summaries)[i];
    if (s && s->tu_source_file) {
      tu_file = s->tu_source_file;
      break;
    }
  }

  char* payload = nullptr;
  size_t payload_len = 0;
  if (!encode_summary_blob (AD_ARGS, summaries, &payload, &payload_len)) {
    AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] encode failed");
    return;
  }
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] emit start, len=%zu", payload_len);
  if (payload_len == 0 || payload_len > (size_t) ~0u) {
    AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] payload too large: %zu", payload_len);
    return;
  }

  char name_buf[128];
  unsigned int h = ad_hash_string (tu_file);
  unsigned int seq = ++g_summary_var_seq;
  int n = snprintf (name_buf, sizeof (name_buf), "%s%08x_%u", kSummaryVarPrefix, h, seq);
  if (n <= 0 || (size_t) n >= sizeof (name_buf)) {
    AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] summary name too long");
    return;
  }

  tree str = build_string ((unsigned int) payload_len, payload);
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] build_string done");
  tree decl = build_decl (UNKNOWN_LOCATION, VAR_DECL, get_identifier (name_buf), TREE_TYPE (str));
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] build_decl done");

  TREE_STATIC (decl) = 1;
  TREE_READONLY (decl) = 1;
  TREE_CONSTANT (decl) = 1;
  TREE_PUBLIC (decl) = 0;
  DECL_EXTERNAL (decl) = 0;
  DECL_ARTIFICIAL (decl) = 1;
  DECL_IGNORED_P (decl) = 1;
  DECL_PRESERVE_P (decl) = 1;
  TREE_USED (decl) = 1;
  DECL_INITIAL (decl) = str;
  DECL_CONTEXT (decl) = NULL_TREE;
  layout_decl (decl, 0);
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] layout_decl done");

  varpool_node::add (decl);
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] varpool add done");
  varpool_node::finalize_decl (decl);
  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] finalize done");

  AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] emitted summary var %s (%u bytes)",
                  name_buf, (unsigned int) payload_len);
  g_wpa_summary_emitted = true;
}

} // namespace

void clearWpaLtoSummaries () {
  g_wpa_summaries = nullptr;
  g_wpa_summary_emitted = false;
  g_summary_var_seq = 0;
}
void clearLtransLtoSummaries () {
  g_ltrans_summaries = nullptr;
  g_ltrans_summaries_loaded = false;
}

void setWpaLtoSummaries (vec<LtoUnifiedResultSummary*, va_gc>* summaries) {
  g_wpa_summaries = summaries;
  g_wpa_summary_emitted = false;
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

void writeArrayDetectLtoSummarySection (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = g_wpa_summaries;
  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[writeArrayDetectLtoSummarySection] skip (no summaries)");
    return;
  }

  emit_summary_var (AD_ARGS);
}

void readArrayDetectLtoSummarySections (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  if (g_ltrans_summaries_loaded) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] already loaded, skipping");
    return;
  }
  AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] ENTRY");
  if (!symtab) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] symtab=NULL, skipping");
    g_ltrans_summaries_loaded = true;
    return;
  }

  unsigned int vars_seen = 0;
  unsigned int entries_total = 0;
  for (varpool_node* vnode = symtab->first_variable ();
       vnode;
       vnode = symtab->next_variable (vnode)) {
    tree decl = vnode->decl;
    if (!is_summary_var_decl (decl)) continue;
    vars_seen++;

    char const* var_name = "<anon>";
    tree decl_name = DECL_NAME (decl);
    if (decl_name) {
      var_name = IDENTIFIER_POINTER (decl_name);
    }

    tree init = DECL_INITIAL (decl);
    if (!init || TREE_CODE (init) != STRING_CST) {
      AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] skip var %s (no STRING_CST)", var_name);
      continue;
    }

    char const* blob = TREE_STRING_POINTER (init);
    size_t blob_len = (size_t) TREE_STRING_LENGTH (init);
    vec<LtoUnifiedResultSummary*, va_gc>* file_summaries = nullptr;
    if (!decode_summary_blob (AD_ARGS, blob, blob_len, &file_summaries)) {
      AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] decode failed: %s", var_name);
      continue;
    }

    appendLtransLtoSummaries (file_summaries);
    entries_total += (unsigned int) vec_safe_length (file_summaries);
  }

  if (!vars_seen) {
    AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] no summary vars found");
  }

  AD_DEBUG_PRINT ("[readArrayDetectLtoSummarySections] EXIT summaries=%u vars=%u",
                  entries_total, vars_seen);
  g_ltrans_summaries_loaded = true;
}

// ============================================================================
// Conversion from FieldOwnedConclusion to LtoUnifiedResultSummary
// ============================================================================

LtoUnifiedResultSummary* convertFieldOwnedConclusionToLtoSummary (
  AD_FUNC_ARGS,
  FieldOwnedConclusion* conclusion,
  ::field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
) {
  if (!conclusion) return nullptr;

  LtoUnifiedResultSummary* summary =
    (LtoUnifiedResultSummary*) ggc_alloc_atomic (sizeof (LtoUnifiedResultSummary));
  memset (summary, 0, sizeof (*summary));

  // === 基本标识信息 ===
  summary->tu_source_file = dup_cstr (main_input_filename);
  summary->type_uid = conclusion->type ? TYPE_UID (conclusion->type) : 0;
  summary->ptr_field_uid = conclusion->field_decl ? DECL_UID (conclusion->field_decl) : 0;
  summary->type_name = dup_cstr (conclusion->type_name ? conclusion->type_name : "<anon>");
  summary->ptr_field_name = dup_cstr (conclusion->field_name ? conclusion->field_name : "<anon>");
  summary->owned_verdict = conclusion->verdict;

  // === 模板参数 (如果类型是模板实例) ===
  summary->template_args = nullptr;
  // TODO: 从 conclusion->type 提取模板参数 (如果需要)

  // === 从 tfad 提取 capacity 证据 ===
  if (tfad) {
    // 提取 malloc 容量关联的整数字段名称
    if (tfad->malloc_evidences_map) {
      unsigned int count = 0;
      // 先统计数量
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

    // 提取 read 证据 (按 integer_field 分组)
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

    // 提取 write 证据 (按 integer_field 分组)
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

    // 查找对应的 TypeFieldAnalysisData
    ::field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude* tfad = nullptr;
    if (detector.m_type_field_writes && conclusion->type && conclusion->field_decl) {
      ::array_detector::TypeFieldKey key = {
        TYPE_MAIN_VARIANT (conclusion->type),
        conclusion->field_decl
      };
      ::field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude** slot =
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
