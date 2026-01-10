// ============================================================================
// ad-pipeline 模块实现
// ============================================================================
// 顶层控制流编排模块
// 实现分析阶段调度和 pipeline 执行
//
// Pipeline 负责 (listof A) -> (listof B) 的批量遍历
// 各子模块只负责 A -> B 的单项转换
//
// 当前支持的分析阶段：
// 1. collectAllFieldWrites          -> ad-field-write
// 2. traceFieldAssignments          -> ad-write-source
// 3. collectAllFieldUses            -> ad-source-use-info
// 4. extractSourceEscapeUseInfo     -> ad-source-escape-use-info (per write)
// 5. synthesizeSourceEscapeConclude -> ad-source-escape-conclude (per write)
// 6. summarizeFieldEscapeConclude   -> ad-field-escape-conclude (per field)
// 7. analyzeOwnershipMove           -> ad-ownership-move (per write)
// 8. summarizeOwnershipConclude     -> ad-ownership-conclude (per field)
// 9. collectAllMallocEvidences      -> ad-field-write-capacity (per field)
// 10. collectAllReadEvidences       -> ad-array-read-capacity (per field)
// 11. collectAllWriteEvidences      -> ad-array-write-capacity (per field)
// 12. generateCapacityConclude      -> ad-capacity-conclude (per field)
// ============================================================================

#include "pipeline.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "source-use-info.hh"
#include "source-escape-use-info.hh"
#include "source-escape-conclude.hh"
#include "field-escape-conclude.hh"
#include "ownership-move.hh"
#include "ownership-conclude.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "field-write-capacity.hh"
#include "array-read-capacity.hh"
#include "array-write-capacity.hh"
#include "capacity-conclude.hh"
#include "owned-conclusion.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

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
  // Step 4: 提取逃逸使用信息 + 合成源级结论 + 汇总字段结论
  // (ad-source-escape-use-info, ad-source-escape-conclude, ad-field-escape-conclude)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_SYNTHESIZE_ESCAPES;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad || !tfad->writes) continue;

      // 遍历该字段的所有写入
      for (unsigned i = 0; i < tfad->writes->length (); i++) {
        Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove * wrapper = (*tfad->writes)[i];
        if (!wrapper) continue;

        // Step 4a: 提取逃逸使用信息 (per write)
        if (wrapper->uses) {
          AD_TRY (extractSourceEscapeUseInfo (AD_ARGS, wrapper->uses));
        }

        // Step 4b: 合成源级逃逸结论 (per write)
        if (!wrapper->escape_conclude && wrapper->uses) {
          AD_TRY (synthesizeSourceEscapeConclude (AD_ARGS, wrapper->uses, &wrapper->escape_conclude));
        }
      }

      // Step 4c: 汇总字段级逃逸结论 (per field)
      AD_TRY (summarizeFieldEscapeConclude (
        AD_ARGS,
        tfad->type,
        tfad->field_decl,
        tfad->writes,
        &tfad->escape_conclude
      ));
    }
  }

  // ========================================================================
  // Step 5: 分析所有权转移 (ad-ownership-move)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_ANALYZE_OWNERSHIP;
  unsigned int transfer_analyzed = 0;
  unsigned int certain_transfers = 0;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad || !tfad->writes) continue;

      // Step 5a: 分析每个写入的所有权转移 (per write)
      for (unsigned i = 0; i < tfad->writes->length (); i++) {
        Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove * wrapper = (*tfad->writes)[i];
        if (!wrapper || !wrapper->write_info) continue;

        // 只处理源为字段访问的写入
        if (wrapper->write_source &&
            wrapper->write_source->source_type == SOURCE_FIELD_ACCESS) {
          AD_TRY (analyzeOwnershipMove (
            AD_ARGS,
            wrapper->write_info,
            wrapper->write_source,
            &wrapper->ownership_move
          ));

          if (wrapper->ownership_move) {
            transfer_analyzed++;
            // 打印调试信息
            printOwnershipMove (AD_ARGS, ctx.debug_file, wrapper->ownership_move);

            if (wrapper->ownership_move->verdict == MOVE_CERTAIN) {
              certain_transfers++;
            }
          }
        }
      }

      // Step 5b: 汇总字段级所有权结论 (per field)
      AD_TRY (summarizeOwnershipConclude (
        AD_ARGS,
        tfad->type,
        tfad->field_decl,
        tfad->writes,
        &tfad->ownership_conclude
      ));
    }
  }
  g_pipeline_state.total_ownership_analyzed = transfer_analyzed;

  AD_DEBUG_PRINT ("ownershipMove: %u analyzed, %u certain",
                  transfer_analyzed, certain_transfers);

  // ========================================================================
  // Step 6: 分析 malloc 容量关联 (ad-field-write-capacity)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_MALLOC_CAPACITY;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (collectAllMallocEvidences (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("mallocCapacity: analyzed all type-field pairs");
  }

  // ========================================================================
  // Step 7: 分析数组读容量关联 (ad-array-read-capacity)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_READ_CAPACITY;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (collectAllReadEvidences (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("readCapacity: analyzed all type-field pairs");
  }

  // ========================================================================
  // Step 8: 分析数组写容量关联 (ad-array-write-capacity)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_WRITE_CAPACITY;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (collectAllWriteEvidences (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("writeCapacity: analyzed all type-field pairs");
  }

  // ========================================================================
  // Step 9: 汇总容量结论 (ad-capacity-conclude)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_CAPACITY_CONCLUDE;

  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (generateCapacityConclude (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("capacityConclude: generated all conclusions");
  }

  // ========================================================================
  // Step 10: 生成 owned 判定 (ad-owned-conclusion)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_GENERATE_VERDICT;

  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions = nullptr;
  AD_TRY (analyzeAllFieldOwnedConclusions (AD_ARGS, detector, owned_conclusions));

  if (owned_conclusions) {
    AD_DEBUG_PRINT ("ownedConclusion: %u fields analyzed",
                    (unsigned)owned_conclusions->length ());
  }

  // ========================================================================
  // Step 11: 输出调试信息
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_OUTPUT;
  AD_TRY (printResults (AD_ARGS, detector));

  // 输出 malloc 容量证据
  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      // 输出三种证据
      if (tfad->malloc_evidences) {
        printAllMallocEvidences (AD_ARGS, ctx.debug_file, tfad->malloc_evidences);
      }
      if (tfad->read_evidences) {
        printAllReadEvidences (AD_ARGS, ctx.debug_file, tfad->read_evidences);
      }
      if (tfad->write_evidences) {
        printAllWriteEvidences (AD_ARGS, ctx.debug_file, tfad->write_evidences);
      }
      // 输出容量结论
      if (tfad->capacity_conclude) {
        printCapacityConclude (AD_ARGS, ctx.debug_file, tfad->capacity_conclude);
      }
    }
  }

  // 输出 owned 结论
  if (owned_conclusions) {
    printAllFieldOwnedConclusions (AD_ARGS, ctx.debug_file, owned_conclusions);
  }

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
