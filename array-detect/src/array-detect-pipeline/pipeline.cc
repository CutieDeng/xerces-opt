#include "pipeline.hh"
#include "field-write-collector.hh"
#include "write-operation-trace.hh"
#include "source-use-analysis.hh"
#include "array-detector.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// Pipeline 实现：调用下层模块的抽象层
// ============================================================================

ArrayDetectErrorCode runArrayDetectionPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("=== Starting Array Detection Pipeline ===");

  // 第一步：提取字段信息
  AD_DEBUG_PRINT ("Step 1: Extracting field information");
  AD_TRY (collectTypesAndFields (detector, AD_ARGS));

  // 第二步：分析字段赋值来源
  AD_DEBUG_PRINT ("Step 2: Analyzing field assignments");
  AD_TRY (traceFieldAssignments (detector, AD_ARGS));

  // 第三步：分析源操作数使用和逃逸
  AD_DEBUG_PRINT ("Step 3: Analyzing source operand uses and escapes");
  unsigned int total_analyzed = 0;
  unsigned int total_escaped = 0;
  AD_TRY (analyzeAllFieldSourceUses (AD_ARGS, detector, total_analyzed, total_escaped));
  AD_DEBUG_PRINT ("Source use analysis complete: %u writes analyzed, %u with escapes",
                  total_analyzed, total_escaped);

  // 第四步：输出结果
  AD_DEBUG_PRINT ("Step 4: Printing results");
  AD_TRY (printResults (AD_ARGS, detector));

  AD_DEBUG_PRINT ("=== Array Detection Pipeline Completed ===");
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 顶层入口：创建检测器并执行分析
// ============================================================================

ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Starting array member detection analysis");
  ArrayDetector detector;
  AD_TRY (init (detector, AD_ARGS));
  AD_TRY_LABEL (runArrayDetectionPipeline (AD_ARGS, detector), analysis_cleanup);
  analysis_cleanup:
  deinit (detector, AD_ARGS);
  AD_DEBUG_PRINT ("Array member detection analysis completed");
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
