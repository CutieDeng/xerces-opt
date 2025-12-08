#include "cutie-context-gcc.hh"
#include "cutie-context-gcc-manager.h"

namespace cutie_ns {

// 公开接口 - 调用内部实现
::cutie_ns::CutieErrorCode init_cutie_context_gcc(CUTIE_FUNC_ARGS) {
  return init_cutie_context_gcc_internal(CUTIE_ARGS);
}

void deinit_cutie_context_gcc(CUTIE_FUNC_ARGS) {
  deinit_cutie_context_gcc_internal(CUTIE_ARGS);
}

bool is_cutie_context_gcc_initialized() {
  return is_cutie_context_gcc_initialized_internal();
}

} // namespace cutie_ns