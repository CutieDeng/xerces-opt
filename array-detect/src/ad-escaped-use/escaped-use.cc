#include "escaped-use.hh"
#include "info-print.hh"

namespace array_detect_ns {

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
