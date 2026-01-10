// ============================================================================
// ad-driver 模块实现
// ============================================================================
// 细节驱动逻辑模块
// 驱动单个写入的完整分析流程
// 调用各子分析模块：write-source, source-use, escaped-use, source-escape, ownership-move
// ============================================================================

#include "driver.hh"
#include "field-wrapper.hh"
#include "array-detector.hh"
#include "escaped-use.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 辅助函数实现
// ============================================================================

void initDriverContext (WriteAnalysisDriverContext& ctx) {
  ctx.write_info = nullptr;
  ctx.source = nullptr;
  ctx.uses = nullptr;
  ctx.escape_conclude = nullptr;
  ctx.move_result = nullptr;
  ctx.is_analyzed = false;
  ctx.has_error = false;
}

void cleanupDriverContext (WriteAnalysisDriverContext& ctx) {
  initDriverContext (ctx);
}

void printDriverContext (
  AD_FUNC_ARGS,
  FILE* out,
  WriteAnalysisDriverContext const& driver_ctx
) {
  (void)ctx; (void)gcc_ctx;
  if (!out) return;

  fprintf (out, "=== WriteAnalysisDriverContext ===\n");
  fprintf (out, "  write_info: %p\n", (void*)driver_ctx.write_info);
  fprintf (out, "  source: %p\n", (void*)driver_ctx.source);
  fprintf (out, "  uses: %p\n", (void*)driver_ctx.uses);
  fprintf (out, "  escape_conclude: %p\n", (void*)driver_ctx.escape_conclude);
  fprintf (out, "  move_result: %p\n", (void*)driver_ctx.move_result);
  fprintf (out, "  is_analyzed: %s\n", driver_ctx.is_analyzed ? "true" : "false");
  fprintf (out, "  has_error: %s\n", driver_ctx.has_error ? "true" : "false");
  fprintf (out, "=== End WriteAnalysisDriverContext ===\n");
}

// ============================================================================
// driveWriteAnalysis
// ============================================================================
// 驱动单个写入的完整分析
// 使用新的 Wrapper 结构（存储指针到子模块结果）

ArrayDetectErrorCode driveWriteAnalysis (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  Wrapper_WriteInfo_WriteSource_SourceEscapeConclude*& result
) AD_FUNCTION_BEGIN {
  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // Step 1: 创建 Wrapper
  auto* wrapper = ggc_alloc<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude> ();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (Wrapper_WriteInfo_WriteSource_SourceEscapeConclude));
  wrapper->write_info = write_info;

  // Step 2: 追踪来源
  ArrayDetector dummy_detector;
  AD_TRY (traceWriteSource (AD_ARGS, dummy_detector, write_info,
    &wrapper->write_source));

  // Step 3: 分析使用链 - 直接获取 Wrapper 列表
  vec<Wrapper_SourceUseInfo_Escaped*, va_gc>* uses = NULL;
  AD_TRY (analyzeSourceUse (AD_ARGS, write_info->rhs, write_info->stmt, &uses));
  wrapper->uses = uses;

  // Step 3.5: 提取逃逸信息 - 填充每个 wrapper 的 escaped_info
  AD_TRY (extractEscapedUses (AD_ARGS, wrapper->uses));

  // Step 4: 生成源级逃逸结论
  // 统计所有 uses 中的逃逸
  unsigned int total_escapes = 0;
  unsigned int safe_debug_escapes = 0;
  unsigned int rejecting_escapes = 0;
  bool has_rejecting = false;

  if (wrapper->uses) {
    for (unsigned int i = 0; i < wrapper->uses->length (); i++) {
      Wrapper_SourceUseInfo_Escaped* uw = (*wrapper->uses)[i];
      if (!uw || !uw->use_info) continue;

      if (uw->escaped_info) {
        total_escapes++;
        if (uw->escaped_info->is_safe_debug) {
          safe_debug_escapes++;
        } else {
          rejecting_escapes++;
          has_rejecting = true;
        }
      }
    }
  }

  // 创建 escape_conclude
  wrapper->escape_conclude = ggc_alloc<SourceEscapeConclude> ();
  if (wrapper->escape_conclude) {
    memset (wrapper->escape_conclude, 0, sizeof (SourceEscapeConclude));
    wrapper->escape_conclude->total_escapes = total_escapes;
    wrapper->escape_conclude->safe_debug_escapes = safe_debug_escapes;
    wrapper->escape_conclude->rejecting_escapes = rejecting_escapes;
    wrapper->escape_conclude->has_rejecting_evidence = has_rejecting;
    wrapper->escape_conclude->is_fully_analyzed = true;
  }

  result = wrapper;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// driveAllWriteAnalysis
