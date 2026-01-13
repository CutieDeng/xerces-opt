// ============================================================================
// ad-transfer-stats 模块实现
// ============================================================================
// 汇总字段级所有权转移统计
// 数据流：field, (listof write-original-source) -> transfer-stats
// ============================================================================

#include "transfer-stats.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// summarizeTransferStats
// ----------------------------------------------------------------------------
// 汇总字段级所有权转移统计
// 输入：type, field_decl, writes (wrapper 列表，transfer_info 已填充)
// 输出：TransferStats*

ArrayDetectErrorCode summarizeTransferStats (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo*, va_gc>* writes,
  TransferStats** result
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果
  TransferStats* stats = ggc_alloc<TransferStats> ();
  if (!stats) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (stats, 0, sizeof (TransferStats));

  // 填充标识
  stats->type = type;
  stats->field_decl = field_decl;

  // 统计写入信息
  if (writes) {
    stats->total_field_writes = writes->length ();

    for (unsigned int i = 0; i < writes->length (); i++) {
      field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo* wrapper = (*writes)[i];
      if (!wrapper) continue;

      // 检查是否来自字段访问
      if (wrapper->write_source &&
          wrapper->write_source->source_type == SOURCE_FIELD_ACCESS) {
        stats->writes_from_field_access++;
      }

      // 检查所有权转移结果
      if (wrapper->transfer_info) {
        stats->writes_analyzed++;

        switch (wrapper->transfer_info->verdict) {
          case TRANSFER_CERTAIN:
            stats->certain_transfers++;
            break;
          case TRANSFER_IMPOSSIBLE:
            stats->impossible_transfers++;
            break;
          case TRANSFER_CONDITIONAL:
            stats->conditional_transfers++;
            break;
          case TRANSFER_NOT_APPLICABLE:
            // 不计入
            break;
        }
      }
    }
  }

  // 计算核心判定
  stats->all_transfers_certain =
    (stats->writes_analyzed > 0) &&
    (stats->certain_transfers == stats->writes_analyzed);
  stats->has_impossible_transfer = (stats->impossible_transfers > 0);

  *result = stats;
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
