#pragma once

// ============================================================================
// ad-array-write-bound 模块
// ============================================================================
// 分析数组写入访问的边界条件，寻找关联整数字段
//
// 数据流：
//   array-write-access -> (listof write-bound-condition)  ; 一对多
//   pointer-field, write-bound-condition -> (or write-capacity-evidence #f)
//
// 场景：分析 if (i < arr->cap) arr->data[i] = x 形式的边界检查
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-detect-context-gcc.hh"
#include "array-write-collect.hh"

namespace array_detect_ns {

// ============================================================================
// 写边界条件
// ============================================================================
// (write-bound-condition
//   access           : array-write-access     ; 原访问
//   condition_stmt   : gimple                 ; 条件语句
//   comparison_code  : tree_code              ; 比较操作码
//   index_operand    : tree                   ; 索引操作数
//   bound_operand    : tree                   ; 边界操作数
//   has_field_bound  : bool                   ; 边界是否为字段
//   bound_field      : tree                   ; 边界字段
//   bound_type       : tree                   ; 边界所属类型
//   condition_bb     : basic_block            ; 条件所在块
//   dominates_access : bool                   ; 是否支配访问
//   location         : location_t)            ; 位置信息

struct WriteBoundCondition {
  // === 原访问 ===
  ArrayWriteAccess* access;

  // === 条件信息 ===
  gimple* condition_stmt;           // 条件语句 (GIMPLE_COND)
  enum tree_code comparison_code;   // 比较操作码 (LT_EXPR, LE_EXPR, etc.)
  tree index_operand;               // 索引操作数
  tree bound_operand;               // 边界操作数

  // === 字段关联 ===
  bool has_field_bound;             // 边界是否为字段引用
  tree bound_field_decl;            // 边界字段 FIELD_DECL
  tree bound_type;                  // 边界所属类型

  // === 控制流信息 ===
  basic_block condition_bb;         // 条件所在基本块
  bool dominates_access;            // 条件是否支配访问

  // === 位置信息 ===
  location_t location;
};

// ============================================================================
// 写容量证据置信度
// ============================================================================

enum WriteEvidenceConfidence {
  WRITE_CONF_CERTAIN,    // 确定：直接的边界检查
  WRITE_CONF_PROBABLE,   // 可能：间接的边界检查
  WRITE_CONF_WEAK        // 弱：推测性关联
};

// ============================================================================
// 写容量证据
// ============================================================================
// (write-capacity-evidence
//   pointer_field    : tree                    ; 指针字段
//   integer_field    : tree                    ; 关联的整数字段
//   containing_type  : tree                    ; 所属类型
//   write_access     : array-write-access      ; 写访问
//   bound_condition  : write-bound-condition   ; 边界条件
//   confidence       : (or 'certain 'probable 'weak)
//   location         : location_t
//   description      : string)

struct WriteCapacityEvidence {
  // === 关联信息 ===
  tree pointer_field;               // 指针字段 (FIELD_DECL)
  tree integer_field;               // 关联的整数字段 (FIELD_DECL)
  tree containing_type;             // 所属类型

  // === 来源信息 ===
  ArrayWriteAccess* write_access;   // 写访问
  WriteBoundCondition* bound_condition; // 边界条件

  // === 置信度 ===
  WriteEvidenceConfidence confidence;

  // === 位置信息 ===
  location_t location;
  char const* description;
};

// ============================================================================
// 函数声明
// ============================================================================

// 查找支配该访问的条件语句
// 输入：access
// 输出：(listof gimple*) - 支配该访问的 GIMPLE_COND 语句
ArrayDetectErrorCode findWriteDominatingConditions (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  vec<gimple*, va_gc>** result
);

// 分析数组写入的所有边界条件（一对多）
// 输入：access
// 输出：(listof write-bound-condition*)
ArrayDetectErrorCode analyzeWriteBoundConditions (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  vec<WriteBoundCondition*, va_gc>** results
);

// 提取写容量证据
// 输入：pointer_field, bound_cond
// 输出：write-capacity-evidence* (如果有效)
ArrayDetectErrorCode extractWriteCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  WriteBoundCondition* bound_cond,
  WriteCapacityEvidence** result
);

// 获取置信度名称
char const* writeConfidenceToString (WriteEvidenceConfidence conf);

// 打印写边界条件
void printWriteBoundCondition (
  AD_FUNC_ARGS,
  FILE* out,
  WriteBoundCondition* bound_cond
);

// 打印写容量证据
void printWriteCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  WriteCapacityEvidence* evidence
);

// 打印所有写容量证据
void printAllWriteCapacityEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<WriteCapacityEvidence*, va_gc>* evidences
);

} // namespace array_detect_ns
