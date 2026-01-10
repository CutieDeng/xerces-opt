#include "escaped-use.hh"
#include "info-print.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸使用提取
// 输入：(listof SourceUseInfo)
// 输出：EscapedUseResult 指针
// ============================================================================

ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  vec<SourceUseInfo>* all_uses,
  EscapedUseResult** out_result
) AD_FUNCTION_BEGIN {
  if (!out_result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果结构
  *out_result = ggc_alloc<EscapedUseResult> ();
  if (!*out_result) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // 初始化结果
  EscapedUseResult* result = *out_result;
  result->escape_count = 0;

  // 分配逃逸向量
  result->escapes = ggc_alloc<vec<SourceUseInfo const*>> ();
  if (!result->escapes) {
    AD_RETURNE (MEMORY_ERROR);
  }
  result->escapes->create (0);

  // 从 all_uses 中提取逃逸使用
  if (all_uses) {
    for (unsigned int i = 0; i < all_uses->length (); i++) {
      SourceUseInfo const &use = (*all_uses)[i];
      if (use.is_escape()) {
        result->escapes->safe_push (&(*all_uses)[i]);
        result->escape_count++;
      }
    }
  }

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

} // namespace array_detect_ns
