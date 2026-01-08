#include <stdlib.h>
#include <string.h>

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
#include "options.h"

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
static bool g_wpa_analysis_done = false;
static ::array_detect_ns::ArrayDetectContextGcc g_plugin_gcc_ctx;

static bool isWpaPhase () {
  return flag_wpa != nullptr;
}

static bool isTruthyEnv (char const* value) {
  if (!value || value[0] == '\0') return false;
  if (strcmp (value, "0") == 0) return false;
  if (strcmp (value, "false") == 0) return false;
  if (strcmp (value, "FALSE") == 0) return false;
  if (strcmp (value, "no") == 0) return false;
  if (strcmp (value, "NO") == 0) return false;
  return true;
}

static bool shouldEnableLtoSummaryBlob () {
  char const* env = getenv ("AD_ENABLE_LTO_SECTIONS");
  if (env) return isTruthyEnv (env);
  return true;
}

static ::array_detect_ns::ArrayDetectErrorCode runAnalysisAndStoreResults () {
  ::array_detect_ns::ArrayDetectContext local_ctx;
  ::array_detect_ns::ArrayDetectContextGcc local_gcc_ctx;
  ::array_detect_ns::ArrayDetectErrorCode result =
    ::array_detect_ns::runArrayDetect (local_ctx, local_gcc_ctx);

  g_wpa_results =
    (vec<::array_detect_ns::UnifiedFieldAnalysisResult*, va_gc>*) local_ctx.unified_results;
  g_wpa_analysis_done = true;
  return result;
}

// Called after execute() to generate summary data
static void ipa_generate_summary_impl (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[ipa_generate_summary] called, g_wpa_results=%p", (void*)g_wpa_results);

  if (isWpaPhase () && !g_wpa_results && !g_wpa_analysis_done) {
    if (!getenv ("AD_ALLOW_WPA_ANALYSIS")) {
      AD_DEBUG_PRINT ("[ipa_generate_summary] WPA: skip analysis (set AD_ALLOW_WPA_ANALYSIS=1 to enable)");
      g_wpa_analysis_done = true;
    } else {
      AD_DEBUG_PRINT ("[ipa_generate_summary] WPA: running analysis for summary");
      runAnalysisAndStoreResults ();
    }
  }

  if (!in_lto_p && flag_generate_lto && !g_wpa_results && !g_wpa_analysis_done) {
    AD_DEBUG_PRINT ("[ipa_generate_summary] LGEN: running analysis for summary");
    runAnalysisAndStoreResults ();
  }

  // Results already collected in execute(), convert to LTO summary format
  if (g_wpa_results && !g_wpa_results->is_empty ()) {
    // Get current source file name
    char const* tu_file = main_input_filename ? main_input_filename : (in_lto_p ? "<lto-wpa>" : "<unknown>");

    vec<::array_detect_ns::LtoUnifiedResultSummary*, va_gc>* summaries =
      ::array_detect_ns::convertAllToLtoSummaries (g_wpa_results, tu_file);

    ::array_detect_ns::setWpaLtoSummaries (summaries);
  }

  if (!in_lto_p && flag_generate_lto && shouldEnableLtoSummaryBlob ()) {
    ::array_detect_ns::writeArrayDetectLtoSummarySection (AD_ARGS);
  }
}

static void ipa_generate_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  ipa_generate_summary_impl (AD_ARGS);
}

// Called to write summary blob into LTO decls
static void ipa_write_summary_impl (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[ipa_write_summary] called");
  if (in_lto_p || !flag_generate_lto) {
    AD_DEBUG_PRINT ("[ipa_write_summary] skip (not LGEN)");
    return;
  }

  char const* enable_sections = getenv ("AD_ENABLE_LTO_SECTIONS");
  AD_DEBUG_PRINT ("[ipa_write_summary] AD_ENABLE_LTO_SECTIONS=%s",
                  enable_sections ? enable_sections : "<null>");
  if (!shouldEnableLtoSummaryBlob ()) {
    AD_DEBUG_PRINT ("[ipa_write_summary] skip LTO summary blob (set AD_ENABLE_LTO_SECTIONS=1 to enable)");
    return;
  }
  ::array_detect_ns::writeArrayDetectLtoSummarySection (AD_ARGS);
}

static void ipa_write_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  ipa_write_summary_impl (AD_ARGS);
}

// Called in LTRANS to read summaries from all input files
static void ipa_read_summary_impl (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[ipa_read_summary] called");
  if (!flag_ltrans) {
    AD_DEBUG_PRINT ("[ipa_read_summary] skip (not in LTRANS)");
    return;
  }

  // Load per-TU summaries from LTO decls for LTRANS.
  ::array_detect_ns::readArrayDetectLtoSummarySections (AD_ARGS);
}

static void ipa_read_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  ipa_read_summary_impl (AD_ARGS);
}

