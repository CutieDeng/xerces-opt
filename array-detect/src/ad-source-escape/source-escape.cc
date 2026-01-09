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
