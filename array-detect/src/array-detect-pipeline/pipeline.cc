#include "pipeline.hh"
#include "field-write-collector.hh"
#include "write-operation-trace.hh"
#include "source-escape-collection.hh"
#include "escape-synthesizer.hh"
#include "ownership-transfer-analysis.hh"
#include "owned-conclusion.hh"
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

  // 第三步：收集源操作数逃逸信息
  AD_DEBUG_PRINT ("Step 3: Collecting source operand escape information");
  unsigned int total_analyzed = 0;
  unsigned int total_escaped = 0;
  AD_TRY (collectAllFieldEscapes (AD_ARGS, detector, total_analyzed, total_escaped));
  AD_DEBUG_PRINT ("Escape collection complete: %u writes collected, %u with escapes",
                  total_analyzed, total_escaped);

  // 第四步：逃逸综合分析
  AD_DEBUG_PRINT ("Step 4: Synthesizing escape information");
  vec<EscapeSynthesisResult*> * synthesis_results = NULL;
  unsigned int total_synthesized = 0;
  AD_TRY (synthesizeAllFieldEscapes (AD_ARGS, detector, synthesis_results, total_synthesized));
  AD_DEBUG_PRINT ("Escape synthesis complete: %u results synthesized", total_synthesized);

  // 第五步：所有权转移分析
  AD_DEBUG_PRINT ("Step 5: Analyzing ownership transfers");
  unsigned int transfer_analyzed = 0;
  unsigned int certain_transfers = 0;
  AD_TRY (analyzeAllOwnershipTransfers (AD_ARGS, detector, transfer_analyzed, certain_transfers));
  AD_DEBUG_PRINT ("Ownership transfer analysis complete: %u analyzed, %u certain transfers",
                  transfer_analyzed, certain_transfers);

  // 第六步：字段 owned 结论分析
  AD_DEBUG_PRINT ("Step 6: Analyzing field owned conclusions");
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions = NULL;
  AD_TRY (analyzeAllFieldOwnedConclusions (AD_ARGS, detector, &owned_conclusions));
  AD_DEBUG_PRINT ("Field owned conclusion analysis complete");

  // 第七步：输出最终结果
  AD_DEBUG_PRINT ("Step 7: Printing final results");
  printAllFieldOwnedConclusions (AD_ARGS, stderr, owned_conclusions);
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
