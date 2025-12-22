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

#include "gcc-ext-util.hh"
#include "info.hh"
#include "info-print.hh"

// ----------------------------------------------------------------------------
// Pass 注册结构
// ----------------------------------------------------------------------------

namespace array_detect_ns {

// Run entry: sets up runtime/context then hands off to the analyzer.
ArrayDetectErrorCode runArrayDetect (AD_FUNC_ARGS) AD_FUNCTION_BEGIN2 {
  initGccContext (AD_ARGS);
  AD_ETRY2 (initContextWithStderr (AD_ARGS), cleanup, false, true, "Failed to init context: %s");
  AD_TRY (runArrayDetectorAnalysis (AD_ARGS));
  AD_RETURNE (OK);
  cleanup:
  deinitContext (AD_ARGS);
} AD_FUNCTION_END3

}

namespace {

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
  pass_array_detect(gcc::context* ctxt)
      : ipa_opt_pass_d(array_detect_pass_data, ctxt,
                       NULL,  // generate_summary
                       NULL,  // write_summary
                       NULL,  // read_summary
                       NULL,  // write_optimization_summary
                       NULL,  // read_optimization_summary
                       NULL,  // stmt_fixup
                       0,     // function_transform_todo_flags_start
                       NULL,  // function_transform
                       NULL)  // variable_transform
  {}

  opt_pass* clone() override { return new pass_array_detect(g); }

  unsigned int execute(function*) override {
    // Regular IPA passes in WPA mode call execute() with NULL function
    // 使用局部上下文，避免全局状态问题
    ::array_detect_ns::ArrayDetectContext local_ctx;
    ::array_detect_ns::ArrayDetectContextGcc local_gcc_ctx;
    ::array_detect_ns::ArrayDetectErrorCode result = runArrayDetect (local_ctx, local_gcc_ctx);
    return result != ::array_detect_ns::OK;
  }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// 插件初始化
// ----------------------------------------------------------------------------

int plugin_init(struct plugin_name_args* plugin_info,
                struct plugin_gcc_version* version) {
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
