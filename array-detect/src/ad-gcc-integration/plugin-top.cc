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

// NOTE: LTO summary conversion from UnifiedFieldAnalysisResult has been removed.
// The result-aggregator module was deleted. LTO summary functionality needs
// to be re-implemented based on the new ad-array-read-capacity / ad-array-write-capacity modules.

// Store results from execute() for later serialization
static bool g_wpa_analysis_done = false;
static ::array_detect_ns::ArrayDetectContextGcc g_plugin_gcc_ctx;

// Custom section name (must match lto-summary.cc)
static char const* const kArrayDetectSectionName = "array_detect";

static bool isWpaPhase () {
  return flag_wpa != nullptr;
}

static ::array_detect_ns::ArrayDetectErrorCode runAnalysisAndStoreResults () {
  // 使用全局 context，确保 LTO 各阶段 context 生命周期一致
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  ::array_detect_ns::ArrayDetectErrorCode result =
    ::array_detect_ns::runArrayDetect (ctx, gcc_ctx);

  // NOTE: Result storage for LTO summary has been removed.
  // The UnifiedFieldAnalysisResult type was deleted along with result-aggregator module.
  g_wpa_analysis_done = true;
  return result;
}

// Called after execute() to generate summary data
::array_detect_ns::ArrayDetectErrorCode ipa_generate_summary_impl (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;
  AD_DEBUG_PRINT ("[ipa_generate_summary] called");

  if (isWpaPhase () && !g_wpa_analysis_done) {
    if (!getenv ("AD_ALLOW_WPA_ANALYSIS")) {
      AD_DEBUG_PRINT ("[ipa_generate_summary] WPA: skip analysis (set AD_ALLOW_WPA_ANALYSIS=1 to enable)");
      g_wpa_analysis_done = true;
    } else {
      AD_DEBUG_PRINT ("[ipa_generate_summary] WPA: running analysis for summary");
      runAnalysisAndStoreResults ();
    }
  }

  if (!in_lto_p && flag_generate_lto && !g_wpa_analysis_done) {
    AD_DEBUG_PRINT ("[ipa_generate_summary] LGEN: running analysis for summary");
    runAnalysisAndStoreResults ();
  }

  // NOTE: LTO summary blob emission is disabled by default during LGEN
  // to avoid varpool manipulation issues. Set AD_ENABLE_LTO_SECTIONS=1 to enable.
  // The analysis results are still computed and output to file if AD_RESULT_FILE is set.
  AD_RETURNE (OK);
} AD_FUNCTION_END

void ipa_generate_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  (void) ipa_generate_summary_impl (AD_ARGS);
}

// Called to write summary blob into LTO sections
// Uses lto_begin_section/lto_write_data/lto_end_section with custom section name
static void ipa_write_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;

  AD_DEBUG_PRINT ("[ipa_write_summary] called, in_lto_p=%d, flag_ltrans=%d, flag_wpa=%s, flag_generate_lto=%d",
                  in_lto_p, flag_ltrans, flag_wpa ? flag_wpa : "<null>", flag_generate_lto);

  // WPA phase: re-emit aggregated summaries for LTRANS
  if (isWpaPhase ()) {
    vec<::array_detect_ns::LtoUnifiedResultSummary*, va_gc>* summaries =
      ::array_detect_ns::getWpaLtoSummaries ();
    unsigned int count = vec_safe_length (summaries);
    AD_DEBUG_PRINT ("[ipa_write_summary] WPA: %u summaries to emit", count);
    if (count > 0) {
      AD_DEBUG_PRINT ("[ipa_write_summary] WPA: writing aggregated summaries for LTRANS");
      (void) ::array_detect_ns::writeArrayDetectLtoSummarySection (AD_ARGS);
    }
    return;
  }

  // LGEN phase: write per-TU summaries
  if (!in_lto_p && flag_generate_lto) {
    AD_DEBUG_PRINT ("[ipa_write_summary] LGEN: writing per-TU summary section");
    (void) ::array_detect_ns::writeArrayDetectLtoSummarySection (AD_ARGS);
    return;
  }

  AD_DEBUG_PRINT ("[ipa_write_summary] skip (not LGEN or WPA)");
}

// Helper: try to read our custom section from file_data
// lto_begin_section creates sections that can be read via lto_get_raw_section_data
// Returns data via out parameters, error code indicates success/failure
static ::array_detect_ns::ArrayDetectErrorCode find_array_detect_section (
  AD_FUNC_ARGS,
  struct lto_file_decl_data* file_data,
  char const** data_out,
  size_t* len_out
) AD_FUNCTION_BEGIN {
  (void) gcc_ctx;

  if (!file_data) {
    AD_DEBUG_PRINT ("[find_array_detect_section] ERROR: null file_data");
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *data_out = nullptr;
  *len_out = 0;

  // Try to get section data using our custom section name
  // lto_get_raw_section_data looks up sections in the section_hash_table
  char const* data = lto_get_raw_section_data (
    file_data,
    LTO_section_decls,  // Section type (used for name construction)
    kArrayDetectSectionName,
    0,  // order
    len_out
  );

  if (data && *len_out > 0) {
    AD_DEBUG_PRINT ("[find_array_detect_section] found section, len=%zu", *len_out);
    *data_out = data;
    AD_RETURNE (OK);
  }

  // Section not found - this is expected if no summaries were written
  AD_DEBUG_PRINT ("[find_array_detect_section] section '%s' not found in file",
                  kArrayDetectSectionName);
  AD_RETURNE (RECOVERABLE_ERROR);
} AD_FUNCTION_END

// Called in WPA/LTRANS to read summaries from all input files
static void ipa_read_summary (void) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;

  AD_DEBUG_PRINT ("[ipa_read_summary] called, in_lto_p=%d, flag_ltrans=%d, flag_wpa=%s",
                  in_lto_p, flag_ltrans, flag_wpa ? flag_wpa : "<null>");

  // Get all input files
  struct lto_file_decl_data** file_data_vec = lto_get_file_decl_data ();
  if (!file_data_vec) {
    AD_DEBUG_PRINT ("[ipa_read_summary] ERROR: no file_data_vec");
    return;
  }

  // WPA phase: read all LGEN summaries from input .o files
  if (isWpaPhase ()) {
    AD_DEBUG_PRINT ("[ipa_read_summary] WPA: reading LGEN summaries");

    // Iterate over all input files
    for (unsigned i = 0; file_data_vec[i]; i++) {
      struct lto_file_decl_data* file_data = file_data_vec[i];
      char const* data = nullptr;
      size_t len = 0;

      ::array_detect_ns::ArrayDetectErrorCode err =
        find_array_detect_section (AD_ARGS, file_data, &data, &len);
      if (err == ::array_detect_ns::OK && data && len > 0) {
        AD_DEBUG_PRINT ("[ipa_read_summary] WPA: reading from file %u, len=%zu", i, len);
        (void) ::array_detect_ns::readArrayDetectLtoSummarySections (AD_ARGS, data, len);
      }
    }

    // Report what we found
    vec<::array_detect_ns::LtoUnifiedResultSummary*, va_gc>* summaries =
      ::array_detect_ns::getLtransLtoSummaries ();
    unsigned int count = vec_safe_length (summaries);
    AD_DEBUG_PRINT ("[ipa_read_summary] WPA: loaded %u summaries from LGEN", count);

    // WPA aggregation: store for potential re-emission to LTRANS
    if (count > 0) {
      ::array_detect_ns::setWpaLtoSummaries (summaries);
      AD_DEBUG_PRINT ("[ipa_read_summary] WPA: stored summaries for re-emission");
    }
    return;
  }

  // LTRANS phase: read WPA-aggregated summaries
  if (flag_ltrans) {
    AD_DEBUG_PRINT ("[ipa_read_summary] LTRANS: reading WPA summaries");

    // Iterate over all input files
    for (unsigned i = 0; file_data_vec[i]; i++) {
      struct lto_file_decl_data* file_data = file_data_vec[i];
      char const* data = nullptr;
      size_t len = 0;

      ::array_detect_ns::ArrayDetectErrorCode err =
        find_array_detect_section (AD_ARGS, file_data, &data, &len);
      if (err == ::array_detect_ns::OK && data && len > 0) {
        AD_DEBUG_PRINT ("[ipa_read_summary] LTRANS: reading from file %u, len=%zu", i, len);
        (void) ::array_detect_ns::readArrayDetectLtoSummarySections (AD_ARGS, data, len);
      }
    }

    vec<::array_detect_ns::LtoUnifiedResultSummary*, va_gc>* summaries =
      ::array_detect_ns::getLtransLtoSummaries ();
    unsigned int count = vec_safe_length (summaries);
    AD_DEBUG_PRINT ("[ipa_read_summary] LTRANS: loaded %u summaries", count);
    return;
  }

  AD_DEBUG_PRINT ("[ipa_read_summary] skip (not in WPA or LTRANS)");
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
    AD_DEBUG_PRINT ("[execute] called, in_lto_p=%d, flag_ltrans=%d, flag_wpa=%s, flag_generate_lto=%d",
                    in_lto_p, flag_ltrans, flag_wpa ? flag_wpa : "<null>", flag_generate_lto);

    if (flag_ltrans) {
      AD_DEBUG_PRINT ("[execute] skip (LTRANS)");
      return 0;
    }

    if (isWpaPhase ()) {
      AD_DEBUG_PRINT ("[execute] WPA: skip (no file aggregation)");
      return 0;
    }

    if (in_lto_p) {
      AD_DEBUG_PRINT ("[execute] skip (in_lto_p)");
      return 0;
    }

    // LGEN with -flto: analysis handled in ipa_generate_summary.
    if (flag_generate_lto) {
      AD_DEBUG_PRINT ("[execute] skip (LGEN, flag_generate_lto)");
      return 0;
    }

    // Regular compilation: run analysis
    AD_DEBUG_PRINT ("[execute] running regular analysis");
    ::array_detect_ns::ArrayDetectErrorCode result = runAnalysisAndStoreResults ();
    AD_DEBUG_PRINT ("[execute] done");
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

  bool gate (function* fn) override {
    ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
    char const* fn_name = nullptr;
    if (fn && fn->decl && DECL_NAME (fn->decl)) {
      fn_name = IDENTIFIER_POINTER (DECL_NAME (fn->decl));
    }
    AD_DEBUG_PRINT ("[ltrans-pass] gate fn=%s flag_ltrans=%d",
                    fn_name ? fn_name : "<null>", flag_ltrans);
    return flag_ltrans;
  }

  static unsigned int execute_impl (AD_FUNC_ARGS, function* fn) {
    char const* fn_name = nullptr;
    if (fn && fn->decl && DECL_NAME (fn->decl)) {
      fn_name = IDENTIFIER_POINTER (DECL_NAME (fn->decl));
    }
    AD_DEBUG_PRINT ("[ltrans-pass] execute_impl fn=%s flag_ltrans=%d",
                    fn_name ? fn_name : "<null>", flag_ltrans);
    if (!flag_ltrans || !fn) return 0;
    if (!gimple_has_body_p (fn->decl)) return 0;
    return ::array_detect_ns::runLtoTransform (AD_ARGS, fn);
  }

  unsigned int execute (function* fn) override {
    ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
    ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
    AD_DEBUG_PRINT ("[ltrans-pass] execute ENTRY");
    unsigned int result = execute_impl (AD_ARGS, fn);
    AD_DEBUG_PRINT ("[ltrans-pass] execute EXIT result=%u", result);
    return result;
  }
};


