#include <stdlib.h>

#include "gcc-common.hh"
#include "plugin-version.h"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "array-detector.hh"
#include "array-detect-context-gcc.hh"
#include "pipeline.hh"
#include "lto-summary.hh"
#include "lto-transform.hh"

#include "gcc-ext-util.hh"
#include "info.hh"
#include "info-print.hh"

// For LTO detection
#include "lto-streamer.h"

// ----------------------------------------------------------------------------
// Pass 注册结构
// ----------------------------------------------------------------------------

namespace array_detect_ns {

// Run entry: sets up runtime/context then hands off to the analyzer.
ArrayDetectErrorCode runArrayDetect (AD_FUNC_ARGS) AD_FUNCTION_BEGIN2 {
  initGccContext (AD_ARGS);
  // AD_ETRY2 (initContextWithStderr (AD_ARGS), cleanup, false, true, "Failed to init context: %s");
  AD_ETRY2 (initContextAdaptive (AD_ARGS), cleanup, false, true, "Failed to init context: %s");
  AD_TRY (runArrayDetectorAnalysis (AD_ARGS));
  AD_RETURNE (OK);
  cleanup:
  deinitContext (AD_ARGS);
} AD_FUNCTION_END3

}

namespace {

// ============================================================================
// LTO hook functions
// ============================================================================

// Store results from execute() for later serialization
static vec<::array_detect_ns::UnifiedFieldAnalysisResult*, va_gc>* g_wpa_results = nullptr;

// Called after execute() to generate summary data
static void ipa_generate_summary (void) {
  char const* debug_file = getenv ("AD_DEBUG_FILE");
  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[ipa_generate_summary] called, g_wpa_results=%p\n", (void*)g_wpa_results);
      fclose (df);
    }
  }

  // Results already collected in execute(), convert to LTO summary format
  if (g_wpa_results && !g_wpa_results->is_empty ()) {
    // Get current source file name
    char const* tu_file = main_input_filename ? main_input_filename : "<unknown>";

    vec<::array_detect_ns::LtoUnifiedResultSummary*, va_gc>* summaries =
      ::array_detect_ns::convertAllToLtoSummaries (g_wpa_results, tu_file);

    ::array_detect_ns::setWpaLtoSummaries (summaries);
  }
}

// Called to write summary to LTO section
static void ipa_write_summary (void) {
  char const* debug_file = getenv ("AD_DEBUG_FILE");
  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[ipa_write_summary] called\n");
      fclose (df);
    }
  }
  ::array_detect_ns::writeArrayDetectLtoSummarySection ();
}

// Called in LTRANS to read summaries from all input files
static void ipa_read_summary (void) {
  char const* debug_file = getenv ("AD_DEBUG_FILE");
  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[ipa_read_summary] called\n");
      fclose (df);
    }
  }

  // NOTE: LTO section reading with custom section ID (100) causes GCC to crash
  // because GCC's internal arrays are sized to LTO_N_SECTION_TYPES (~23).
  // We use file-based approach instead (AD_RESULT_FILE).
  // The file-based data is read in initLtoTransformContext via parseResultFileForOwnedFields.
}

// Called for each function during LTRANS to apply transformations
static unsigned int ipa_function_transform (cgraph_node* node) {
  char const* debug_file = getenv ("AD_DEBUG_FILE");
  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[ipa_function_transform] ENTRY, in_lto_p=%d, flag_ltrans=%d, node=%p\n",
               in_lto_p, flag_ltrans, (void*)node);
      fclose (df);
    }
  }

  // Only transform in LTRANS phase
  if (!in_lto_p || !flag_ltrans) return 0;

  function* fn = node->get_fun ();
  if (!fn) return 0;

  char const* fn_name = node->name ();

  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[ipa_function_transform] processing function: %s\n",
               fn_name ? fn_name : "<anon>");
      fclose (df);
    }
  }

  return ::array_detect_ns::runLtoTransform (fn);
}

// ============================================================================
// Pass data and class
// ============================================================================

const pass_data array_detect_pass_data = {
  .type = IPA_PASS,
  .name = "array-detect-wpa-pass",
  .optinfo_flags = OPTGROUP_NONE,
  .tv_id = TV_NONE,
  .properties_required = 0,
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0,
};

class pass_array_detect : public ipa_opt_pass_d {
 public:
  pass_array_detect (gcc::context * ctxt)
      : ipa_opt_pass_d (array_detect_pass_data, ctxt,
                       ipa_generate_summary,     // generate_summary
                       ipa_write_summary,        // write_summary
                       ipa_read_summary,         // read_summary
                       NULL,                     // write_optimization_summary
                       NULL,                     // read_optimization_summary
                       NULL,                     // stmt_fixup
                       TODO_update_ssa_only_virtuals,  // function_transform_todo_flags_start
                       ipa_function_transform,   // function_transform
                       NULL)                     // variable_transform
  {}

