#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace array_detect_ns;

class ArrayDetector;
// 分析函数中的字段赋值模式
ArrayDetectErrorCode analyzeFieldAssignmentsInFunctions (ArrayDetector &self, AD_FUNC_ARGS);

// 收集所有类型和字段信息
ArrayDetectErrorCode collectTypesAndFields (ArrayDetector &detector, AD_FUNC_ARGS);

// 追踪字段赋值：收集、分析和判断字段是否为 owned 数组
// 三步分析流程：收集 -> 分析 -> 判断
ArrayDetectErrorCode traceFieldAssignments (ArrayDetector &detector, AD_FUNC_ARGS);

} // namespace array_detector

namespace array_detect_ns {

using array_detector::ArrayDetector;

} // namespace array_detect_ns
