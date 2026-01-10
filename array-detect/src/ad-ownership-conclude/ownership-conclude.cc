// ============================================================================
// ad-ownership-conclude 模块实现
// ============================================================================
// 汇总字段级所有权结论
// 数据流：field, (listof write-original-source) -> ownership-conclude
// ============================================================================

#include "ownership-conclude.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// summarizeOwnershipConclude
// ----------------------------------------------------------------------------
// 汇总字段级所有权结论
// 输入：type, field_decl, writes (wrapper 列表，ownership_move 已填充)
// 输出：OwnershipConclude*

ArrayDetectErrorCode summarizeOwnershipConclude (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove*, va_gc>* writes,
  OwnershipConclude** result
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果
  OwnershipConclude* conclude = ggc_alloc<OwnershipConclude> ();
  if (!conclude) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (conclude, 0, sizeof (OwnershipConclude));

  // 填充标识
  conclude->type = type;
  conclude->field_decl = field_decl;

  // 统计写入信息
  if (writes) {
    conclude->total_field_writes = writes->length ();

    for (unsigned int i = 0; i < writes->length (); i++) {
      field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove* wrapper = (*writes)[i];
      if (!wrapper) continue;

      // 检查是否来自字段访问
      if (wrapper->write_source &&
          wrapper->write_source->source_type == SOURCE_FIELD_ACCESS) {
        conclude->writes_from_field_access++;
      }

      // 检查所有权转移结果
      if (wrapper->ownership_move) {
        conclude->writes_analyzed++;

        switch (wrapper->ownership_move->verdict) {
          case MOVE_CERTAIN:
            conclude->certain_transfers++;
            break;
          case MOVE_IMPOSSIBLE:
            conclude->impossible_transfers++;
            break;
          case MOVE_CONDITIONAL:
            conclude->conditional_transfers++;
            break;
          case MOVE_NOT_APPLICABLE:
            // 不计入
            break;
        }
      }
    }
  }

  // 计算核心判定
  conclude->all_transfers_certain =
    (conclude->writes_analyzed > 0) &&
    (conclude->certain_transfers == conclude->writes_analyzed);
  conclude->has_impossible_transfer = (conclude->impossible_transfers > 0);

  *result = conclude;
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
