#include "escape-synthesizer.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "gcc-ext-util.hh"
#include "field-source-variant.hh"

namespace array_detect_ns {

// ============================================================================
// 调试逃逸识别（唯一允许字符串匹配的场景）
// ============================================================================

bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!function_name) {
    return false;
  }

  // 已知安全调试函数列表
  static char const * safe_debug_functions[] = {
    "printf",
    "fprintf",
    "sprintf",
    "snprintf",
    "vprintf",
    "vfprintf",
    "vsprintf",
    "vsnprintf",
    "puts",
    "fputs",
    "putchar",
    "fputc",
    "perror",
    "std::cout",
    "std::cerr",
    "std::clog",
    NULL
  };

  for (int i = 0; safe_debug_functions[i] != NULL; i++) {
    if (strcmp (function_name, safe_debug_functions[i]) == 0) {
      return true;
    }
    // 也检查包含该名称的情况（如 __printf_chk）
    if (strstr (function_name, safe_debug_functions[i]) != NULL) {
      return true;
    }
  }

  return false;
}

// 判断逃逸是否为安全调试逃逸
bool isSafeDebugEscape (
  AD_FUNC_ARGS,
  SourceUseInfo const * escape
) {
  if (!escape || !escape->is_escape()) {
    return false;
  }

  // 只有参数传递和外部调用可能是调试调用
  if (escape->escape_kind != SU_ESCAPE_PARAMETER &&
      escape->escape_kind != SU_ESCAPE_EXTERNAL_CALL) {
    return false;
  }

  // 字符串匹配仅在此处使用
  return isKnownSafeDebugFunction (AD_ARGS, escape->escape_target);
}

