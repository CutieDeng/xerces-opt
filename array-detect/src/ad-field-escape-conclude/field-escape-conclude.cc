// ============================================================================
// ad-field-escape-conclude 模块实现
// ============================================================================
// 数据流：field, (listof write-original-source) -> field-escape-conclude
// 汇总单个 (type, field) 的所有写入操作的逃逸信息
// ============================================================================

#include "field-escape-conclude.hh"
#include "source-escape-conclude.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "gcc-ext-util.hh"
#include "write-source.hh"
#include "field-wrapper.hh"

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
// summarizeFieldEscape
// ----------------------------------------------------------------------------
// 字段逃逸结论汇总：TypeFieldAnalysisData -> FieldEscapeConclude

ArrayDetectErrorCode summarizeFieldEscape (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
) AD_FUNCTION_BEGIN {
  if (!field_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配汇总结构
  FieldEscapeConclude * summary = ggc_alloc<FieldEscapeConclude> ();
  if (!summary) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (summary, 0, sizeof (FieldEscapeConclude));

  // 设置标识
  summary->type = field_data->type;
  summary->field_decl = field_data->field_decl;

  // 遍历所有字段写入分析 Wrapper
  if (field_data->writes) {
    for (unsigned int i = 0; i < field_data->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*field_data->writes)[i];
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

  AD_RETURNO (summary);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// synthesizeAllFieldEscapes
// ----------------------------------------------------------------------------
// 综合所有字段的逃逸信息
// 前置条件：escape_use_info 已由 extractAllSourceEscapeUseInfo 填充

ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_synthesized
) AD_FUNCTION_BEGIN {
  total_synthesized = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  // 遍历所有 (type, field) 的写入操作
  typedef hash_map<array_detector::TypeFieldKey, array_detector::TypeFieldAnalysisData*, array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    array_detector::TypeFieldAnalysisData * write_ops = (*iter).second;

    if (!write_ops || !write_ops->writes) continue;

    // 为每个写入生成 SourceEscapeConclude（使用 source-escape-conclude 模块）
    for (unsigned i = 0; i < write_ops->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*write_ops->writes)[i];
      if (!wrapper) continue;

      // 调用 source-escape-conclude 模块生成结论
      if (!wrapper->escape_conclude && wrapper->uses) {
        AD_TRY (synthesizeSourceEscapeConclude (AD_ARGS, wrapper->uses, &wrapper->escape_conclude));
      }

      total_synthesized++;
    }

    // 生成 (type, field) 级别逃逸汇总
    FieldEscapeConclude * escape_conclude = ggc_alloc<FieldEscapeConclude> ();
    if (escape_conclude) {
      memset (escape_conclude, 0, sizeof (FieldEscapeConclude));

      // 设置标识
      escape_conclude->type = write_ops->type;
      escape_conclude->field_decl = write_ops->field_decl;

      // 汇总统计
      for (unsigned i = 0; i < write_ops->writes->length (); i++) {
        Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*write_ops->writes)[i];
        if (!wrapper) continue;

        escape_conclude->total_field_writes++;

        // 统计来源类型分布
        countSourceType (wrapper->write_source, escape_conclude);

        // 汇总逃逸
        if (wrapper->escape_conclude) {
          escape_conclude->total_escapes += wrapper->escape_conclude->total_escapes;
          escape_conclude->safe_debug_escapes += wrapper->escape_conclude->safe_debug_escapes;
          escape_conclude->rejecting_escapes += wrapper->escape_conclude->rejecting_escapes;

          if (wrapper->escape_conclude->total_escapes > 0) {
            escape_conclude->field_writes_with_escape++;
          }
          if (wrapper->escape_conclude->has_rejecting_evidence) {
            escape_conclude->field_writes_with_rejecting++;
          }
          if (!wrapper->escape_conclude->is_fully_analyzed) {
            escape_conclude->field_writes_without_analysis++;
          }
        } else {
          escape_conclude->field_writes_without_analysis++;
        }
      }

      escape_conclude->has_rejecting_evidence = (escape_conclude->field_writes_with_rejecting > 0);
      escape_conclude->rejection_ratio = (escape_conclude->total_field_writes > 0)
        ? (float)escape_conclude->field_writes_with_rejecting / (float)escape_conclude->total_field_writes
        : 0.0f;

      write_ops->escape_conclude = escape_conclude;
    }
  }

  AD_DEBUG_PRINT ("escapeSynth: %u evidence results", total_synthesized);
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
