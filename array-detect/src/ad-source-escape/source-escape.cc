#include "source-escape.hh"
#include "info-print.hh"

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
  field_analysis::FieldUsePoint const * escape
) {
  if (!escape || !escape->is_escape()) {
    return false;
  }

  // 只有参数传递和外部调用可能是调试调用
  if (escape->escape_kind != field_analysis::FIELD_ESC_PARAMETER &&
      escape->escape_kind != field_analysis::FIELD_ESC_EXTERNAL_CALL) {
    return false;
  }

  // 字符串匹配仅在此处使用
  return isKnownSafeDebugFunction (AD_ARGS, escape->escape_target);
}

// ============================================================================
// 源逃逸结论生成
// 输入：escaped_uses (逃逸使用列表)
// 输出：直接写入 wrapper 成员地址
// ============================================================================

ArrayDetectErrorCode generateSourceEscapeConclude (
  AD_FUNC_ARGS,
  vec<field_analysis::FieldUsePoint const*>* escaped_uses,
  unsigned int* out_total_escapes,
  unsigned int* out_safe_debug_escapes,
  unsigned int* out_rejecting_escapes,
  bool* out_has_rejecting_evidence
) AD_FUNCTION_BEGIN {
  if (!out_total_escapes || !out_safe_debug_escapes ||
      !out_rejecting_escapes || !out_has_rejecting_evidence) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化输出
  *out_total_escapes = 0;
  *out_safe_debug_escapes = 0;
  *out_rejecting_escapes = 0;
  *out_has_rejecting_evidence = false;

  if (!escaped_uses) {
    AD_RETURNE (OK);
  }

  // 统计逃逸
  *out_total_escapes = escaped_uses->length ();

  // 遍历所有逃逸，区分调试逃逸和非调试逃逸
  for (unsigned int i = 0; i < escaped_uses->length (); i++) {
    field_analysis::FieldUsePoint const * escape = (*escaped_uses)[i];
    if (isSafeDebugEscape (AD_ARGS, escape)) {
      (*out_safe_debug_escapes)++;
    } else {
      (*out_rejecting_escapes)++;
    }
  }

  // 核心判定：存在非调试逃逸 => 拒绝 owned
  *out_has_rejecting_evidence = (*out_rejecting_escapes > 0);

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