// ============================================================================
// LTRANS function_transform entry (IPA pass uses cgraph_node*).
// ============================================================================

static unsigned int runLtoTransformEntry (cgraph_node* node) {
  if (!flag_ltrans) return 0;
  if (!node) return 0;
  if (!node->get_body ()) return 0;
  function* fn = node->get_fun ();
  if (!fn) return 0;
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  return ::array_detect_ns::runLtoTransform (AD_ARGS, fn);
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

const pass_data array_detect_ltrans_pass_data = {
  .type = GIMPLE_PASS,
  .name = "array-detect-ltrans-pass",
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
                       0,                        // function_transform_todo_flags_start
                       runLtoTransformEntry,      // function_transform
                       NULL)                     // variable_transform
  {}

  opt_pass * clone () override { return new pass_array_detect (g); }

  // Gate function: must return true for IPA summary hooks to work
  bool gate (function* /*fn*/) override {
    // LGEN with -flto: allow opt-out.
    if (!in_lto_p && flag_generate_lto && getenv ("AD_SKIP_LTO_PASS")) {
      return false;
    }
    return true;
  }

  static unsigned int execute_impl (AD_FUNC_ARGS) {
    (void) gcc_ctx;
    AD_DEBUG_PRINT ("[execute] called, in_lto_p=%d, flag_ltrans=%d, flag_wpa=%s",
                    in_lto_p, flag_ltrans, flag_wpa ? flag_wpa : "<null>");

    if (flag_ltrans) {
      AD_DEBUG_PRINT ("[execute] skip (LTRANS)");
      return 0;
    }

    if (isWpaPhase ()) {
      AD_DEBUG_PRINT ("[execute] WPA: skip (no file aggregation)");
      return 0;
    }

    if (in_lto_p) return 0;

    // LGEN with -flto: analysis handled in ipa_generate_summary.
    if (flag_generate_lto) return 0;

    // Regular compilation: run analysis
    ::array_detect_ns::ArrayDetectErrorCode result = runAnalysisAndStoreResults ();
    return result != ::array_detect_ns::OK;
  }

  unsigned int execute (function* /*fn*/) override {
    ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
    ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
    return execute_impl (AD_ARGS);
  }
};

class pass_array_detect_ltrans : public gimple_opt_pass {
 public:
  pass_array_detect_ltrans (gcc::context * ctxt)
      : gimple_opt_pass (array_detect_ltrans_pass_data, ctxt) {}

  opt_pass * clone () override { return new pass_array_detect_ltrans (g); }

  bool gate (function* /*fn*/) override { return true; }

  static unsigned int execute_impl (AD_FUNC_ARGS, function* fn) {
    char const* fn_name = nullptr;
    if (fn && fn->decl && DECL_NAME (fn->decl)) {
      fn_name = IDENTIFIER_POINTER (DECL_NAME (fn->decl));
    }
    AD_DEBUG_PRINT ("[ltrans-pass] execute fn=%s flag_ltrans=%d",
                    fn_name ? fn_name : "<null>", flag_ltrans);
    if (!flag_ltrans || !fn) return 0;
    if (!gimple_has_body_p (fn->decl)) return 0;
    return ::array_detect_ns::runLtoTransform (AD_ARGS, fn);
  }

  unsigned int execute (function* fn) override {
    ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
    ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
    return execute_impl (AD_ARGS, fn);
  }
};


// ============================================================================
// PLUGIN_FINISH callback for LTO aggregation output (WPA)
// ============================================================================

static void plugin_finish_callback_impl (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[plugin_finish_callback] in_lto_p=%d, flag_ltrans=%d",
                  in_lto_p, flag_ltrans);

  if (in_lto_p && !flag_ltrans) {
    ::array_detect_ns::closeGlobalDebugFile ();
    return;
  }

  ::array_detect_ns::closeGlobalDebugFile ();
}

static void plugin_init_debug (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[plugin_init] in_lto_p=%d, flag_ltrans=%d, flag_generate_lto=%d, flag_wpa=%s",
                  in_lto_p, flag_ltrans, flag_generate_lto, flag_wpa ? flag_wpa : "<null>");
}

static void plugin_finish_callback (void* /*gcc_data*/, void* /*user_data*/) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  plugin_finish_callback_impl (AD_ARGS);
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

  ::array_detect_ns::initGlobalDebugFileFromEnv ();
  {
    ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
    ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
    plugin_init_debug (AD_ARGS);
  }

  struct register_pass_info pass_info;
  if (flag_ltrans) {
    pass_info.pass = new pass_array_detect_ltrans (g);
    pass_info.reference_pass_name = "optimized";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
  } else {
    pass_info.pass = new pass_array_detect (g);
    pass_info.reference_pass_name = "inline";       // 在内联优化之后插入，以便分析内联后的代码
    pass_info.ref_pass_instance_number = 1;         // 符合规则
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
  }

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
