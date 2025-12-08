#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detector {

using namespace cutie_ns;

class ArrayDetector;
CutieErrorCode analyze_field_assignments_in_functions(ArrayDetector &self, CUTIE_FUNC_ARGS);

} // namespace array_detector

namespace cutie_ns {

using namespace array_detector;

CutieErrorCode collect_all_types_and_fields(CUTIE_FUNC_ARGS, ArrayDetector* detector);

// 主分析入口：接收已初始化的 ArrayDetector 对象
// 语义：执行完整的数组成员检测分析流程
CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS);

// 字段赋值追踪：收集、分析和判断字段是否为 owned 数组
// 语义：三步分析流程 - 收集 -> 分析 -> 判断
CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector);

// 直接执行分析（新入口）：在 plugin-top 中创建 ArrayDetector 后调用
// 语义：跳过 array_detect_analysis 的创建步骤，直接执行分析
CutieErrorCode array_detect_execute_with_detector(CUTIE_FUNC_ARGS, ArrayDetector* detector);

// GCC 专用的执行入口函数
CutieErrorCode array_detect_execute_with_gcc_context(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx, ArrayDetector* detector);

} // namespace cutie_ns