// ============================================================================
// PLUGIN_FINISH callback for LTO aggregation output (WPA)
// ============================================================================

// 使用 AD_FUNCTION_BEGIN2 因为 finish 阶段 context 可能已部分清理
::array_detect_ns::ArrayDetectErrorCode plugin_finish_callback_impl (AD_FUNC_ARGS) AD_FUNCTION_BEGIN2
  AD_DEBUG_PRINT ("[plugin_finish_callback] ENTRY in_lto_p=%d, flag_ltrans=%d, flag_wpa=%s",
                  in_lto_p, flag_ltrans, flag_wpa ? flag_wpa : "<null>");

  if (in_lto_p && !flag_ltrans) {
    AD_DEBUG_PRINT ("[plugin_finish_callback] WPA phase finish, closing debug file");
    ::array_detect_ns::closeGlobalDebugFile ();
    ecode = ::array_detect_ns::OK;
    goto plugin_finish_cleanup;
  }

  AD_DEBUG_PRINT ("[plugin_finish_callback] non-WPA finish, closing debug file");
  ::array_detect_ns::closeGlobalDebugFile ();
  ecode = ::array_detect_ns::OK;
plugin_finish_cleanup:
AD_FUNCTION_END3

// 使用 AD_FUNCTION_BEGIN2 因为 init 阶段栈帧追踪可能未初始化
::array_detect_ns::ArrayDetectErrorCode plugin_init_debug_impl (AD_FUNC_ARGS) AD_FUNCTION_BEGIN2
  AD_DEBUG_PRINT ("[plugin_init] ENTRY in_lto_p=%d, flag_ltrans=%d, flag_generate_lto=%d, flag_wpa=%s",
                  in_lto_p, flag_ltrans, flag_generate_lto, flag_wpa ? flag_wpa : "<null>");

  // 打印 LTO 阶段详细信息
  if (flag_ltrans) {
    AD_DEBUG_PRINT ("[plugin_init] Phase: LTRANS (code generation)");
  } else if (flag_wpa) {
    AD_DEBUG_PRINT ("[plugin_init] Phase: WPA (whole program analysis)");
  } else if (flag_generate_lto) {
    AD_DEBUG_PRINT ("[plugin_init] Phase: LGEN (LTO generation)");
  } else {
    AD_DEBUG_PRINT ("[plugin_init] Phase: Regular compilation (no LTO)");
  }

  ecode = ::array_detect_ns::OK;
