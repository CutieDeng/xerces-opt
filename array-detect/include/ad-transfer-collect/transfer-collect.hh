#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-write.hh"
#include "write-source.hh"

namespace array_detect_ns {

// ============================================================================
// 所有权转移收集模块 (Transfer Collect)
// ============================================================================
// 数据流位置：field-write-info, write-original-source -> transfer-info
// 分析字段复制（如 a.ptr = b.ptr）中源字段是否被销毁
//
// (transfer-info
//   source-field      : tree           ; 源字段 (b.ptr)
//   source-object     : tree           ; 源对象 (b)
//   transfer-stmt     : gimple*        ; 转移语句
//   verdict           : transfer-verdict ; CERTAIN/IMPOSSIBLE/CONDITIONAL
//   invalidation-points : (listof invalidation-point*))
// ============================================================================

// 销毁类型枚举
enum InvalidationKind {
  INVALIDATION_NONE = 0,           // 无销毁
  INVALIDATION_NULL_ASSIGN,        // 赋值为 NULL
  INVALIDATION_CLOBBER_FIELD,      // 字段 clobber
  INVALIDATION_CLOBBER_OBJECT,     // 对象 clobber
  INVALIDATION_OTHER_ASSIGN        // 其他赋值
};

// ============================================================================
// 销毁点信息
// ============================================================================

struct InvalidationPoint {
  gimple* stmt;                    // 销毁语句
  InvalidationKind kind;           // 销毁类型
  location_t location;             // 源码位置
  basic_block bb;                  // 所在基本块
  char const* description;         // 销毁描述（调试用）
};

// ============================================================================
// 转移结论枚举
// ============================================================================

enum TransferVerdict {
  TRANSFER_CERTAIN,                // 必然转移（所有路径都有销毁点）
  TRANSFER_IMPOSSIBLE,             // 不可能转移（无销毁点）
  TRANSFER_CONDITIONAL,            // 条件转移（部分路径有销毁点）
  TRANSFER_NOT_APPLICABLE          // 不适用（非字段访问源）
};

// ============================================================================
// 转移信息 (TransferInfo)
// ============================================================================

struct TransferInfo {
  // === 基本信息 ===
  tree source_field;                              // 源字段（b.ptr）
  tree source_object;                             // 源对象（b）
  tree source_field_decl;                         // 源字段声明
  char const* source_field_name;                  // 源字段名
  gimple* transfer_stmt;                          // 转移语句
  location_t transfer_location;                   // 转移位置

  // === 销毁点信息 ===
  vec<InvalidationPoint*, va_gc>* invalidation_points;

  // === 分析结论 ===
  TransferVerdict verdict;

  // === 路径统计 ===
  unsigned int paths_with_invalidation;
  unsigned int paths_without_invalidation;
  unsigned int total_exit_paths;

  // === 元数据 ===
  bool is_analyzed;
  char const* verdict_description;
};

// ============================================================================
// 核心分析函数
// ============================================================================

// 主入口：收集单个字段写入的转移信息
// 输入：write_info, write_source (当 write_source->source_type == SOURCE_FIELD_ACCESS)
// 输出：TransferInfo 指针
ArrayDetectErrorCode collectTransfer (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  ::array_detector::WriteOriginalSource* write_source,
  TransferInfo** out_transfer
);

// ============================================================================
// 调试输出
// ============================================================================

// 打印转移信息
void printTransferInfo (
  AD_FUNC_ARGS,
  FILE* out,
  TransferInfo* result
);

} // namespace array_detect_ns
