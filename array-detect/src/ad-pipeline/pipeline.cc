// ============================================================================
// ad-pipeline 模块实现
// ============================================================================
// 顶层控制流编排模块
// 实现分析阶段调度和 pipeline 执行
// ============================================================================

// 使用相对路径确保包含正确的头文件
#include "../../include/ad-pipeline/pipeline.hh"
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
// 全局 Pipeline 状态
// ============================================================================

static PipelineState g_pipeline_state = {
  PHASE_COLLECT_WRITES,
  0, 0, 0, 0, 0
};

PipelineState* getPipelineState (AD_FUNC_ARGS) {
  (void)ctx; (void)gcc_ctx;
  return &g_pipeline_state;
}

// ============================================================================
// Pipeline 实现：调用下层模块的抽象层
// ============================================================================

ArrayDetectErrorCode runPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  // Step 1: Extract field information
  g_pipeline_state.current_phase = PHASE_COLLECT_WRITES;
  AD_TRY (collectTypesAndFields (detector, AD_ARGS));

  // Step 2: Analyze field assignments
  g_pipeline_state.current_phase = PHASE_TRACE_SOURCES;
  AD_TRY (traceFieldAssignments (detector, AD_ARGS));

  // Step 3: Collect source operand escape information
  g_pipeline_state.current_phase = PHASE_ANALYZE_USES;
  unsigned int total_analyzed = 0;
  unsigned int total_escaped = 0;
  AD_TRY (collectAllFieldEscapes (AD_ARGS, detector, total_analyzed, total_escaped));
  g_pipeline_state.total_escapes_analyzed = total_analyzed;

  // Step 4: Generate escape evidence
  g_pipeline_state.current_phase = PHASE_SYNTHESIZE_ESCAPES;
  vec<EscapeEvidenceResult*> * evidence_results = NULL;
  unsigned int total_synthesized = 0;
  AD_TRY (synthesizeAllFieldEscapes (AD_ARGS, detector, evidence_results, total_synthesized));

  // Step 5: Analyze ownership transfers
  g_pipeline_state.current_phase = PHASE_ANALYZE_OWNERSHIP;
  unsigned int transfer_analyzed = 0;
  unsigned int certain_transfers = 0;
  AD_TRY (analyzeAllOwnershipTransfers (AD_ARGS, detector, transfer_analyzed, certain_transfers));
  g_pipeline_state.total_ownership_analyzed = transfer_analyzed;

  // Step 6: Analyze field owned conclusions
  g_pipeline_state.current_phase = PHASE_GENERATE_VERDICT;
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions = NULL;
  AD_TRY (analyzeAllFieldOwnedConclusions (AD_ARGS, detector, owned_conclusions));
  g_pipeline_state.total_verdicts_generated = vec_safe_length(owned_conclusions);

  // Step 7: Collect array accesses
  g_pipeline_state.current_phase = PHASE_COLLECT_ACCESSES;
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses = NULL;
  AD_TRY (collectAllArrayAccessesByTypeField (AD_ARGS, array_accesses));

  // Step 8: Analyze bound conditions
  g_pipeline_state.current_phase = PHASE_ANALYZE_BOUNDS;
  if (array_accesses) {
    AD_TRY (analyzeAllBoundConditions (AD_ARGS, array_accesses));
  }

  // Step 9: Analyze pointer-capacity associations
  g_pipeline_state.current_phase = PHASE_ASSOCIATE_CAPACITY;
  vec<PointerCapacityAssociation*, va_gc>* capacity_results = NULL;
  AD_TRY (analyzeAllCapacityAssociations (AD_ARGS, detector, owned_conclusions, array_accesses, capacity_results));

  // Step 10: Aggregate results
  g_pipeline_state.current_phase = PHASE_AGGREGATE_RESULTS;
  vec<UnifiedFieldAnalysisResult*, va_gc>* unified_results = NULL;
  AD_TRY (aggregateAllResults (AD_ARGS, detector, owned_conclusions,
                               capacity_results, array_accesses, unified_results));
  AD_DEBUG_PRINT ("pipeline: %u unified results", (unsigned int)vec_safe_length(unified_results));

  // Store results in context for LTO serialization
  ctx.unified_results = unified_results;

  // Step 11: Print debug results
  g_pipeline_state.current_phase = PHASE_OUTPUT;
  printAllFieldOwnedConclusions (AD_ARGS, ctx.debug_file, owned_conclusions);
  printAllPointerCapacityAssociations (AD_ARGS, ctx.debug_file, capacity_results);
  printAllUnifiedResults (AD_ARGS, ctx.debug_file, unified_results);
  AD_TRY (printResults (AD_ARGS, detector));

  // Step 12: Write unified Racket datum results
  AD_TRY (writeUnifiedResultsToRacketDatum (AD_ARGS, unified_results));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 单阶段执行（预留接口）
// ============================================================================

ArrayDetectErrorCode runPhase (
  AD_FUNC_ARGS,
  AnalysisPhase phase,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  (void)phase;
  (void)detector;
  // TODO: 实现单阶段执行
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 向后兼容：保留旧函数签名
// ============================================================================

ArrayDetectErrorCode runArrayDetectionPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
) {
  return runPipeline (AD_ARGS, detector);
}

// ============================================================================
// 顶层入口：创建检测器并执行分析
// ============================================================================

ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ArrayDetector detector;
  AD_TRY (init (detector, AD_ARGS));
  AD_TRY_LABEL (runPipeline (AD_ARGS, detector), analysis_cleanup);
  analysis_cleanup:
  deinit (detector, AD_ARGS);
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
