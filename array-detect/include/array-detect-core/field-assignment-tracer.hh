#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace ::array_detect_ns;

class ArrayDetector;

// 追踪字段赋值：分析字段赋值来源
// 语义：执行两步分析 - 赋值分析 -> 候选判断
// 前置条件：字段已通过 collectTypesAndFields 收集
ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS);

} // namespace array_detector
