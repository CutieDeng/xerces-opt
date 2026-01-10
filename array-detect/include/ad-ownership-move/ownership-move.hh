#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-write.hh"
#include "write-source.hh"

namespace array_detector {
  class ArrayDetector;
} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 所有权转移分析模块 (Ownership Move Analysis)
// ============================================================================
// 数据流位置：field-write-info, write-original-source -> ownership-move
// 分析字段复制（如 a.ptr = b.ptr）中源字段是否被销毁
//
// (ownership-move-result
//   source-field      : tree           ; 源字段 (b.ptr)
//   source-object     : tree           ; 源对象 (b)
//   transfer-stmt     : gimple*        ; 转移语句
//   verdict           : ownership-verdict ; CERTAIN/IMPOSSIBLE/CONDITIONAL
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
// 所有权转移结论枚举
// ============================================================================

enum OwnershipMoveVerdict {
  MOVE_CERTAIN,                    // 必然转移（所有路径都有销毁点）
  MOVE_IMPOSSIBLE,                 // 不可能转移（无销毁点）
  MOVE_CONDITIONAL,                // 条件转移（部分路径有销毁点）
  MOVE_NOT_APPLICABLE              // 不适用（非字段访问源）
};

// ============================================================================
// 所有权转移结果 (OwnershipMoveResult)
// ============================================================================

struct OwnershipMoveResult {
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
  OwnershipMoveVerdict verdict;

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

// 主入口：分析单个字段写入的所有权转移
// 输入：write_info, source_kind, field_access_data
// 输出：直接写入 out_move 指向的指针 (wrapper->move)
ArrayDetectErrorCode analyzeOwnershipMove (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  field_analysis::FieldSourceKind source_kind,
  field_analysis::FieldAccessData* field_access_data,
  field_analysis::FieldMoveAnalysis** out_move
);

// ============================================================================
// Pipeline 接口
// ============================================================================

// 分析所有字段写入操作的所有权转移
ArrayDetectErrorCode analyzeAllOwnershipMoves (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_certain_moves
);

// ============================================================================
// 调试输出
// ============================================================================

// 打印所有权转移结果
void printOwnershipMoveResult (
  AD_FUNC_ARGS,
  FILE* out,
  field_analysis::FieldMoveAnalysis* result
);

} // namespace array_detect_ns
