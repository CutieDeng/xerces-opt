#include "pipeline.hh"
#include "field-write-collector.hh"
#include "write-operation-trace.hh"
#include "source-escape-collection.hh"
#include "escape-synthesizer.hh"
#include "ownership-transfer-analysis.hh"
#include "owned-conclusion.hh"
#include "capacity-association.hh"
#include "array-access-collector.hh"
#include "bound-condition-analyzer.hh"
#include "result-aggregator.hh"
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

  // 第四步：逃逸证据生成（两层架构）
  AD_DEBUG_PRINT ("Step 4: Generating escape evidence");
  vec<EscapeEvidenceResult*> * evidence_results = NULL;
  unsigned int total_synthesized = 0;
  AD_TRY (synthesizeAllFieldEscapes (AD_ARGS, detector, evidence_results, total_synthesized));
  AD_DEBUG_PRINT ("Escape evidence generation complete: %u results", total_synthesized);

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

  // 第七步：数组访问收集 [原 Step 8，提前执行]
  AD_DEBUG_PRINT ("Step 7: Collecting array access patterns");
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses = NULL;
  AD_TRY (collectAllArrayAccessesByTypeField (AD_ARGS, &array_accesses));
  AD_DEBUG_PRINT ("Array access collection complete");

  // 第八步：边界条件分析 [原 Step 9，提前执行]
  AD_DEBUG_PRINT ("Step 8: Analyzing bound conditions");
  if (array_accesses) {
    AD_TRY (analyzeAllBoundConditions (AD_ARGS, array_accesses));
  }
  AD_DEBUG_PRINT ("Bound condition analysis complete");

  // 第九步：指针-容量关联分析 [原 Step 7，现在可以使用 array_accesses]
  AD_DEBUG_PRINT ("Step 9: Analyzing pointer-capacity associations");
  vec<PointerCapacityAssociation*, va_gc>* capacity_results = NULL;
  AD_TRY (analyzeAllCapacityAssociations (AD_ARGS, detector, owned_conclusions, array_accesses, &capacity_results));
  AD_DEBUG_PRINT ("Pointer-capacity association analysis complete");

  // 第十步：结果聚合
  AD_DEBUG_PRINT ("Step 10: Aggregating all results");
  vec<UnifiedFieldAnalysisResult*, va_gc>* unified_results = NULL;
  AD_TRY (aggregateAllResults (AD_ARGS, detector, owned_conclusions,
                               capacity_results, array_accesses, &unified_results));
  AD_DEBUG_PRINT ("Result aggregation complete: %u unified results",
                  (unsigned int)vec_safe_length(unified_results));

  // Store results in context for LTO serialization
  ctx.unified_results = unified_results;

  // 第十一步：输出调试信息
  AD_DEBUG_PRINT ("Step 11: Printing debug results");
  printAllFieldOwnedConclusions (AD_ARGS, ctx.debug_file, owned_conclusions);
  printAllPointerCapacityAssociations (AD_ARGS, ctx.debug_file, capacity_results);
  printAllUnifiedResults (AD_ARGS, ctx.debug_file, unified_results);
  AD_TRY (printResults (AD_ARGS, detector));

  // 第十二步：写入统一格式的 Racket datum 结果文件
  AD_DEBUG_PRINT ("Step 12: Writing unified Racket datum results");
  AD_TRY (writeUnifiedResultsToRacketDatum (AD_ARGS, unified_results));

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
