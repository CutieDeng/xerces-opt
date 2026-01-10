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
  ctx.use_result = nullptr;
  ctx.escaped_uses = nullptr;
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
  fprintf (out, "  use_result: %p\n", (void*)driver_ctx.use_result);
  fprintf (out, "  escaped_uses: %p\n", (void*)driver_ctx.escaped_uses);
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
// 直接写入 wrapper 成员地址，不使用 fill 函数

ArrayDetectErrorCode driveWriteAnalysis (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude*& result
) AD_FUNCTION_BEGIN {
  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // Step 1: 创建 Wrapper
  Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude* wrapper = NULL;
  AD_TRY (createFieldWriteWrapper (AD_ARGS, write_info, wrapper));
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // Step 2: 追踪来源 - 直接写入 wrapper 成员
  ArrayDetector dummy_detector;
  AD_TRY (traceWriteSource (AD_ARGS, dummy_detector, write_info,
    &wrapper->source_kind, &wrapper->source_data));

  // Step 3: 分析使用链 - 直接写入 wrapper 成员
  AD_TRY (analyzeSourceUse (AD_ARGS, write_info->rhs, write_info->stmt, write_info->stmt,
    &wrapper->source_operand,
    &wrapper->source_stmt,
    &wrapper->all_uses,
    &wrapper->total_use_count,
    &wrapper->max_use_depth,
    &wrapper->is_fully_analyzed
  ));

  // Step 4: 提取逃逸使用 - 直接写入 wrapper 成员
  AD_TRY (extractEscapedUses (AD_ARGS, wrapper->all_uses,
    &wrapper->escape_uses,
    &wrapper->escape_count,
    &wrapper->has_escape
  ));

  // Step 5: 生成源级逃逸结论 - 直接写入 wrapper 成员
  AD_TRY (generateSourceEscapeConclude (AD_ARGS, wrapper->escape_uses,
    &wrapper->total_escapes,
    &wrapper->safe_debug_escapes,
    &wrapper->rejecting_escapes,
    &wrapper->has_rejecting_evidence
  ));

  // Step 6: 所有权转移分析 - 直接写入 wrapper->move
  AD_TRY (analyzeOwnershipMove (AD_ARGS, write_info,
    wrapper->source_kind,
    &wrapper->source_data.field_access,
    &wrapper->move
  ));

  result = wrapper;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// driveAllWriteAnalysis
// ============================================================================
// 驱动所有写入的分析
// 直接写入 wrapper 成员地址

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
      Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude* wrapper = (*tfwo->writes)[i];
      if (!wrapper) continue;

      total_driven++;

      // 从 wrapper 创建临时 FieldWriteInfo
      FieldWriteInfo temp_write_info;
      memset (&temp_write_info, 0, sizeof (FieldWriteInfo));
      temp_write_info.type = wrapper->type;
      temp_write_info.field_decl = wrapper->field;
      temp_write_info.function_decl = wrapper->func;
      temp_write_info.bb = wrapper->bb;
      temp_write_info.stmt = wrapper->stmt;
      temp_write_info.lhs = wrapper->lhs;
      temp_write_info.rhs = wrapper->rhs;
      temp_write_info.location = wrapper->write_location;

      // Step 1: 追踪来源（如果尚未追踪）
      if (wrapper->source_kind == FIELD_SRC_UNKNOWN) {
        AD_TRY (traceWriteSource (AD_ARGS, detector, &temp_write_info,
          &wrapper->source_kind, &wrapper->source_data));
      }

      // Step 2: 分析使用链（如果尚未分析）
      if (!wrapper->all_uses) {
        AD_TRY (analyzeSourceUse (AD_ARGS, wrapper->rhs, wrapper->stmt, wrapper->stmt,
          &wrapper->source_operand,
          &wrapper->source_stmt,
          &wrapper->all_uses,
          &wrapper->total_use_count,
          &wrapper->max_use_depth,
          &wrapper->is_fully_analyzed
        ));

        // Step 3: 提取逃逸使用
        AD_TRY (extractEscapedUses (AD_ARGS, wrapper->all_uses,
          &wrapper->escape_uses,
          &wrapper->escape_count,
          &wrapper->has_escape
        ));

        // Step 4: 生成源级逃逸结论
        AD_TRY (generateSourceEscapeConclude (AD_ARGS, wrapper->escape_uses,
          &wrapper->total_escapes,
          &wrapper->safe_debug_escapes,
          &wrapper->rejecting_escapes,
          &wrapper->has_rejecting_evidence
        ));
      }

      // Step 5: 所有权转移分析（如果尚未分析）
      if (!wrapper->move) {
        AD_TRY (analyzeOwnershipMove (AD_ARGS, &temp_write_info,
          wrapper->source_kind,
          &wrapper->source_data.field_access,
          &wrapper->move
        ));
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
  vec<Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude*>*& records
) AD_FUNCTION_BEGIN {
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
// 从 Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude 提取各分析阶段的结果

ArrayDetectErrorCode unwrapAnalysis (
  AD_FUNC_ARGS,
  Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude* record,
  WriteAnalysisDriverContext& driver_ctx
) AD_FUNCTION_BEGIN {
  (void)ctx; (void)gcc_ctx;

  initDriverContext (driver_ctx);

  if (!record) {
    driver_ctx.has_error = true;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude 是 Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude 的别名
  // Wrapper 是扁平化结构，包含了所有分析阶段的结果
  // 这里只标记已分析

  driver_ctx.is_analyzed = true;
  driver_ctx.has_error = false;

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
