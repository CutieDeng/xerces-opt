#pragma once

#include "cutie-context-gcc.hh"

namespace cutie_ns {

// Internal functions - not exposed in main header
::cutie_ns::CutieErrorCode init_cutie_context_gcc_internal(CUTIE_FUNC_ARGS);
void deinit_cutie_context_gcc_internal(CUTIE_FUNC_ARGS);
bool is_cutie_context_gcc_initialized_internal();
CutieContextGcc& get_cutie_context_gcc_safe();

} // namespace cutie_ns