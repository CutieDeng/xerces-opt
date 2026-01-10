#include "field-escape.hh"
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

  // 分配证据引用向量
  summary->all_source_concludes = ggc_alloc<vec<SourceEscapeConclude*>> ();
  summary->all_source_concludes->create (0);

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
// 组合接口：综合所有字段的逃逸信息

ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<SourceEscapeConclude*> * &evidence_results,
  unsigned int &total_synthesized
) AD_FUNCTION_BEGIN {
  total_synthesized = 0;

  if (!detector.m_type_field_writes) {
    evidence_results = NULL;
    AD_RETURNE (OK);
  }

  // 分配结果向量
  evidence_results = ggc_alloc<vec<SourceEscapeConclude*>> ();
  evidence_results->create (0);

  // 遍历所有 (type, field) 的写入操作
  typedef hash_map<array_detector::TypeFieldKey, array_detector::TypeFieldAnalysisData*, array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    array_detector::TypeFieldKey const &key = (*iter).first;
    array_detector::TypeFieldAnalysisData * write_ops = (*iter).second;

    (void)key;
    if (!write_ops || !write_ops->writes) continue;

    // 字段级别是否存在拒绝证据
    bool field_has_rejecting = false;

    // 遍历该 (type, field) 的所有写入分析 Wrapper
    for (unsigned i = 0; i < write_ops->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*write_ops->writes)[i];
      if (!wrapper) continue;

      // 从 uses 遍历并统计逃逸
      unsigned int safe_debug_count = 0;
      unsigned int rejecting_count = 0;
      unsigned int total_escape_count = 0;
      bool is_fully_analyzed = true;

      if (wrapper->uses) {
        for (unsigned j = 0; j < wrapper->uses->length (); j++) {
          Wrapper_SourceUseInfo_SourceEscapeUseInfo * use_wrapper = (*wrapper->uses)[j];
          if (!use_wrapper || !use_wrapper->use_info) continue;

          // 检查是否逃逸
          if (use_wrapper->escape_use_info) {
            total_escape_count++;
            if (use_wrapper->escape_use_info->is_safe_debug) {
              safe_debug_count++;
            } else {
              rejecting_count++;
            }
          }
        }
      }

      // 创建并填充 SourceEscapeConclude
      if (!wrapper->escape_conclude) {
        wrapper->escape_conclude = ggc_alloc<SourceEscapeConclude> ();
        if (wrapper->escape_conclude) {
          memset (wrapper->escape_conclude, 0, sizeof (SourceEscapeConclude));
        }
      }

      if (wrapper->escape_conclude) {
        wrapper->escape_conclude->total_escapes = total_escape_count;
        wrapper->escape_conclude->safe_debug_escapes = safe_debug_count;
        wrapper->escape_conclude->rejecting_escapes = rejecting_count;
        wrapper->escape_conclude->has_rejecting_evidence = (rejecting_count > 0);
        wrapper->escape_conclude->is_fully_analyzed = is_fully_analyzed;
      }

      total_synthesized++;

      if (rejecting_count > 0) {
        field_has_rejecting = true;
      }
    }

    // 生成 (type, field) 级别逃逸汇总并存储到 escape_conclude
    FieldEscapeConclude * escape_conclude = ggc_alloc<FieldEscapeConclude> ();
    if (escape_conclude) {
      memset (escape_conclude, 0, sizeof (FieldEscapeConclude));

      // 设置标识
      escape_conclude->type = write_ops->type;
      escape_conclude->field_decl = write_ops->field_decl;

      // 汇总统计
      if (write_ops->writes) {
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
          }
        }
      }

      escape_conclude->has_rejecting_evidence = field_has_rejecting;
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
