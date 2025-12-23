#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace ::array_detect_ns;

class ArrayDetector;

// 调试功能：分析函数中的字段赋值模式
// 用于详细的调试输出，分析每个函数中的字段赋值情况
ArrayDetectErrorCode analyzeFieldAssignmentsInFunctions (ArrayDetector &detector, AD_FUNC_ARGS);

} // namespace array_detector