  opt_pass * clone () override { return new pass_array_detect (g); }

  // Gate function: must return true for IPA summary hooks to work
  bool gate (function* /*fn*/) override {
    // IMPORTANT: Always return true during LTO phases to enable summary hooks
    // - Regular compilation: run analysis
    // - WPA phase: write_summary needs gate=true to be called
    // - LTRANS phase: read_summary and function_transform need gate=true
    return true;
  }

  unsigned int execute (function* /*fn*/) override {
    char const* debug_file = getenv ("AD_DEBUG_FILE");
    if (debug_file) {
      FILE* df = fopen (debug_file, "a");
      if (df) {
        fprintf (df, "[execute] called, in_lto_p=%d, flag_ltrans=%d\n", in_lto_p, flag_ltrans);
        fclose (df);
      }
    }

    // Skip analysis during LTO WPA/LTRANS phases - we only need the summary hooks
    // Analysis was already done during initial compilation
    if (in_lto_p) {
      // Aggregated results are written in plugin_finish_callback
      return 0;
    }

    // Regular compilation: run analysis
    ::array_detect_ns::ArrayDetectContext local_ctx;
    ::array_detect_ns::ArrayDetectContextGcc local_gcc_ctx;
    ::array_detect_ns::ArrayDetectErrorCode result = runArrayDetect (local_ctx, local_gcc_ctx);

    // Store results for generate_summary (cast from void* stored in context)
    g_wpa_results = (vec<::array_detect_ns::UnifiedFieldAnalysisResult*, va_gc>*) local_ctx.unified_results;

    return result != ::array_detect_ns::OK;
  }
};

// ============================================================================
// PLUGIN_FINISH callback for LTO aggregation output
// ============================================================================

// File-based LTO aggregation: read existing results and aggregate
static void aggregateFileResults (char const* result_file, char const* debug_file) {
  FILE* df = debug_file ? fopen (debug_file, "a") : nullptr;

  FILE* rf = fopen (result_file, "r");
  if (!rf) {
    if (df) { fprintf (df, "[aggregateFileResults] Cannot open result file\n"); fclose (df); }
    return;
  }

  // Parse and aggregate by (type, field)
  struct AggEntry {
    char type[256];
    char field[256];
    char owned[32];
    char malloc_fields[512];    // comma-separated field names
    char read_bounds[512];      // comma-separated bound field names
    char write_bounds[512];     // comma-separated bound field names
  };

  AggEntry entries[64];
  int entry_count = 0;

  char line[4096];
  while (fgets (line, sizeof(line), rf) && entry_count < 64) {
    char type_str[256] = "", field_str[256] = "";
    char owned_str[32] = "maybe";
    char malloc_str[256] = "", reads_str[256] = "", writes_str[256] = "";

    // Parse type and field
    char* p;
    if ((p = strstr(line, "(type \""))) sscanf(p, "(type \"%255[^\"]\")", type_str);
    if ((p = strstr(line, "(field \""))) sscanf(p, "(field \"%255[^\"]\")", field_str);
    if ((p = strstr(line, "(owned "))) sscanf(p, "(owned %31[^)])", owned_str);

    // Extract inner content of (malloc-size (...))
    if ((p = strstr(line, "(malloc-size ("))) {
      p += 14; // skip "(malloc-size ("
      char* end = strchr(p, ')');
      if (end) { int len = end - p; if (len < 256) { strncpy(malloc_str, p, len); malloc_str[len] = 0; } }
    }

    // Extract reads bounds - look for patterns like (("field1" "field2"))
    if ((p = strstr(line, "(reads ("))) {
      p += 8; // skip "(reads ("
      char* end = p; int depth = 1;
      while (*end && depth > 0) { if (*end == '(') depth++; else if (*end == ')') depth--; end++; }
      end--; // back to closing paren
      int len = end - p; if (len > 0 && len < 256) { strncpy(reads_str, p, len); reads_str[len] = 0; }
    }

    // Extract writes bounds
    if ((p = strstr(line, "(writes ("))) {
      p += 9; // skip "(writes ("
      char* end = p; int depth = 1;
      while (*end && depth > 0) { if (*end == '(') depth++; else if (*end == ')') depth--; end++; }
      end--;
      int len = end - p; if (len > 0 && len < 256) { strncpy(writes_str, p, len); writes_str[len] = 0; }
    }

    if (type_str[0] == 0 || field_str[0] == 0) continue;

    // Find or create entry
    int idx = -1;
    for (int i = 0; i < entry_count; i++) {
      if (strcmp(entries[i].type, type_str) == 0 && strcmp(entries[i].field, field_str) == 0) {
        idx = i; break;
      }
    }

    if (idx < 0) {
      idx = entry_count++;
      strcpy(entries[idx].type, type_str);
      strcpy(entries[idx].field, field_str);
      strcpy(entries[idx].owned, owned_str);
      strcpy(entries[idx].malloc_fields, malloc_str);
      strcpy(entries[idx].read_bounds, reads_str);
      strcpy(entries[idx].write_bounds, writes_str);
    } else {
      // Merge owned: YES > NO > UNDETERMINED
      if (strcmp(owned_str, "yes") == 0) strcpy(entries[idx].owned, "yes");
      else if (strcmp(owned_str, "no") == 0 && strcmp(entries[idx].owned, "yes") != 0)
        strcpy(entries[idx].owned, "no");

      // Merge malloc-size (take first non-empty)
      if (malloc_str[0] && !entries[idx].malloc_fields[0])
        strcpy(entries[idx].malloc_fields, malloc_str);

      // Merge reads (append if different and has content)
      if (reads_str[0] && strlen(entries[idx].read_bounds) + strlen(reads_str) < 500) {
        if (entries[idx].read_bounds[0]) strcat(entries[idx].read_bounds, " ");
        strcat(entries[idx].read_bounds, reads_str);
      }

      // Merge writes
      if (writes_str[0] && strlen(entries[idx].write_bounds) + strlen(writes_str) < 500) {
        if (entries[idx].write_bounds[0]) strcat(entries[idx].write_bounds, " ");
        strcat(entries[idx].write_bounds, writes_str);
      }
    }
  }
  fclose (rf);

  if (df) fprintf(df, "[aggregateFileResults] Parsed %d unique (type, field) entries\n", entry_count);

  // Write aggregated results
  FILE* out = fopen (result_file, "w");
  if (!out) { if (df) fclose(df); return; }

  fprintf(out, ";; LTO Aggregated Results\n");
  for (int i = 0; i < entry_count; i++) {
    AggEntry* e = &entries[i];
    fprintf(out, "((type \"%s\")(field \"%s\")(owned %s)", e->type, e->field, e->owned);
    fprintf(out, "(malloc-size (%s))", e->malloc_fields);
    fprintf(out, "(reads (%s))", e->read_bounds);
    fprintf(out, "(writes (%s)))\n", e->write_bounds);
  }
  fclose(out);

  if (df) { fprintf(df, "[aggregateFileResults] Wrote aggregated results\n"); fclose(df); }
}

