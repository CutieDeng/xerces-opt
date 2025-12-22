#include "pipeline.hh"
#include "field-write-collector.hh"
#include "write-operation-trace.hh"
#include "array-detector.hh"
#include "info-print.hh"

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
  // 遍历所有函数，提取类型和字段信息，填充 detector.m_type_field_writes
  AD_DEBUG_PRINT("Step 1: Extracting field information");
  AD_TRY(collectTypesAndFields(detector, AD_ARGS));
  
  // 第二步：分析信息
  // 追踪字段赋值，分析字段的赋值来源，判断是否为 owned 数组
  AD_DEBUG_PRINT("Step 2: Analyzing field assignments");
  AD_TRY(traceFieldAssignments(detector, AD_ARGS));
  
  // 第三步：输出信息
  // 生成并输出分析报告到文件
  AD_DEBUG_PRINT("Step 3: Printing results");
  AD_TRY(printResults(AD_ARGS, detector));
  
  AD_DEBUG_PRINT("=== Array Detection Pipeline Completed ===");
  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// 顶层入口：创建检测器并执行分析
// ============================================================================

ArrayDetectErrorCode runArrayDetectorAnalysis(AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Starting array member detection analysis");
  ArrayDetector detector;
  // 延迟初始化：在使用前分配 vec 指针
  AD_TRY (init (detector, AD_ARGS));
  // 直接调用 pipeline 执行完整流程
  AD_TRY_LABEL (runArrayDetectionPipeline (AD_ARGS, detector), analysis_cleanup);
  analysis_cleanup:
  // 清理资源
  deinit (detector, AD_ARGS);
  AD_DEBUG_PRINT ("Array member detection analysis completed");
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns

