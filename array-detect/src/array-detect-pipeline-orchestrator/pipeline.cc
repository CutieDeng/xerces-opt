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
  // Step 1: Extract field information
  AD_TRY (collectTypesAndFields (detector, AD_ARGS));

  // Step 2: Analyze field assignments
  AD_TRY (traceFieldAssignments (detector, AD_ARGS));

  // Step 3: Collect source operand escape information
  unsigned int total_analyzed = 0;
  unsigned int total_escaped = 0;
  AD_TRY (collectAllFieldEscapes (AD_ARGS, detector, total_analyzed, total_escaped));

  // Step 4: Generate escape evidence
  vec<EscapeEvidenceResult*> * evidence_results = NULL;
  unsigned int total_synthesized = 0;
  AD_TRY (synthesizeAllFieldEscapes (AD_ARGS, detector, evidence_results, total_synthesized));

  // Step 5: Analyze ownership transfers
  unsigned int transfer_analyzed = 0;
  unsigned int certain_transfers = 0;
  AD_TRY (analyzeAllOwnershipTransfers (AD_ARGS, detector, transfer_analyzed, certain_transfers));

  // Step 6: Analyze field owned conclusions
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions = NULL;
  AD_TRY (analyzeAllFieldOwnedConclusions (AD_ARGS, detector, &owned_conclusions));

  // Step 7: Collect array accesses
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses = NULL;
  AD_TRY (collectAllArrayAccessesByTypeField (AD_ARGS, &array_accesses));

  // Step 8: Analyze bound conditions
  if (array_accesses) {
    AD_TRY (analyzeAllBoundConditions (AD_ARGS, array_accesses));
  }

  // Step 9: Analyze pointer-capacity associations
  vec<PointerCapacityAssociation*, va_gc>* capacity_results = NULL;
  AD_TRY (analyzeAllCapacityAssociations (AD_ARGS, detector, owned_conclusions, array_accesses, &capacity_results));

  // Step 10: Aggregate results
  vec<UnifiedFieldAnalysisResult*, va_gc>* unified_results = NULL;
  AD_TRY (aggregateAllResults (AD_ARGS, detector, owned_conclusions,
                               capacity_results, array_accesses, &unified_results));
  AD_DEBUG_PRINT ("pipeline: %u unified results", (unsigned int)vec_safe_length(unified_results));

  // Store results in context for LTO serialization
  ctx.unified_results = unified_results;

  // Step 11: Print debug results
  printAllFieldOwnedConclusions (AD_ARGS, ctx.debug_file, owned_conclusions);
  printAllPointerCapacityAssociations (AD_ARGS, ctx.debug_file, capacity_results);
  printAllUnifiedResults (AD_ARGS, ctx.debug_file, unified_results);
  AD_TRY (printResults (AD_ARGS, detector));

  // Step 12: Write unified Racket datum results
  AD_TRY (writeUnifiedResultsToRacketDatum (AD_ARGS, unified_results));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 顶层入口：创建检测器并执行分析
// ============================================================================

ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ArrayDetector detector;
  AD_TRY (init (detector, AD_ARGS));
  AD_TRY_LABEL (runArrayDetectionPipeline (AD_ARGS, detector), analysis_cleanup);
  analysis_cleanup:
  deinit (detector, AD_ARGS);
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
