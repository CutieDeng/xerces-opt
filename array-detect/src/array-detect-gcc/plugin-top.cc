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
  ::array_detect_ns::writeArrayDetectLtoSummarySection ();
}

// Called in LTRANS to read summaries from all input files
static void ipa_read_summary (void) {
  ::array_detect_ns::readArrayDetectLtoSummarySections ();
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
                       ipa_generate_summary,  // generate_summary
                       ipa_write_summary,     // write_summary
                       ipa_read_summary,      // read_summary
                       NULL,  // write_optimization_summary
                       NULL,  // read_optimization_summary
                       NULL,  // stmt_fixup
                       0,     // function_transform_todo_flags_start
                       NULL,  // function_transform
                       NULL)  // variable_transform
  {}

  opt_pass * clone () override { return new pass_array_detect (g); }

  // Gate function: only run during regular compilation, skip during LTO link
  bool gate (function*) override {
    // During LTO WPA/LTRANS phases, we only need summary hooks, not execute()
    return !in_lto_p;
  }

  unsigned int execute (function*) override {
    // Skip analysis during LTO WPA/LTRANS phases - we only need the summary hooks
    // Analysis was already done during initial compilation
    if (in_lto_p) {
      // In LTRANS phase, output aggregated results once
      static bool ltrans_output_done = false;
      if (flag_ltrans && !ltrans_output_done && ::array_detect_ns::hasLtransLtoSummaries ()) {
        ltrans_output_done = true;
        char const* result_file = getenv ("AD_RESULT_FILE");
        if (result_file) {
          ::array_detect_ns::writeLtransResultsToRacketDatum (result_file);
        }
      }
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
  pass_info.reference_pass_name = "cdtor";        // 在此 pass 之后插入
  pass_info.ref_pass_instance_number = 1;         // 符合规则
  pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback (plugin_info->base_name,
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,
                    &pass_info);

  return 0;
}

int plugin_is_GPL_compatible = 1;
