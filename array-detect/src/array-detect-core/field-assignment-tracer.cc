#include "prelude.hh"
#include "state.hh"
#include "field-assignment-tracer.hh"
#include "array-detector.hh"
#include "field-analysis-main.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// 追踪字段赋值：分析字段赋值来源
// 语义：执行两步分析 - 赋值分析 -> 候选判断
// 前置条件：字段已通过 collectTypesAndFields 收集
ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Tracing field assignments");
  
  // 使用新的分析流程
  vec<tree> field_decls;
  vec<FieldAnalysisResult*> field_results;
  
  // 执行完整的字段分析
  AD_TRY (performFieldAnalysis (AD_ARGS, detector, field_decls, field_results));
  
  // 更新 FieldInfo 的 is_array_candidate 字段
  AD_TRY (updateFieldInfoFromResults (AD_ARGS, field_decls, field_results, detector));
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
