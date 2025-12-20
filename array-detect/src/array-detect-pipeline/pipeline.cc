#include "pipeline.hh"
#include "array-detector-driver.hh"
#include "info-print.hh"
#include "field-analysis-main.hh"

namespace array_detect_ns {

// ============================================================================
// Pipeline 实现：简单暴力地调用下层模块
// ============================================================================

ArrayDetectErrorCode runArrayDetectionPipeline(
  AD_FUNC_ARGS,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT("=== Starting Array Detection Pipeline ===");
  
  // 第一步：提取字段信息
  // 遍历所有函数，提取类型和字段信息，填充 detector.m_fields
  AD_DEBUG_PRINT("Step 1: Extracting field information");
  AD_TRY(array_detector::collectTypesAndFields(detector, AD_ARGS));
  
  // 第二步：分析信息
  // 追踪字段赋值，分析字段的赋值来源，判断是否为 owned 数组
  AD_DEBUG_PRINT("Step 2: Analyzing field assignments");
  AD_TRY(array_detector::traceFieldAssignments(detector, AD_ARGS));
  
  // 第三步：输出信息
  // 生成并输出分析报告到文件
  AD_DEBUG_PRINT("Step 3: Printing results");
  AD_TRY(printResults(AD_ARGS, detector));
  
  AD_DEBUG_PRINT("=== Array Detection Pipeline Completed ===");
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns

