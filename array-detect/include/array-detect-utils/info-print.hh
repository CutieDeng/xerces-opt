#pragma once

#include "array-detect-utils/prelude.hh"
#include "array-detect-core/array-detector.hh"
#include "array-detect-context/state.hh"

// ----------------------------------------------------------------------------
// print_results 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace cutie_ns {

CutieErrorCode print_results(CUTIE_FUNC_ARGS, ArrayDetector* detector);

} // namespace cutie_ns
