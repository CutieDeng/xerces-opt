// ============================================================================
// ad-driver 模块实现
// ============================================================================
// 细节驱动逻辑模块
// 实现单个写入的完整分析流程驱动
// ============================================================================

// 使用相对路径确保包含正确的头文件
#include "../../include/ad-driver/driver.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 辅助函数实现
// ============================================================================

void initDriverContext (WriteAnalysisDriverContext& ctx) {
  ctx.write_info = nullptr;
  ctx.source = nullptr;
  ctx.use_result = nullptr;
  ctx.escaped_uses = nullptr;
  ctx.escape_conclude = nullptr;
  ctx.move_result = nullptr;
  ctx.is_analyzed = false;
  ctx.has_error = false;
}

void cleanupDriverContext (WriteAnalysisDriverContext& ctx) {
  initDriverContext(ctx);
}

void printDriverContext (
  AD_FUNC_ARGS,
  FILE* out,
  WriteAnalysisDriverContext const& driver_ctx
) {
  (void)ctx; (void)gcc_ctx;
  if (!out) return;

  fprintf(out, "=== WriteAnalysisDriverContext ===\n");
  fprintf(out, "  is_analyzed: %s\n", driver_ctx.is_analyzed ? "true" : "false");
  fprintf(out, "  has_error: %s\n", driver_ctx.has_error ? "true" : "false");
  fprintf(out, "=== End WriteAnalysisDriverContext ===\n");
}

// ============================================================================
// 驱动函数实现
// ============================================================================

// 注意：当前实现是 stub，实际的分析逻辑在 pipeline 各阶段中
// 这些函数预留作为未来重构的接口

ArrayDetectErrorCode driveWriteAnalysis (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  FieldWriteAnalysisRecord*& result
) AD_FUNCTION_BEGIN {
  (void)write_info;
  result = nullptr;

  // TODO: 实现完整的单写入分析驱动
  // 当前逻辑分散在 pipeline 的各个步骤中
  // 未来可以将这些逻辑统一到此函数

  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode driveFieldAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree type,
  tree field_decl,
  vec<FieldWriteAnalysisRecord*>*& records
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)type;
  (void)field_decl;
  records = nullptr;

  // TODO: 实现从 detector 中获取指定字段的所有写入，并驱动分析

  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode unwrapAnalysis (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  WriteAnalysisDriverContext& driver_ctx
) AD_FUNCTION_BEGIN {
  initDriverContext(driver_ctx);

  if (!record) {
    driver_ctx.has_error = true;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // FieldWriteAnalysisRecord 是 FieldWriteAnalysisWrapper 的别名
  // Wrapper 是扁平化结构，包含了所有分析阶段的结果
  // 这里我们只需标记已分析

  driver_ctx.is_analyzed = true;
  driver_ctx.has_error = false;

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
