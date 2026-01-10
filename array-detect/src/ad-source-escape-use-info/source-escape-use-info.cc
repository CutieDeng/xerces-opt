// ============================================================================
// ad-source-escape-use-info 模块实现
// ============================================================================
// 提取逃逸使用信息
// 数据流：(listof wrapper-source-use-info-source-escape-use-info) -> 填充 escape_use_info 字段
// ============================================================================

#include "source-escape-use-info.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 辅助函数实现
// ============================================================================

// ----------------------------------------------------------------------------
// isEscapeSafeDebug
// ----------------------------------------------------------------------------
// 判断是否为安全调试逃逸（如 printf/fprintf 等）

bool isEscapeSafeDebug (
  AD_FUNC_ARGS,
  SourceUseInfo const* use_info
) {
  (void)ctx; (void)gcc_ctx;

  if (!use_info) return false;
  if (use_info->escape_kind == SU_ESCAPE_NONE) return false;

  // 检查是否传递给调试输出函数
  if (use_info->escape_target) {
    char const* target = use_info->escape_target;

    // 常见的调试输出函数
    if (strcmp (target, "printf") == 0 ||
        strcmp (target, "fprintf") == 0 ||
        strcmp (target, "vprintf") == 0 ||
        strcmp (target, "vfprintf") == 0 ||
        strcmp (target, "puts") == 0 ||
        strcmp (target, "fputs") == 0 ||
        strcmp (target, "fwrite") == 0 ||
        strcmp (target, "write") == 0 ||
        strcmp (target, "__builtin_printf") == 0) {
      return true;
    }

    // 检查是否包含 "debug"、"log"、"trace" 等关键词
    if (strstr (target, "debug") != NULL ||
        strstr (target, "Debug") != NULL ||
        strstr (target, "DEBUG") != NULL ||
        strstr (target, "log") != NULL ||
        strstr (target, "Log") != NULL ||
        strstr (target, "LOG") != NULL ||
        strstr (target, "trace") != NULL ||
        strstr (target, "Trace") != NULL ||
        strstr (target, "TRACE") != NULL ||
        strstr (target, "dump") != NULL ||
        strstr (target, "Dump") != NULL ||
        strstr (target, "DUMP") != NULL ||
        strstr (target, "print") != NULL ||
        strstr (target, "Print") != NULL ||
        strstr (target, "PRINT") != NULL) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// 逃逸使用提取
// ============================================================================
// 输入：wrapper 列表（已填充 use_info）
// 输出：填充每个 wrapper 的 escape_use_info 字段

ArrayDetectErrorCode extractSourceEscapeUseInfo (
  AD_FUNC_ARGS,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses
) AD_FUNCTION_BEGIN {
  if (!uses) {
    AD_RETURNE (OK);  // 空列表，无需处理
  }

  for (unsigned int i = 0; i < uses->length (); i++) {
    Wrapper_SourceUseInfo_SourceEscapeUseInfo* wrapper = (*uses)[i];
    if (!wrapper || !wrapper->use_info) continue;

    SourceUseInfo const* use_info = wrapper->use_info;

    // 检查是否为逃逸
    if (use_info->escape_kind == SU_ESCAPE_NONE) {
      wrapper->escape_use_info = NULL;
      continue;
    }

    // 创建 SourceEscapeUseInfo
    auto* escape_use_info = ggc_alloc<SourceEscapeUseInfo> ();
    if (!escape_use_info) {
      AD_RETURNE (MEMORY_ERROR);
    }

    // 填充逃逸信息
    escape_use_info->escape_kind = use_info->escape_kind;
    escape_use_info->escape_target = use_info->escape_target;
    escape_use_info->target_decl = use_info->target_info.function_decl;
    escape_use_info->is_safe_debug = isEscapeSafeDebug (AD_ARGS, use_info);

    wrapper->escape_use_info = escape_use_info;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeUseInfo (
  SourceEscapeUseInfo const* info,
  FILE* output
) {
  if (!info || !output) return;

  fprintf (output, "  SourceEscapeUseInfo:\n");
  fprintf (output, "    escape_kind: %s\n", getEscapeKindString (info->escape_kind));
  fprintf (output, "    escape_target: %s\n",
           info->escape_target ? info->escape_target : "<none>");
  fprintf (output, "    target_decl: %p\n", (void*)info->target_decl);
  fprintf (output, "    is_safe_debug: %s\n", info->is_safe_debug ? "true" : "false");
}

} // namespace array_detect_ns