static void plugin_finish_callback (void* /*gcc_data*/, void* /*user_data*/) {
  char const* debug_file = getenv ("AD_DEBUG_FILE");
  char const* result_file = getenv ("AD_RESULT_FILE");
  char const* aggregated_file = getenv ("AD_AGGREGATED_FILE");

  // Debug output
  if (debug_file) {
    FILE* df = fopen (debug_file, "a");
    if (df) {
      fprintf (df, "[plugin_finish_callback] in_lto_p=%d, flag_ltrans=%d\n",
               in_lto_p, flag_ltrans);
      fclose (df);
    }
  }

  // In LTRANS phase, aggregate file-based results and write to AD_AGGREGATED_FILE
  if (in_lto_p && flag_ltrans && result_file && aggregated_file) {
    // Aggregate the per-TU results from AD_RESULT_FILE
    aggregateFileResults (result_file, debug_file);

    // Write aggregated result to AD_AGGREGATED_FILE (overwrite mode)
    FILE* src = fopen (result_file, "r");
    FILE* dst = fopen (aggregated_file, "w");
    if (src && dst) {
      char buf[4096];
      size_t n;
      while ((n = fread (buf, 1, sizeof(buf), src)) > 0) {
        fwrite (buf, 1, n, dst);
      }
    }
    if (src) fclose (src);
    if (dst) fclose (dst);

    if (debug_file) {
      FILE* df = fopen (debug_file, "a");
      if (df) {
        fprintf (df, "[plugin_finish_callback] wrote aggregated results to: %s\n", aggregated_file);
        fclose (df);
      }
    }
  }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// 插件初始化
// ----------------------------------------------------------------------------

int plugin_init (struct plugin_name_args * plugin_info,
                struct plugin_gcc_version * version) {
  // 版本检查
  if (!plugin_default_version_check (version, &gcc_version)) {
    return 1;
  }

  struct register_pass_info pass_info;
  pass_info.pass = new pass_array_detect (g);
  pass_info.reference_pass_name = "inline";       // 在内联优化之后插入，以便分析内联后的代码
  pass_info.ref_pass_instance_number = 1;         // 符合规则
  pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback (plugin_info->base_name,
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,
                    &pass_info);

  // Register finish callback for LTO aggregation output
  register_callback (plugin_info->base_name,
                    PLUGIN_FINISH,
                    plugin_finish_callback,
                    NULL);

  return 0;
}

int plugin_is_GPL_compatible = 1;
