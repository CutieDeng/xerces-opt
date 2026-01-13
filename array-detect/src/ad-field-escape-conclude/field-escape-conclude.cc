// ============================================================================
// ad-field-escape-conclude 模块实现
// ============================================================================
// 数据流：field, (listof source-escape-conclude) -> field-escape-conclude
// 汇总单个 (type, field) 的所有写入操作的逃逸信息
// ============================================================================

#include "field-escape-conclude.hh"
#include "write-source.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// countSourceType
// ----------------------------------------------------------------------------
// 统计来源类型分布

void countSourceType (
  ::array_detector::WriteOriginalSource * write_source,
  FieldEscapeConclude * summary
) {
  if (!write_source || !summary) return;

  switch (write_source->source_type) {
    case ::array_detector::SOURCE_FUNCTION_CALL:
      summary->source_function_call++;
      break;
    case ::array_detector::SOURCE_FIELD_ACCESS:
      summary->source_field_access++;
      break;
    case ::array_detector::SOURCE_CONSTANT:
      summary->source_constant++;
      break;
    case ::array_detector::SOURCE_COMPUTATION:
      summary->source_computation++;
      break;
    case ::array_detector::SOURCE_PHI:
      summary->source_phi++;
      break;
    case ::array_detector::SOURCE_UNKNOWN:
    default:
      summary->source_unknown++;
      break;
  }
}

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// summarizeFieldEscapeConclude
// ----------------------------------------------------------------------------
// 汇总字段级逃逸结论
// 输入：type, field_decl, writes (wrapper 列表，escape_conclude 已填充)

ArrayDetectErrorCode summarizeFieldEscapeConclude (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo*, va_gc>* writes,
  FieldEscapeConclude** result
) AD_FUNCTION_BEGIN {
  if (!result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配汇总结构
  auto* summary = ggc_alloc<FieldEscapeConclude> ();
  if (!summary) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (summary, 0, sizeof (FieldEscapeConclude));

  // 设置标识
  summary->type = type;
  summary->field_decl = field_decl;

  // 遍历所有写入 wrapper
  if (writes) {
    for (unsigned int i = 0; i < writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo * wrapper = (*writes)[i];
      if (!wrapper) continue;

      summary->total_field_writes++;

      // 统计来源类型分布（从 write_source 读取）
      countSourceType (wrapper->write_source, summary);

      // 统计逃逸（从 escape_conclude 读取）
      if (wrapper->escape_conclude) {
        summary->total_escapes += wrapper->escape_conclude->total_escapes;
        summary->safe_debug_escapes += wrapper->escape_conclude->safe_debug_escapes;
        summary->rejecting_escapes += wrapper->escape_conclude->rejecting_escapes;

        if (wrapper->escape_conclude->total_escapes > 0) {
          summary->field_writes_with_escape++;
        }
        if (wrapper->escape_conclude->has_rejecting_evidence) {
          summary->field_writes_with_rejecting++;
        }
        if (!wrapper->escape_conclude->is_fully_analyzed) {
          summary->field_writes_without_analysis++;
        }
      } else {
        // 无 escape_conclude 视为未分析
        summary->field_writes_without_analysis++;
      }
    }
  }

  // 计算核心判定
  summary->has_rejecting_evidence = (summary->field_writes_with_rejecting > 0);
  summary->rejection_ratio = (summary->total_field_writes > 0)
    ? (float)summary->field_writes_with_rejecting / (float)summary->total_field_writes
    : 0.0f;

  *result = summary;
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