// ============================================================================
// 逃逸使用提取：SourceUseResult -> EscapedUseResult
// ============================================================================

ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  SourceUseResult * use_result,
  EscapedUseResult * &result
) AD_FUNCTION_BEGIN {
  // 向后兼容：使用旧变量名
  SourceUseResult * raw_result = use_result;
  if (!raw_result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果结构
  EscapeExtractionResult * extraction = ggc_alloc<EscapeExtractionResult> ();
  if (!extraction) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (extraction, 0, sizeof (EscapeExtractionResult));

  // 复制源信息
  extraction->source_operand = raw_result->source_operand;
  extraction->source_stmt = raw_result->source_stmt;
  extraction->original_write_info = raw_result->original_write_info;

  // 分配逃逸向量
  extraction->escapes = ggc_alloc<vec<SourceUseInfo const *>> ();
  extraction->escapes->create (0);
  extraction->escape_count = 0;

  // 从 all_uses 中提取逃逸使用
  if (raw_result->all_uses) {
    for (unsigned int i = 0; i < raw_result->all_uses->length (); i++) {
      SourceUseInfo const &use = (*raw_result->all_uses)[i];
      if (use.is_escape()) {
        extraction->escapes->safe_push (&use);
        extraction->escape_count++;
      }
    }
  }

  AD_RETURNO (extraction);
} AD_FUNCTION_END

// ============================================================================
// 源逃逸结论生成：EscapedUseResult -> SourceEscapeConclude
// ============================================================================

ArrayDetectErrorCode generateSourceEscapeConclude (
  AD_FUNC_ARGS,
  EscapedUseResult * escaped_uses,
  tree type,
  tree field_decl,
  location_t write_location,
  SourceEscapeConclude * &result
) AD_FUNCTION_BEGIN {
  // 向后兼容：使用旧变量名
  EscapedUseResult * extraction = escaped_uses;
  if (!extraction) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果结构
  EscapeEvidenceResult * evidence = ggc_alloc<EscapeEvidenceResult> ();
  if (!evidence) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (evidence, 0, sizeof (EscapeEvidenceResult));

  // 设置写入位置标识
  evidence->type = type;
  evidence->field_decl = field_decl;
  evidence->write_location = write_location;
  evidence->original_write_info = extraction->original_write_info;

  // 统计逃逸
  evidence->total_escapes = extraction->escape_count;
  evidence->safe_debug_escapes = 0;
  evidence->rejecting_escapes = 0;

  // 遍历所有逃逸，区分调试逃逸和非调试逃逸
  if (extraction->escapes) {
    for (unsigned int i = 0; i < extraction->escapes->length (); i++) {
      SourceUseInfo const * escape = (*extraction->escapes)[i];
      if (isSafeDebugEscape (AD_ARGS, escape)) {
        evidence->safe_debug_escapes++;
      } else {
        evidence->rejecting_escapes++;
      }
    }
  }

  // 核心判定：存在非调试逃逸 => 拒绝 owned
  evidence->has_rejecting_evidence = (evidence->rejecting_escapes > 0);

  AD_RETURNO (evidence);
} AD_FUNCTION_END

// ============================================================================
// 字段逃逸结论汇总：TypeFieldAnalysisData -> FieldEscapeConclude
// ============================================================================

ArrayDetectErrorCode summarizeFieldEscape (
  AD_FUNC_ARGS,
  array_detector::TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
) AD_FUNCTION_BEGIN {
  if (!field_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配汇总结构
  TypeFieldEscapeSummary * summary = ggc_alloc<TypeFieldEscapeSummary> ();
  if (!summary) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (summary, 0, sizeof (TypeFieldEscapeSummary));

  // 设置标识
  summary->type = field_data->type;
  summary->field_decl = field_data->field_decl;

  // 分配证据引用向量
  summary->all_source_concludes = ggc_alloc<vec<SourceEscapeConclude*>> ();
  summary->all_source_concludes->create (0);

  // 遍历所有字段写入分析 Wrapper
  if (field_data->writes) {
    for (unsigned int i = 0; i < field_data->writes->length (); i++) {
      array_detector::FieldWriteAnalysisWrapper * wrapper =
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

// ============================================================================
// 辅助函数：判断 FieldUsePoint 是否为安全调试逃逸
// ============================================================================

static bool isFieldUsePointSafeDebugEscape (
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

// ============================================================================
// 组合接口：综合所有字段的逃逸信息
// ============================================================================

ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<EscapeEvidenceResult*> * &evidence_results,
  unsigned int &total_synthesized
) AD_FUNCTION_BEGIN {
  total_synthesized = 0;

  if (!detector.m_type_field_writes) {
    evidence_results = NULL;
    AD_RETURNE (OK);
  }

  // 分配结果向量
  evidence_results = ggc_alloc<vec<EscapeEvidenceResult*>> ();
  evidence_results->create (0);

  // 遍历所有 (type, field) 的写入操作
  typedef hash_map<array_detector::TypeFieldKey, array_detector::TypeFieldWriteOps*, array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    array_detector::TypeFieldKey const &key = (*iter).first;
    array_detector::TypeFieldWriteOps * write_ops = (*iter).second;

    if (!write_ops || !write_ops->writes) continue;

    // 字段级别是否存在拒绝证据
    bool field_has_rejecting = false;

    // 遍历该 (type, field) 的所有写入分析 Wrapper
    for (unsigned i = 0; i < write_ops->writes->length (); i++) {
      array_detector::FieldWriteAnalysisWrapper * wrapper = (*write_ops->writes)[i];
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
          if (isFieldUsePointSafeDebugEscape (AD_ARGS, escape_use)) {
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
          array_detector::FieldWriteAnalysisWrapper * wrapper = (*write_ops->writes)[i];
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

// ============================================================================
// 调试输出
// ============================================================================

void printEscapedUseResult (
  EscapedUseResult const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "\n");
  fprintf (output, "=== Escaped Use Result ===\n");
  fprintf (output, "Escape count: %u\n", result->escape_count);

  if (result->escapes) {
    for (unsigned int i = 0; i < result->escapes->length (); i++) {
      SourceUseInfo const * escape = (*result->escapes)[i];
      if (!escape) continue;
      fprintf (output, "  [%u] kind=%s, target=%s\n",
               i, getEscapeKindString (escape->escape_kind),
               escape->escape_target ? escape->escape_target : "<none>");
    }
  }

  fprintf (output, "==========================\n");
}

void printSourceEscapeConclude (
  SourceEscapeConclude const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "\n");
  fprintf (output, "=== Source Escape Conclude ===\n");
  fprintf (output, "Write location: %s:%d\n",
           LOCATION_FILE (result->write_location),
           LOCATION_LINE (result->write_location));
  fprintf (output, "Total escapes: %u\n", result->total_escapes);
  fprintf (output, "Safe debug escapes: %u\n", result->safe_debug_escapes);
  fprintf (output, "Rejecting escapes: %u\n", result->rejecting_escapes);
  fprintf (output, "Has rejecting evidence: %s\n",
           result->has_rejecting_evidence ? "YES" : "NO");
  fprintf (output, "==============================\n");
}

} // namespace array_detect_ns
