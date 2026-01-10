// ============================================================================
// ad-source-escape-conclude 模块实现
// ============================================================================
// 数据流：(listof wrapper-source-use-info-source-escape-use-info) -> source-escape-conclude
// 对单个写入的逃逸证据进行统计和判定
// ============================================================================

#include "source-escape-conclude.hh"
#include "source-escape-use-info.hh"
#include "array-detector.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 调试逃逸识别
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

// ============================================================================
// 源逃逸结论生成
// ============================================================================
// 输入：uses (wrapper 列表，escape_use_info 已填充)
// 输出：SourceEscapeConclude*

ArrayDetectErrorCode synthesizeSourceEscapeConclude (
  AD_FUNC_ARGS,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses,
  SourceEscapeConclude** out_conclude
) AD_FUNCTION_BEGIN {
  if (!out_conclude) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果结构
  auto* conclude = ggc_alloc<SourceEscapeConclude> ();
  if (!conclude) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (conclude, 0, sizeof (SourceEscapeConclude));

  // 统计逃逸
  unsigned int total_escapes = 0;
  unsigned int safe_debug_escapes = 0;
  unsigned int rejecting_escapes = 0;

  if (uses) {
    for (unsigned int i = 0; i < uses->length (); i++) {
      Wrapper_SourceUseInfo_SourceEscapeUseInfo* uw = (*uses)[i];
      if (!uw) continue;

      // 检查 escape_use_info 是否存在（表示已逃逸）
      if (uw->escape_use_info) {
        total_escapes++;
        if (uw->escape_use_info->is_safe_debug) {
          safe_debug_escapes++;
        } else {
          rejecting_escapes++;
        }
      }
    }
  }

  // 填充结论
  conclude->total_escapes = total_escapes;
  conclude->safe_debug_escapes = safe_debug_escapes;
  conclude->rejecting_escapes = rejecting_escapes;
  conclude->has_rejecting_evidence = (rejecting_escapes > 0);
  conclude->is_fully_analyzed = true;

  *out_conclude = conclude;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeConclude (
  SourceEscapeConclude const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "  SourceEscapeConclude:\n");
  fprintf (output, "    total_escapes: %u\n", result->total_escapes);
  fprintf (output, "    safe_debug_escapes: %u\n", result->safe_debug_escapes);
  fprintf (output, "    rejecting_escapes: %u\n", result->rejecting_escapes);
  fprintf (output, "    has_rejecting_evidence: %s\n",
           result->has_rejecting_evidence ? "true" : "false");
}

// ============================================================================
// Pipeline 接口实现
// ============================================================================

// 为所有写入生成源级逃逸结论
ArrayDetectErrorCode synthesizeAllSourceEscapeConclude (
  AD_FUNC_ARGS,
  ::array_detector::ArrayDetector &detector,
  unsigned int &total_synthesized
) AD_FUNCTION_BEGIN {
  total_synthesized = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<::array_detector::TypeFieldKey,
                   ::array_detector::TypeFieldAnalysisData*,
                   ::array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    ::array_detector::TypeFieldAnalysisData * tfad = (*iter).second;
    if (!tfad || !tfad->writes) continue;

    for (unsigned i = 0; i < tfad->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*tfad->writes)[i];
      if (!wrapper) continue;

      // 仅当 escape_conclude 尚未填充且有 uses 时才生成
      if (!wrapper->escape_conclude && wrapper->uses) {
        AD_TRY (synthesizeSourceEscapeConclude (AD_ARGS, wrapper->uses, &wrapper->escape_conclude));
        total_synthesized++;
      }
    }
  }

  AD_DEBUG_PRINT ("sourceConclude: synthesized %u escape conclusions", total_synthesized);
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
