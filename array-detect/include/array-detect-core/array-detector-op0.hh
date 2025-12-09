#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace array_detect_ns;

class ArrayDetector;
ArrayDetectErrorCode analyze_field_assignments_in_functions(ArrayDetector &self, AD_FUNC_ARGS);

} // namespace array_detector

namespace array_detect_ns {

using namespace array_detector;

ArrayDetectErrorCode collect_all_types_and_fields(AD_FUNC_ARGS, ArrayDetector* detector);

// 主分析入口：接收已初始化的 ArrayDetector 对象
// 语义：执行完整的数组成员检测分析流程
ArrayDetectErrorCode array_detect_analysis(AD_FUNC_ARGS);

// 字段赋值追踪：收集、分析和判断字段是否为 owned 数组
// 语义：三步分析流程 - 收集 -> 分析 -> 判断
ArrayDetectErrorCode trace_field_assignments(AD_FUNC_ARGS, ArrayDetector* detector);

// 直接执行分析（新入口）：在 plugin-top 中创建 ArrayDetector 后调用
// 语义：跳过 array_detect_analysis 的创建步骤，直接执行分析
ArrayDetectErrorCode array_detect_execute_with_detector(AD_FUNC_ARGS, ArrayDetector* detector);

// GCC 专用的执行入口函数
ArrayDetectErrorCode array_detect_execute_with_gcc_context(::array_detect_ns::ArrayDetectContext& ctx, ::array_detect_ns::ArrayDetectContextGcc& gcc_ctx, ArrayDetector* detector);

} // namespace array_detect_ns
