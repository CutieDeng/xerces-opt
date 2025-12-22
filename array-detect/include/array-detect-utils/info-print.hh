#pragma once

#include "prelude.hh"
#include "state.hh"
#include "context.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printResults(AD_FUNC_ARGS, ArrayDetector &detector);

} // namespace array_detect_ns
