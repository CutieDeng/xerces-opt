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
// 7. analyzeTransferInfo           -> ad-ownership-move (per write)
// 8. summarizeTransferStats         -> ad-transfer-stats (per field)
// 9. groupMallocEvidences           -> ad-malloc-group (per field)
// 10. groupReadEvidences            -> ad-read-group (per field)
// 11. groupWriteEvidences           -> ad-write-group (per field)
// 12. generateCapacityConclude      -> ad-capacity-conclude (per field)
// ============================================================================

#include "pipeline.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "source-use-info.hh"
#include "source-escape-use-info.hh"
#include "source-escape-conclude.hh"
#include "field-escape-conclude.hh"
#include "transfer-collect.hh"
#include "transfer-stats.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "malloc-capacity.hh"
#include "malloc-group.hh"
#include "array-read-collect.hh"
#include "array-read-bound.hh"
#include "read-group.hh"
#include "array-write-collect.hh"
#include "array-write-bound.hh"
#include "write-group.hh"
#include "capacity-conclude.hh"
#include "own-conclude.hh"
#include "lto-summary.hh"
#include "result-output.hh"

// For flag_generate_lto
#include "options.h"

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
        Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo * wrapper = (*tfad->writes)[i];
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
        Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo * wrapper = (*tfad->writes)[i];
        if (!wrapper || !wrapper->write_info) continue;

        // 只处理源为字段访问的写入
        if (wrapper->write_source &&
            wrapper->write_source->source_type == SOURCE_FIELD_ACCESS) {
          AD_TRY (collectTransfer (
            AD_ARGS,
            wrapper->write_info,
            wrapper->write_source,
            &wrapper->transfer_info
          ));

          if (wrapper->transfer_info) {
            transfer_analyzed++;
            // 打印调试信息
            printTransferInfo (AD_ARGS, ctx.debug_file, wrapper->transfer_info);

            if (wrapper->transfer_info->verdict == TRANSFER_CERTAIN) {
              certain_transfers++;
            }
          }
        }
      }

      // Step 5b: 汇总字段级所有权转移统计 (per field)
      AD_TRY (summarizeTransferStats (
        AD_ARGS,
        tfad->type,
        tfad->field_decl,
        tfad->writes,
        &tfad->transfer_stats
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

      AD_TRY (groupMallocEvidencesForFieldWrapper (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("mallocCapacity: analyzed all type-field pairs");
  }

  // ========================================================================
  // Step 7: 分析数组读容量关联 (ad-read-group)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_READ_CAPACITY;

  // Step 7a: 扫描整个程序一次，收集所有数组读取（直接填充到 Wrapper）
  AD_TRY (scanAllArrayReadAccesses (AD_ARGS, detector.m_type_field_writes));

  // 统计总数
  unsigned total_read_accesses = 0;
  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;
    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (tfad && tfad->array_reads) {
        total_read_accesses += tfad->array_reads->length ();
      }
    }
  }
  AD_DEBUG_PRINT ("readCapacity: scanned %u array read accesses", total_read_accesses);

  // Step 7b: 为每个 (type, field) 分组读取证据
  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (groupReadEvidencesForFieldWrapper (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("readCapacity: grouped all type-field pairs");
  }

  // ========================================================================
  // Step 8: 分析数组写容量关联 (ad-write-group)
  // ========================================================================
  g_pipeline_state.current_phase = PHASE_WRITE_CAPACITY;

  // Step 8a: 扫描整个程序一次，收集所有数组写入（直接填充到 Wrapper）
  AD_TRY (scanAllArrayWriteAccesses (AD_ARGS, detector.m_type_field_writes));

  // 统计总数
  unsigned total_write_accesses = 0;
  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;
    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (tfad && tfad->array_writes) {
        total_write_accesses += tfad->array_writes->length ();
      }
    }
  }
  AD_DEBUG_PRINT ("writeCapacity: scanned %u array write accesses", total_write_accesses);

  // Step 8b: 为每个 (type, field) 分组写入证据
  if (detector.m_type_field_writes) {
    typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

    for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
         iter != detector.m_type_field_writes->end ();
         ++iter) {
      TypeFieldAnalysisData * tfad = (*iter).second;
      if (!tfad) continue;

      AD_TRY (groupWriteEvidencesForFieldWrapper (AD_ARGS, tfad));
    }

    AD_DEBUG_PRINT ("writeCapacity: grouped all type-field pairs");
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
  // Step 10b: LTO summary 转换 (仅在 -flto 编译时)
  // ========================================================================
  if (flag_generate_lto && owned_conclusions) {
    vec<LtoUnifiedResultSummary*, va_gc>* lto_summaries =
      convertAllFieldOwnedConclusionsToLtoSummaries (AD_ARGS, owned_conclusions, detector);
    if (lto_summaries && lto_summaries->length () > 0) {
      setWpaLtoSummaries (lto_summaries);
      AD_DEBUG_PRINT ("ltoSummary: %u summaries prepared for serialization",
                      (unsigned)lto_summaries->length ());
    }
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
      if (tfad->malloc_evidences_map) {
        printMallocEvidenceGroups (AD_ARGS, ctx.debug_file, tfad->malloc_evidences_map);
      }
      if (tfad->read_evidences_map) {
        printReadEvidenceGroups (AD_ARGS, ctx.debug_file, tfad->read_evidences_map);
      }
      if (tfad->write_evidences_map) {
        printWriteEvidenceGroups (AD_ARGS, ctx.debug_file, tfad->write_evidences_map);
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

  // ========================================================================
  // Step 12: 输出结果到 AD_RESULT_FILE (非 LTO 模式)
  // ========================================================================
  AD_TRY (writeResultsToFile (AD_ARGS, owned_conclusions, detector));

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