// ============================================================================
// 驱动所有写入的分析
// 使用新的 Wrapper 结构

ArrayDetectErrorCode driveAllWriteAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

  unsigned int total_driven = 0;
  unsigned int total_success = 0;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldAnalysisData* tfwo = (*iter).second;
    if (!tfwo || !tfwo->writes) continue;

    for (unsigned int i = 0; i < tfwo->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude* wrapper = (*tfwo->writes)[i];
      if (!wrapper) continue;

      total_driven++;

      // Step 1: 追踪来源（如果尚未追踪）
      if (!wrapper->write_source) {
        AD_TRY (traceWriteSource (AD_ARGS, detector, wrapper->write_info,
          &wrapper->write_source));
      }

      // Step 2: 分析使用链（如果尚未分析）
      if (!wrapper->uses && wrapper->write_info) {
        vec<Wrapper_SourceUseInfo_Escaped*, va_gc>* uses = NULL;
        AD_TRY (analyzeSourceUse (AD_ARGS,
          wrapper->write_info->rhs,
          wrapper->write_info->stmt,
          &uses));
        wrapper->uses = uses;

        // Step 2.5: 提取逃逸信息
        AD_TRY (extractEscapedUses (AD_ARGS, wrapper->uses));
      }

      // Step 3: 生成源级逃逸结论（如果尚未生成）
      if (!wrapper->escape_conclude && wrapper->uses) {
        unsigned int total_escapes = 0;
        unsigned int safe_debug_escapes = 0;
        unsigned int rejecting_escapes = 0;
        bool has_rejecting = false;

        for (unsigned int j = 0; j < wrapper->uses->length (); j++) {
          Wrapper_SourceUseInfo_Escaped* uw = (*wrapper->uses)[j];
          if (!uw || !uw->use_info) continue;

          if (uw->escaped_info) {
            total_escapes++;
            if (uw->escaped_info->is_safe_debug) {
              safe_debug_escapes++;
            } else {
              rejecting_escapes++;
              has_rejecting = true;
            }
          }
        }

        wrapper->escape_conclude = ggc_alloc<SourceEscapeConclude> ();
        if (wrapper->escape_conclude) {
          memset (wrapper->escape_conclude, 0, sizeof (SourceEscapeConclude));
          wrapper->escape_conclude->total_escapes = total_escapes;
          wrapper->escape_conclude->safe_debug_escapes = safe_debug_escapes;
          wrapper->escape_conclude->rejecting_escapes = rejecting_escapes;
          wrapper->escape_conclude->has_rejecting_evidence = has_rejecting;
          wrapper->escape_conclude->is_fully_analyzed = true;
        }
      }

      total_success++;
    }
  }

  AD_DEBUG_PRINT ("driver: %u total, %u success", total_driven, total_success);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// driveFieldAnalysis
// ============================================================================
// 驱动单个字段的所有写入分析

ArrayDetectErrorCode driveFieldAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree type,
  tree field_decl,
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude*, va_gc>*& records
) AD_FUNCTION_BEGIN {
  (void)ctx; (void)gcc_ctx;

  if (!detector.m_type_field_writes) {
    records = NULL;
    AD_RETURNE (OK);
  }

  // 查找 (type, field_decl) 的写入操作
  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;

  TypeFieldAnalysisData** tfwo_ptr = detector.m_type_field_writes->get (key);
  if (!tfwo_ptr || !*tfwo_ptr || !(*tfwo_ptr)->writes) {
    records = NULL;
    AD_RETURNE (OK);
  }

  // 返回现有的 wrappers 列表
  records = (*tfwo_ptr)->writes;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// unwrapAnalysis
// ============================================================================
// 从 Wrapper 提取各分析阶段的结果

ArrayDetectErrorCode unwrapAnalysis (
  AD_FUNC_ARGS,
  Wrapper_WriteInfo_WriteSource_SourceEscapeConclude* record,
  WriteAnalysisDriverContext& driver_ctx
) AD_FUNCTION_BEGIN {
  (void)ctx; (void)gcc_ctx;

  initDriverContext (driver_ctx);

  if (!record) {
    driver_ctx.has_error = true;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 从 Wrapper 提取数据到 driver_ctx
  driver_ctx.write_info = record->write_info;
  driver_ctx.source = record->write_source;
  driver_ctx.escape_conclude = record->escape_conclude;
  driver_ctx.uses = record->uses;

  driver_ctx.is_analyzed = true;
  driver_ctx.has_error = false;

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
