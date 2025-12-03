#include "prelude.hh"
#include "state.hh"
#include "array-detector-op0.hh"

namespace cutie_ns {
CutieErrorCode array_detect_execute (CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  CUTIE_TRY (initWithStderr(CUTIE_ARGS));
  
  CUTIE_TRY (array_detect_analysis (CUTIE_ARGS));
  
  ecode = ::cutie_ns::OK;
  cleanup:
  cutie_ns::deinit(CUTIE_ARGS);
} CUTIE_FUNCTION_END2

CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  // 重命名标签以避免与宏中的cleanup冲突
  CUTIE_DEBUG_PRINT("Starting array member detection analysis");
  
  // 创建数组检测器
  ArrayDetector detector;
  
  // 执行分析
  CUTIE_TRY_LABEL(trace_field_assignments(CUTIE_ARGS, &detector), analysis_cleanup);
  
  CUTIE_TRY_LABEL(cutie_ns::print_results(CUTIE_ARGS, &detector), analysis_cleanup);
  
  analysis_cleanup:
  // 使用显式清理函数替代析构函数
  detector.cleanup();
  
  CUTIE_DEBUG_PRINT("Array member detection analysis completed");
  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  // 追踪字段的赋值操作
  CUTIE_DEBUG_PRINT("Tracing field assignments");
  
  // 第一步：收集所有类型和字段
  CUTIE_TRY (collect_all_types_and_fields (CUTIE_ARGS, detector));
  
  // 第二步：分析字段赋值
  CUTIE_TRY (analyze_field_assignments_in_functions (CUTIE_ARGS, detector));

  // 第三步：分析使用情况，判断是否是数组候选
  CUTIE_TRY (detector->analyze_usage (CUTIE_ARGS));

  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

} // namespace cutie_ns
