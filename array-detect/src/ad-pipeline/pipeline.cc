// ============================================================================
// ad-pipeline 模块实现
// ============================================================================
// 顶层控制流编排模块
// 实现分析阶段调度和 pipeline 执行
//
// 当前支持的分析阶段（对应已定义的新模块）：
// 1. collectAllFieldWrites          -> ad-field-write
// 2. traceFieldAssignments          -> ad-write-source
// 3. collectAllFieldUses            -> ad-source-use-info
// 4. extractAllSourceEscapeUseInfo  -> ad-source-escape-use-info
// 5. synthesizeAllSourceEscapeConclude -> ad-source-escape-conclude
// 6. summarizeAllFieldEscapeConclude   -> ad-field-escape-conclude
// 7. analyzeAllOwnershipMoves       -> ad-ownership-move
//
// 未来扩展（待定义数据流）：
// - owned-verdict, capacity-assoc, array-access, bound-condition, result-aggregator
// ============================================================================

#include "pipeline.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "source-use-info.hh"
#include "source-escape-use-info.hh"
#include "source-escape-conclude.hh"
#include "field-escape-conclude.hh"
#include "ownership-move.hh"
#include "array-detector.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 公开接口实现
// ============================================================================

// 全局 Pipeline 状态（模块内部使用）
static PipelineState g_pipeline_state = {
  PHASE_COLLECT_WRITES,
  0, 0, 0, 0, 0
};

// ----------------------------------------------------------------------------
// getPipelineState
// ----------------------------------------------------------------------------

PipelineState* getPipelineState (AD_FUNC_ARGS) {
  (void)ctx; (void)gcc_ctx;
  return &g_pipeline_state;
}

// ============================================================================
// Pipeline 实现
// ============================================================================
// 当前只包含已定义数据流的模块调用
// 对应 10 个新模块：
// - ad-field-write, ad-write-source, ad-source-use-info
// - ad-source-escape-use-info, ad-source-escape-conclude, ad-field-escape-conclude
// - ad-ownership-move, ad-field-wrapper
// - ad-pipeline, ad-driver

ArrayDetectErrorCode runPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  // ========================================================================
  // Step 1: 收集字段写入 (ad-field-write)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_COLLECT_WRITES;
  AD_TRY (collectAllFieldWrites (AD_ARGS, detector));

  // ========================================================================
  // Step 2: 追踪写入来源 (ad-write-source)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_TRACE_SOURCES;
  AD_TRY (traceFieldAssignments (AD_ARGS, detector));

  // ========================================================================
  // Step 3: 分析使用链 (ad-source-use-info)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_ANALYZE_USES;
  unsigned int total_analyzed = 0;
  AD_TRY (collectAllFieldUses (AD_ARGS, detector, total_analyzed));
  g_pipeline_state.total_escapes_analyzed = total_analyzed;

  // ========================================================================
  // Step 3.5: 提取逃逸使用信息 (ad-source-escape-use-info)
  // ========================================================================
  unsigned int total_extracted = 0;
  AD_TRY (extractAllSourceEscapeUseInfo (AD_ARGS, detector, total_extracted));

  // ========================================================================
  // Step 4: 合成源级逃逸结论 (ad-source-escape-conclude)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_SYNTHESIZE_ESCAPES;
  unsigned int total_synthesized = 0;
  AD_TRY (synthesizeAllSourceEscapeConclude (AD_ARGS, detector, total_synthesized));

  // ========================================================================
  // Step 5: 汇总字段级逃逸结论 (ad-field-escape-conclude)
  // ========================================================================
  unsigned int total_summarized = 0;
  AD_TRY (summarizeAllFieldEscapeConclude (AD_ARGS, detector, total_summarized));

  // ========================================================================
  // Step 6: 分析所有权转移 (ad-ownership-move)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_ANALYZE_OWNERSHIP;
  unsigned int transfer_analyzed = 0;
  unsigned int certain_transfers = 0;
  AD_TRY (analyzeAllOwnershipMoves (AD_ARGS, detector, transfer_analyzed, certain_transfers));
  g_pipeline_state.total_ownership_analyzed = transfer_analyzed;

  // ========================================================================
  // Step 7: 输出调试信息
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_OUTPUT;
  AD_TRY (printResults (AD_ARGS, detector));

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
