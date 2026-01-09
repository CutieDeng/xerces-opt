// ============================================================================
// ad-field-wrapper 模块实现
// ============================================================================
// Field 分析聚合数据结构和 Wrapper 组装函数
// 数据流：各分析阶段结果 -> FieldWriteAnalysisWrapper
// ============================================================================

#include "field-wrapper.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "source-use.hh"
#include "escaped-use.hh"
#include "source-escape.hh"
#include "ownership-move.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// fillWrapperUseAnalysis_convertEscapeKind
// ----------------------------------------------------------------------------
// 转换逃逸类型

FieldEscapeKind fillWrapperUseAnalysis_convertEscapeKind (SourceUseEscapeKind kind) {
  switch (kind) {
    case SU_ESCAPE_NONE:          return FIELD_ESC_NONE;
    case SU_ESCAPE_RETURN:        return FIELD_ESC_RETURN;
    case SU_ESCAPE_PARAMETER:     return FIELD_ESC_PARAMETER;
    case SU_ESCAPE_GLOBAL_STORE:  return FIELD_ESC_GLOBAL;
    case SU_ESCAPE_HEAP_STORE:    return FIELD_ESC_HEAP;
    case SU_ESCAPE_FIELD_STORE:   return FIELD_ESC_FIELD;
    case SU_ESCAPE_INDIRECT_CALL: return FIELD_ESC_INDIRECT_CALL;
    case SU_ESCAPE_VIRTUAL_CALL:  return FIELD_ESC_VIRTUAL_CALL;
    case SU_ESCAPE_EXTERNAL_CALL: return FIELD_ESC_EXTERNAL_CALL;
    case SU_ESCAPE_UNKNOWN:       return FIELD_ESC_UNKNOWN;
    default:                      return FIELD_ESC_UNKNOWN;
  }
}

// ----------------------------------------------------------------------------
// fillWrapperUseAnalysis_convertUseKind
// ----------------------------------------------------------------------------
// 转换使用类型

FieldUseKind fillWrapperUseAnalysis_convertUseKind (SourceUseKind kind) {
  switch (kind) {
    case SU_USE_LOAD:         return FIELD_USE_LOAD;
    case SU_USE_STORE:        return FIELD_USE_STORE;
    case SU_USE_CALL_ARG:     return FIELD_USE_CALL_ARG;
    case SU_USE_RETURN:       return FIELD_USE_RETURN;
    case SU_USE_PHI:          return FIELD_USE_PHI;
    case SU_USE_ASSIGN:       return FIELD_USE_ASSIGN;
    case SU_USE_ARITHMETIC:   return FIELD_USE_ARITHMETIC;
    case SU_USE_COMPARISON:   return FIELD_USE_COMPARISON;
    case SU_USE_ADDRESS_TAKEN: return FIELD_USE_ADDRESS;
    case SU_USE_CONDITIONAL:  return FIELD_USE_CONDITIONAL;
    case SU_USE_OTHER:        return FIELD_USE_OTHER;
    default:                  return FIELD_USE_OTHER;
  }
}

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// createFieldWriteWrapper
// ----------------------------------------------------------------------------
// 创建并初始化空的 Wrapper

ArrayDetectErrorCode createFieldWriteWrapper (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  FieldWriteAnalysisWrapper*& wrapper
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配 Wrapper
  wrapper = ggc_alloc<FieldWriteAnalysisWrapper> ();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (FieldWriteAnalysisWrapper));

  // 填充 FieldWrite 部分
  wrapper->type = write_info->type;
  wrapper->field = write_info->field_decl;
  wrapper->func = write_info->function_decl;
  wrapper->bb = write_info->bb;
  wrapper->stmt = write_info->stmt;
  wrapper->lhs = write_info->lhs;
  wrapper->rhs = write_info->rhs;
  wrapper->write_location = write_info->location;

  // 初始化其他部分为默认值
  wrapper->source_kind = FIELD_SRC_UNKNOWN;
  wrapper->source_operand = NULL_TREE;
  wrapper->source_stmt = NULL;
  wrapper->all_uses = NULL;
  wrapper->total_use_count = 0;
  wrapper->max_use_depth = 0;
  wrapper->is_fully_analyzed = false;
  wrapper->escape_uses = NULL;
  wrapper->escape_count = 0;
  wrapper->has_escape = false;
  wrapper->total_escapes = 0;
  wrapper->safe_debug_escapes = 0;
  wrapper->rejecting_escapes = 0;
  wrapper->has_rejecting_evidence = false;
  wrapper->move = NULL;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// fillWrapperSource
