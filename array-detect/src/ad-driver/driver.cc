// ============================================================================
// ad-driver 模块实现
// ============================================================================
// 细节驱动逻辑模块
// 驱动单个写入的完整分析流程
// 调用各子分析模块：write-source, source-use, escaped-use, source-escape, ownership-move
// ============================================================================

#include "../../include/ad-driver/driver.hh"
#include "../../include/ad-field-wrapper/field-wrapper.hh"
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

ArrayDetectErrorCode driveWriteAnalysis (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  FieldWriteAnalysisRecord*& result
) AD_FUNCTION_BEGIN {
  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // Step 1: 创建 Wrapper
  FieldWriteAnalysisWrapper* wrapper = NULL;
  AD_TRY (createFieldWriteWrapper (AD_ARGS, write_info, wrapper));
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // Step 2: 追踪来源
  WriteOriginalSource* source = NULL;
  ArrayDetector dummy_detector;
  AD_TRY (traceWriteSource (AD_ARGS, dummy_detector, write_info, source));
  if (source) {
    AD_TRY (fillWrapperSource (AD_ARGS, wrapper, source));
  }

  // Step 3: 分析使用链
  SourceUseResult* use_result = NULL;
  AD_TRY (analyzeSourceUse (AD_ARGS, write_info->rhs, write_info->stmt, write_info->stmt, use_result));

  // Step 4: 提取逃逸使用
  EscapedUseResult* escaped_uses = NULL;
  if (use_result) {
    AD_TRY (extractEscapedUses (AD_ARGS, use_result, escaped_uses));
    AD_TRY (fillWrapperUseAnalysis (AD_ARGS, wrapper, use_result, escaped_uses));
  }

  // Step 5: 生成源级逃逸结论
  SourceEscapeConclude* escape_conclude = NULL;
  if (escaped_uses) {
    AD_TRY (generateSourceEscapeConclude (AD_ARGS, escaped_uses,
      write_info->type, write_info->field_decl, write_info->location,
      escape_conclude));
    if (escape_conclude) {
      AD_TRY (fillWrapperEscapeConclude (AD_ARGS, wrapper, escape_conclude));
    }
  }

  // Step 6: 所有权转移分析（仅当来源是字段访问时）
  if (source && source->source_type == SOURCE_FIELD_ACCESS) {
    OwnershipMoveResult* move_result = NULL;
    AD_TRY (analyzeOwnershipMove (AD_ARGS, write_info, source, move_result));
    if (move_result) {
      AD_TRY (fillWrapperOwnershipMove (AD_ARGS, wrapper, move_result));
    }
  }

  result = wrapper;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// driveAllWriteAnalysis
// ============================================================================
// 驱动所有写入的分析

ArrayDetectErrorCode driveAllWriteAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  unsigned int total_driven = 0;
  unsigned int total_success = 0;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps* tfwo = (*iter).second;
    if (!tfwo || !tfwo->writes) continue;

    for (unsigned int i = 0; i < tfwo->writes->length (); i++) {
      FieldWriteAnalysisWrapper* wrapper = (*tfwo->writes)[i];
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
        WriteOriginalSource* source = NULL;
        AD_TRY (traceWriteSource (AD_ARGS, detector, &temp_write_info, source));
        if (source) {
          AD_TRY (fillWrapperSource (AD_ARGS, wrapper, source));
        }
      }

      // Step 2: 分析使用链（如果尚未分析）
      if (!wrapper->all_uses) {
        SourceUseResult* use_result = NULL;
        AD_TRY (analyzeSourceUse (AD_ARGS, wrapper->rhs, wrapper->stmt, wrapper->stmt, use_result));

        if (use_result) {
          EscapedUseResult* escaped_uses = NULL;
          AD_TRY (extractEscapedUses (AD_ARGS, use_result, escaped_uses));
          AD_TRY (fillWrapperUseAnalysis (AD_ARGS, wrapper, use_result, escaped_uses));

          // Step 3: 生成源级逃逸结论
          if (escaped_uses) {
            SourceEscapeConclude* escape_conclude = NULL;
            AD_TRY (generateSourceEscapeConclude (AD_ARGS, escaped_uses,
              wrapper->type, wrapper->field, wrapper->write_location,
              escape_conclude));
            if (escape_conclude) {
              AD_TRY (fillWrapperEscapeConclude (AD_ARGS, wrapper, escape_conclude));
            }
          }
        }
      }

      // Step 4: 所有权转移分析（如果尚未分析且来源是字段访问）
      if (!wrapper->move && wrapper->source_kind == FIELD_SRC_FIELD_ACCESS) {
        WriteOriginalSource temp_source;
        memset (&temp_source, 0, sizeof (WriteOriginalSource));
        temp_source.source_type = SOURCE_FIELD_ACCESS;
        temp_source.data.field_access.access_stmt = wrapper->source_data.field_access.stmt;
        temp_source.data.field_access.field_decl = wrapper->source_data.field_access.field;
        temp_source.data.field_access.base_object = wrapper->source_data.field_access.object;
        temp_source.data.field_access.object_type = wrapper->source_data.field_access.object_type;
        temp_source.data.field_access.field_name = wrapper->source_data.field_access.field_name;
        temp_source.data.field_access.type_name = wrapper->source_data.field_access.type_name;
        temp_source.data.field_access.location = wrapper->source_data.field_access.location;

        OwnershipMoveResult* move_result = NULL;
        AD_TRY (analyzeOwnershipMove (AD_ARGS, &temp_write_info, &temp_source, move_result));
        if (move_result) {
          AD_TRY (fillWrapperOwnershipMove (AD_ARGS, wrapper, move_result));
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
  vec<FieldWriteAnalysisRecord*>*& records
) AD_FUNCTION_BEGIN {
  if (!detector.m_type_field_writes) {
    records = NULL;
    AD_RETURNE (OK);
  }

  // 查找 (type, field_decl) 的写入操作
  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;

  TypeFieldWriteOps** tfwo_ptr = detector.m_type_field_writes->get (key);
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
// 从 FieldWriteAnalysisRecord 提取各分析阶段的结果

ArrayDetectErrorCode unwrapAnalysis (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  WriteAnalysisDriverContext& driver_ctx
) AD_FUNCTION_BEGIN {
  (void)ctx; (void)gcc_ctx;

  initDriverContext (driver_ctx);

  if (!record) {
    driver_ctx.has_error = true;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // FieldWriteAnalysisRecord 是 FieldWriteAnalysisWrapper 的别名
  // Wrapper 是扁平化结构，包含了所有分析阶段的结果
  // 这里只标记已分析

  driver_ctx.is_analyzed = true;
  driver_ctx.has_error = false;

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
