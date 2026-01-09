#include "escaped-use.hh"
#include "info-print.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸使用提取
// 输入：all_uses (所有使用点)
// 输出：直接写入 out_escape_uses 和 out_escape_count
// ============================================================================

ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  vec<field_analysis::FieldUsePoint>* all_uses,
  vec<field_analysis::FieldUsePoint const*>** out_escape_uses,
  unsigned int* out_escape_count,
  bool* out_has_escape
) AD_FUNCTION_BEGIN {
  if (!out_escape_uses || !out_escape_count || !out_has_escape) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化输出
  *out_escape_count = 0;
  *out_has_escape = false;

  // 分配逃逸向量
  *out_escape_uses = ggc_alloc<vec<field_analysis::FieldUsePoint const*>> ();
  if (!*out_escape_uses) {
    AD_RETURNE (MEMORY_ERROR);
  }
  (*out_escape_uses)->create (0);

  // 从 all_uses 中提取逃逸使用
  if (all_uses) {
    for (unsigned int i = 0; i < all_uses->length (); i++) {
      field_analysis::FieldUsePoint const &use = (*all_uses)[i];
      if (use.is_escape()) {
        (*out_escape_uses)->safe_push (&(*all_uses)[i]);
        (*out_escape_count)++;
        *out_has_escape = true;
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
