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
// 第一层模块：逃逸提取
// ============================================================================

ArrayDetectErrorCode extractEscapes (
  AD_FUNC_ARGS,
  SourceUseAnalysisResult * raw_result,
  EscapeExtractionResult * &result
) AD_FUNCTION_BEGIN {
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
// 第二层模块：逃逸证据生成
// ============================================================================

ArrayDetectErrorCode generateEscapeEvidence (
  AD_FUNC_ARGS,
  EscapeExtractionResult * extraction,
  tree type,
  tree field_decl,
  location_t write_location,
  EscapeEvidenceResult * &result
) AD_FUNCTION_BEGIN {
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
// 第三层模块：(type, field) 级别汇总
// ============================================================================

ArrayDetectErrorCode summarizeTypeFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::TypeFieldAnalysisData * field_data,
  TypeFieldEscapeSummary * &result
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
  summary->all_evidences = ggc_alloc<vec<EscapeEvidenceResult*>> ();
  summary->all_evidences->create (0);

  // 遍历所有写入操作记录
  if (field_data->write_analysis_records) {
    for (unsigned int i = 0; i < field_data->write_analysis_records->length (); i++) {
      array_detector::FieldWriteAnalysisRecord * record =
        (*field_data->write_analysis_records)[i];
      if (!record) continue;

      summary->total_writes++;

      // 统计来源类型分布
      if (record->source_info) {
        switch (record->source_info->source_type) {
          case array_detector::SOURCE_FUNCTION_CALL:
            summary->source_function_call++;
            break;
          case array_detector::SOURCE_FIELD_ACCESS:
            summary->source_field_access++;
            break;
          case array_detector::SOURCE_CONSTANT:
            summary->source_constant++;
            break;
          case array_detector::SOURCE_COMPUTATION:
            summary->source_computation++;
            break;
          case array_detector::SOURCE_PHI:
            summary->source_phi++;
            break;
          case array_detector::SOURCE_UNKNOWN:
          default:
            summary->source_unknown++;
            break;
        }
      } else {
        summary->writes_without_analysis++;
        summary->source_unknown++;
      }

      // 统计逃逸
      if (record->escape_evidence) {
        EscapeEvidenceResult * evidence = record->escape_evidence;

        summary->total_escapes += evidence->total_escapes;
        summary->safe_debug_escapes += evidence->safe_debug_escapes;
        summary->rejecting_escapes += evidence->rejecting_escapes;

        if (evidence->total_escapes > 0) {
          summary->writes_with_escape++;
        }
        if (evidence->has_rejecting_evidence) {
          summary->writes_with_rejecting++;
        }

        // 添加到证据引用列表
        summary->all_evidences->safe_push (evidence);
      } else {
        summary->writes_without_analysis++;
      }
    }
  }

  // 计算核心判定
  summary->has_rejecting_evidence = (summary->writes_with_rejecting > 0);
  summary->rejection_ratio = (summary->total_writes > 0)
    ? (float)summary->writes_with_rejecting / (float)summary->total_writes
    : 0.0f;

  AD_RETURNO (summary);
} AD_FUNCTION_END

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

    if (!write_ops || !write_ops->write_analysis_records) continue;

    // 字段级别是否存在拒绝证据
    bool field_has_rejecting = false;

    // 遍历该 (type, field) 的所有写入操作
    for (unsigned i = 0; i < write_ops->write_analysis_records->length (); i++) {
      array_detector::FieldWriteAnalysisRecord * record = (*write_ops->write_analysis_records)[i];
      if (!record || !record->write_capture) continue;

      // 从 record->escape_analysis 读取逃逸分析结果
      SourceUseAnalysisResult * raw_result = record->escape_analysis;
      if (!raw_result) {
        continue;
      }

      // 获取写入位置
      location_t write_loc = gimple_location (raw_result->source_stmt);

      // 第一层：提取逃逸
      EscapeExtractionResult * extraction = NULL;
      AD_TRY (extractEscapes (AD_ARGS, raw_result, extraction));

      if (!extraction) {
        continue;
      }

      // 第二层：生成证据
      EscapeEvidenceResult * evidence = NULL;
      AD_TRY (generateEscapeEvidence (AD_ARGS, extraction, key.type, key.field_decl, write_loc, evidence));

      if (evidence) {
        record->escape_evidence = evidence;
        evidence_results->safe_push (evidence);
        total_synthesized++;

        if (evidence->has_rejecting_evidence) {
          field_has_rejecting = true;
        }
      }
    }

    // 更新字段级别拒绝标志
    write_ops->has_rejecting_evidence = field_has_rejecting;

    // 第三层：生成 (type, field) 级别汇总
    TypeFieldEscapeSummary * summary = NULL;
    AD_TRY (summarizeTypeFieldEscapes (AD_ARGS, write_ops, summary));
    write_ops->escape_summary = summary;
  }

  AD_DEBUG_PRINT ("escapeSynth: %u evidence results", total_synthesized);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printEscapeExtractionResult (
  EscapeExtractionResult const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "\n");
  fprintf (output, "=== Escape Extraction Result ===\n");
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

  fprintf (output, "================================\n");
}

void printEscapeEvidenceResult (
  EscapeEvidenceResult const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "\n");
  fprintf (output, "=== Escape Evidence Result ===\n");
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
