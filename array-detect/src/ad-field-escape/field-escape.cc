#include "field-escape.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "gcc-ext-util.hh"
#include "write-source.hh"

namespace array_detect_ns {

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// synthesizeAllFieldEscapes_isFieldUsePointSafeDebugEscape
// ----------------------------------------------------------------------------
// 判断 FieldUsePoint 是否为安全调试逃逸

bool synthesizeAllFieldEscapes_isFieldUsePointSafeDebugEscape (
  AD_FUNC_ARGS,
  field_analysis::FieldUsePoint const * use_point
) {
  if (!use_point || use_point->escape_kind == field_analysis::FIELD_ESC_NONE) {
    return false;
  }

  // 只有参数传递和外部调用可能是调试调用
  if (use_point->escape_kind != field_analysis::FIELD_ESC_PARAMETER &&
      use_point->escape_kind != field_analysis::FIELD_ESC_EXTERNAL_CALL) {
    return false;
  }

  return isKnownSafeDebugFunction (AD_ARGS, use_point->escape_target);
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
  array_detector::TypeFieldAnalysisData * field_data,
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
      array_detector::Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude * wrapper =
        (*field_data->writes)[i];
      if (!wrapper) continue;

      summary->total_field_writes++;

      // 统计来源类型分布（从 wrapper 的 FieldWriteSource 部分读取）
      switch (wrapper->source_kind) {
        case field_analysis::FIELD_SRC_FUNCTION_CALL:
          summary->source_function_call++;
          break;
        case field_analysis::FIELD_SRC_FIELD_ACCESS:
          summary->source_field_access++;
          break;
        case field_analysis::FIELD_SRC_CONSTANT:
          summary->source_constant++;
          break;
        case field_analysis::FIELD_SRC_COMPUTATION:
          summary->source_computation++;
          break;
        case field_analysis::FIELD_SRC_PHI:
          summary->source_phi++;
          break;
        case field_analysis::FIELD_SRC_UNKNOWN:
        default:
          summary->source_unknown++;
          break;
      }

      // 统计逃逸（从 wrapper 的 FieldEscapeConclude 部分读取）
      summary->total_escapes += wrapper->total_escapes;
      summary->safe_debug_escapes += wrapper->safe_debug_escapes;
      summary->rejecting_escapes += wrapper->rejecting_escapes;

      if (wrapper->total_escapes > 0) {
        summary->field_writes_with_escape++;
      }
      if (wrapper->has_rejecting_evidence) {
        summary->field_writes_with_rejecting++;
      }

      // 检查是否完全分析
      if (!wrapper->is_fully_analyzed) {
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
      array_detector::Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude * wrapper = (*write_ops->writes)[i];
      if (!wrapper) continue;

      // 从 wrapper 的 FieldUseAnalysis 部分读取数据
      if (!wrapper->is_fully_analyzed && wrapper->total_use_count == 0) {
        // 未分析的写入操作
        continue;
      }

      // 直接在 wrapper 上计算逃逸结论
      // 遍历 escape_uses 统计调试逃逸和拒绝逃逸
      unsigned int safe_debug_count = 0;
      unsigned int rejecting_count = 0;

      if (wrapper->escape_uses) {
        for (unsigned j = 0; j < wrapper->escape_uses->length (); j++) {
          field_analysis::FieldUsePoint const * escape_use = (*wrapper->escape_uses)[j];
          if (synthesizeAllFieldEscapes_isFieldUsePointSafeDebugEscape (AD_ARGS, escape_use)) {
            safe_debug_count++;
          } else {
            rejecting_count++;
          }
        }
      }

      // 填充 wrapper 的 FieldEscapeConclude 部分
      wrapper->total_escapes = wrapper->escape_count;
      wrapper->safe_debug_escapes = safe_debug_count;
      wrapper->rejecting_escapes = rejecting_count;
      wrapper->has_rejecting_evidence = (rejecting_count > 0);

      total_synthesized++;

      if (wrapper->has_rejecting_evidence) {
        field_has_rejecting = true;
      }
    }

    // 更新字段级别拒绝标志
    write_ops->has_rejecting_evidence = field_has_rejecting;

    // 生成 (type, field) 级别汇总并存储到 conclude
    field_analysis::FieldConclude * conclude = ggc_alloc<field_analysis::FieldConclude> ();
    if (conclude) {
      memset (conclude, 0, sizeof (field_analysis::FieldConclude));

      // 汇总统计
      if (write_ops->writes) {
        for (unsigned i = 0; i < write_ops->writes->length (); i++) {
          array_detector::Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude * wrapper = (*write_ops->writes)[i];
          if (!wrapper) continue;

          conclude->total_writes++;

          // 统计来源类型分布
          switch (wrapper->source_kind) {
            case field_analysis::FIELD_SRC_FUNCTION_CALL:
              conclude->src_function_call++;
              break;
            case field_analysis::FIELD_SRC_FIELD_ACCESS:
              conclude->src_field_access++;
              break;
            case field_analysis::FIELD_SRC_CONSTANT:
              conclude->src_constant++;
              break;
            case field_analysis::FIELD_SRC_COMPUTATION:
              conclude->src_computation++;
              break;
            case field_analysis::FIELD_SRC_PHI:
              conclude->src_phi++;
              break;
            default:
              conclude->src_unknown++;
              break;
          }

          // 汇总逃逸
          conclude->total_escapes += wrapper->total_escapes;
          conclude->safe_escapes += wrapper->safe_debug_escapes;
          conclude->rejecting_escapes += wrapper->rejecting_escapes;

          if (wrapper->has_escape) {
            conclude->writes_with_escape++;
          }
          if (wrapper->has_rejecting_evidence) {
            conclude->writes_with_rejecting++;
          }
          if (!wrapper->is_fully_analyzed) {
            conclude->writes_without_analysis++;
          }
        }
      }

      conclude->has_rejecting = field_has_rejecting;
      conclude->rejection_ratio = (conclude->total_writes > 0)
        ? (float)conclude->writes_with_rejecting / (float)conclude->total_writes
        : 0.0f;

      write_ops->conclude = conclude;
    }
  }

  AD_DEBUG_PRINT ("escapeSynth: %u evidence results", total_synthesized);
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
