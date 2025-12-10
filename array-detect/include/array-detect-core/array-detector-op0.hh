#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace array_detect_ns;

class ArrayDetector;
// 分析函数中的字段赋值模式
ArrayDetectErrorCode analyzeFieldAssignmentsInFunctions(ArrayDetector &self, AD_FUNC_ARGS);

} // namespace array_detector

namespace array_detect_ns {

using namespace array_detector;

// 收集所有类型和字段信息
ArrayDetectErrorCode collectTypesAndFields(AD_FUNC_ARGS, ArrayDetector &detector);

// 主分析入口：执行完整的数组成员检测分析流程
ArrayDetectErrorCode analyzeArrayDetection(AD_FUNC_ARGS);

// 追踪字段赋值：收集、分析和判断字段是否为 owned 数组
// 三步分析流程：收集 -> 分析 -> 判断
ArrayDetectErrorCode traceFieldAssignments(AD_FUNC_ARGS, ArrayDetector &detector);

// 使用已创建的检测器执行分析（跳过内部创建步骤）
ArrayDetectErrorCode analyzeWithDetector(AD_FUNC_ARGS, ArrayDetector &detector);

} // namespace array_detect_ns