// ============================================================================
// 填充来源部分

ArrayDetectErrorCode fillWrapperSource (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  WriteOriginalSource* source
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!wrapper || !source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 转换 SourceType -> FieldSourceKind
  switch (source->source_type) {
    case SOURCE_FUNCTION_CALL:
      wrapper->source_kind = FIELD_SRC_FUNCTION_CALL;
      wrapper->source_data.function_call.stmt = source->data.function_call.call_stmt;
      switch (source->data.function_call.call_type) {
        case CALL_VIRTUAL:
          wrapper->source_data.function_call.call_kind = FIELD_CALL_VIRTUAL;
          break;
        case CALL_DIRECT:
          wrapper->source_data.function_call.call_kind = FIELD_CALL_DIRECT;
          break;
        case CALL_INDIRECT:
          wrapper->source_data.function_call.call_kind = FIELD_CALL_INDIRECT;
          break;
        default:
          wrapper->source_data.function_call.call_kind = FIELD_CALL_UNKNOWN;
          break;
      }
      wrapper->source_data.function_call.name = source->data.function_call.function_name;
      wrapper->source_data.function_call.location = source->data.function_call.location;
      break;

    case SOURCE_CONSTANT:
      wrapper->source_kind = FIELD_SRC_CONSTANT;
      wrapper->source_data.constant.value = source->data.constant.constant_value;
      wrapper->source_data.constant.str = source->data.constant.constant_str;
      break;

    case SOURCE_FIELD_ACCESS:
      wrapper->source_kind = FIELD_SRC_FIELD_ACCESS;
      wrapper->source_data.field_access.stmt = source->data.field_access.access_stmt;
      wrapper->source_data.field_access.field = source->data.field_access.field_decl;
      wrapper->source_data.field_access.object = source->data.field_access.base_object;
      wrapper->source_data.field_access.object_type = source->data.field_access.object_type;
      wrapper->source_data.field_access.field_name = source->data.field_access.field_name;
      wrapper->source_data.field_access.type_name = source->data.field_access.type_name;
      wrapper->source_data.field_access.location = source->data.field_access.location;
      break;

    case SOURCE_COMPUTATION:
      wrapper->source_kind = FIELD_SRC_COMPUTATION;
      wrapper->source_data.computation.stmt = source->data.computation.compute_stmt;
      wrapper->source_data.computation.expr = source->data.computation.compute_expr;
      wrapper->source_data.computation.desc = source->data.computation.description;
      wrapper->source_data.computation.location = source->data.computation.location;
      break;

    case SOURCE_PHI:
      wrapper->source_kind = FIELD_SRC_PHI;
      wrapper->source_data.phi.stmt = source->data.phi.phi_stmt;
      wrapper->source_data.phi.ssa_name = source->data.phi.ssa_name;
      wrapper->source_data.phi.var = source->data.phi.var_decl;
      wrapper->source_data.phi.var_name = source->data.phi.var_name;
      wrapper->source_data.phi.location = source->data.phi.location;
      break;

    default:
      wrapper->source_kind = FIELD_SRC_UNKNOWN;
      break;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// fillWrapperUseAnalysis
// ----------------------------------------------------------------------------
// 填充使用分析部分

ArrayDetectErrorCode fillWrapperUseAnalysis (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  SourceUseResult* use_result,
  EscapedUseResult* escaped_uses
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;
  (void)escaped_uses;  // 逃逸使用从 use_result 中提取

  if (!wrapper || !use_result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 填充基本信息
  wrapper->source_operand = use_result->source_operand;
  wrapper->source_stmt = use_result->source_stmt;
  wrapper->total_use_count = use_result->total_use_count;
  wrapper->max_use_depth = use_result->max_use_depth;
  wrapper->is_fully_analyzed = use_result->is_fully_analyzed;
  wrapper->escape_count = use_result->escape_count;
  wrapper->has_escape = use_result->has_escape;

  // 转换并复制 all_uses
  if (use_result->all_uses && use_result->all_uses->length () > 0) {
    wrapper->all_uses = ggc_alloc<vec<FieldUsePoint>> ();
    if (!wrapper->all_uses) {
      AD_RETURNE (MEMORY_ERROR);
    }
    wrapper->all_uses->create (use_result->all_uses->length ());

    // 同时创建 escape_uses 列表
    wrapper->escape_uses = ggc_alloc<vec<FieldUsePoint const*>> ();
    if (!wrapper->escape_uses) {
      AD_RETURNE (MEMORY_ERROR);
    }
    wrapper->escape_uses->create (0);

    for (unsigned i = 0; i < use_result->all_uses->length (); i++) {
      SourceUseInfo const& src = (*use_result->all_uses)[i];
      FieldUsePoint fp;
      fp.kind = fillWrapperUseAnalysis_convertUseKind (src.kind);
      fp.stmt = src.use_stmt;
      fp.operand = src.use_operand;
      fp.location = src.source_location;
      fp.bb_index = src.bb_index;
      fp.escape_kind = fillWrapperUseAnalysis_convertEscapeKind (src.escape_kind);
      fp.escape_target = src.escape_target;
      wrapper->all_uses->safe_push (fp);

      // 如果是逃逸使用，添加到 escape_uses
      if (src.is_escape ()) {
        wrapper->escape_uses->safe_push (&(*wrapper->all_uses)[wrapper->all_uses->length () - 1]);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// fillWrapperEscapeConclude
// ============================================================================
// 填充逃逸结论部分

ArrayDetectErrorCode fillWrapperEscapeConclude (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  SourceEscapeConclude* conclude
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!wrapper || !conclude) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  wrapper->total_escapes = conclude->total_escapes;
  wrapper->safe_debug_escapes = conclude->safe_debug_escapes;
  wrapper->rejecting_escapes = conclude->rejecting_escapes;
  wrapper->has_rejecting_evidence = conclude->has_rejecting_evidence;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// fillWrapperOwnershipMove
// ============================================================================
// 填充所有权转移部分

ArrayDetectErrorCode fillWrapperOwnershipMove (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  OwnershipMoveResult* move_result
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!wrapper) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  if (!move_result) {
    // 可选，没有所有权分析结果
    wrapper->move = NULL;
    AD_RETURNE (OK);
  }

  // 创建 FieldMoveAnalysis
  FieldMoveAnalysis* field_move = ggc_alloc<FieldMoveAnalysis> ();
  if (!field_move) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (field_move, 0, sizeof (FieldMoveAnalysis));

  // 填充数据
  field_move->source_field = move_result->source_field;
  field_move->source_object = move_result->source_object;
  field_move->transfer_stmt = move_result->transfer_stmt;
  field_move->transfer_location = move_result->transfer_location;

  // 转换销毁点（简化：不复制详细信息）
  field_move->invalidations = NULL;

  // 转换判定
  switch (move_result->verdict) {
    case MOVE_CERTAIN:
      field_move->verdict = FIELD_MOVE_CERTAIN;
      break;
    case MOVE_IMPOSSIBLE:
      field_move->verdict = FIELD_MOVE_IMPOSSIBLE;
      break;
    case MOVE_CONDITIONAL:
      field_move->verdict = FIELD_MOVE_CONDITIONAL;
      break;
    default:
      field_move->verdict = FIELD_MOVE_NOT_APPLICABLE;
      break;
  }

  field_move->paths_with = move_result->paths_with_invalidation;
  field_move->paths_without = move_result->paths_without_invalidation;
  field_move->total_paths = move_result->total_exit_paths;
  field_move->verdict_desc = move_result->verdict_description;

  wrapper->move = field_move;

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
