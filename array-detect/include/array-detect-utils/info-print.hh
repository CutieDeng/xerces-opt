#pragma once

#include "prelude.hh"
#include "array-detector.hh"
#include "state.hh"

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace array_detect_ns {

ArrayDetectErrorCode printResults(AD_FUNC_ARGS, ArrayDetector &detector);

} // namespace array_detect_ns
