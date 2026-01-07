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
  FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
  if (df) {
    fprintf (df, "[ipa_generate_summary] called, g_wpa_results=%p\n", (void*)g_wpa_results);
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
  FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
  if (df) {
    fprintf (df, "[ipa_write_summary] called\n");
  }
  ::array_detect_ns::writeArrayDetectLtoSummarySection ();
}

// Called in LTRANS to read summaries from all input files
static void ipa_read_summary (void) {
  FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
  if (df) {
    fprintf (df, "[ipa_read_summary] called\n");
  }

  // LTO summary reading is handled via readArrayDetectLtoSummarySections.
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
                       0,                        // function_transform_todo_flags_start
                       NULL,                     // function_transform
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
    FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
    if (df) {
      fprintf (df, "[execute] called, in_lto_p=%d, flag_ltrans=%d\n", in_lto_p, flag_ltrans);
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
// PLUGIN_FINISH callback for LTO aggregation output (WPA)
// ============================================================================

static void plugin_finish_callback (void* /*gcc_data*/, void* /*user_data*/) {
  FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
  char const* aggregated_file = getenv ("AD_AGGREGATED_FILE");

  // Debug output
  if (df) {
    fprintf (df, "[plugin_finish_callback] in_lto_p=%d, flag_ltrans=%d\n",
             in_lto_p, flag_ltrans);
  }

  // In WPA phase, read LTO summaries and write aggregated results
  if (in_lto_p && !flag_ltrans && aggregated_file) {
    ::array_detect_ns::clearLtransLtoSummaries ();
    ::array_detect_ns::readArrayDetectLtoSummarySections ();
    ::array_detect_ns::writeLtransAggregatedResults (aggregated_file);
    if (df) {
      fprintf (df, "[plugin_finish_callback] wrote aggregated results to: %s\n", aggregated_file);
    }
  }

  ::array_detect_ns::closeGlobalDebugFile ();
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

  ::array_detect_ns::initGlobalDebugFileFromEnv ();

  return 0;
}

int plugin_is_GPL_compatible = 1;
