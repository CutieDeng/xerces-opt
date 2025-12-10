#pragma once

#include "prelude.hh"
#include "array-detector.hh"
#include "state.hh"

// ----------------------------------------------------------------------------
// print_results 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace array_detect_ns {

ArrayDetectErrorCode print_results(AD_FUNC_ARGS, ArrayDetector &detector);

} // namespace array_detect_ns