AD_FUNCTION_END3

void plugin_finish_callback (void* /*gcc_data*/, void* /*user_data*/) {
  ::array_detect_ns::ArrayDetectContext& ctx = ::array_detect_ns::g_array_detect_ctx;
  ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx = g_plugin_gcc_ctx;
  (void) plugin_finish_callback_impl (AD_ARGS);
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

    // 初始化全局 context buffers，确保所有 LTO 阶段可用
    AD_DEBUG_PRINT ("[plugin_init] Initializing context buffers...");
    ::array_detect_ns::initContextBuffers (ctx, gcc_ctx, 512);
    AD_DEBUG_PRINT ("[plugin_init] Context buffers initialized");

    (void) plugin_init_debug_impl (AD_ARGS);

    struct register_pass_info pass_info;
    if (flag_ltrans) {
      AD_DEBUG_PRINT ("[plugin_init] Registering LTRANS pass (pass_array_detect_ltrans)");
      pass_info.pass = new pass_array_detect_ltrans (g);
      pass_info.reference_pass_name = "optimized";
      pass_info.ref_pass_instance_number = 1;
      pass_info.pos_op = PASS_POS_INSERT_AFTER;
    } else {
      AD_DEBUG_PRINT ("[plugin_init] Registering IPA pass (pass_array_detect)");
      pass_info.pass = new pass_array_detect (g);
      pass_info.reference_pass_name = "inline";       // 在内联优化之后插入，以便分析内联后的代码
      pass_info.ref_pass_instance_number = 1;         // 符合规则
      pass_info.pos_op = PASS_POS_INSERT_AFTER;
    }

    register_callback (plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,
                      &pass_info);
    AD_DEBUG_PRINT ("[plugin_init] Pass registered successfully");

    // Register finish callback for LTO aggregation output
    register_callback (plugin_info->base_name,
                      PLUGIN_FINISH,
                      plugin_finish_callback,
                      NULL);
    AD_DEBUG_PRINT ("[plugin_init] Finish callback registered, plugin_init complete");
  }

  return 0;
}

int plugin_is_GPL_compatible = 1;
