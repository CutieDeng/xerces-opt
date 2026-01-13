#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-wrapper.hh"
#include "transfer-collect.hh"

namespace array_detect_ns {

// ============================================================================
// 字段所有权转移统计模块 (Transfer Stats)
// ============================================================================
// 数据流位置：field, (listof write-original-source) -> transfer-stats
// 汇总单个 (type, field) 的所有写入操作的所有权转移信息
//
// (transfer-stats
//   type                    : tree
//   field-decl              : tree
//   total-field-writes      : nat
//   writes-from-field-access: nat       ; 来自字段访问的写入数
//   certain-transfers       : nat       ; 确定转移数
//   impossible-transfers    : nat       ; 不可能转移数
//   conditional-transfers   : nat)      ; 条件转移数
// ============================================================================

struct TransferStats {
  // === 标识 ===
  tree type;
  tree field_decl;

  // === 写入统计 ===
  unsigned int total_field_writes;              // 总写入操作数
  unsigned int writes_from_field_access;        // 来自字段访问的写入数（可分析所有权）
  unsigned int writes_analyzed;                 // 已分析所有权的写入数

  // === 转移结论统计 ===
  unsigned int certain_transfers;               // MOVE_CERTAIN 数量
  unsigned int impossible_transfers;            // MOVE_IMPOSSIBLE 数量
  unsigned int conditional_transfers;           // MOVE_CONDITIONAL 数量

  // === 核心判定 ===
  bool all_transfers_certain;                   // 所有转移都是确定的
  bool has_impossible_transfer;                 // 存在不可能转移
};

// ============================================================================
// 核心分析函数
// ============================================================================

// 汇总字段级所有权转移统计
// 输入：type, field_decl, writes (wrapper 列表，ownership_move 已填充)
// 输出：TransferStats*
ArrayDetectErrorCode summarizeTransferStats (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo*, va_gc>* writes,
  TransferStats** result
);

} // namespace array_detect_ns
