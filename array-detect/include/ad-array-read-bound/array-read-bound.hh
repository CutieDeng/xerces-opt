#pragma once

// ============================================================================
// ad-array-read-bound 模块
// ============================================================================
// 分析数组读取访问的边界条件，寻找关联整数字段
//
// 数据流：
//   array-read-access -> (listof read-bound-condition)  ; 一对多
//   pointer-field, read-bound-condition -> (or read-capacity-evidence #f)
//
// 场景：分析 if (i < arr->size && i >= 0) x = arr->data[i] 形式的边界检查
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-detect-context-gcc.hh"
#include "array-read-collect.hh"

namespace array_detect_ns {

// ============================================================================
// 读边界条件
// ============================================================================
// (read-bound-condition
//   access           : array-read-access      ; 原访问
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

struct ReadBoundCondition {
  // === 原访问 ===
  ArrayReadAccess* access;

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
// 读容量证据置信度
// ============================================================================

enum ReadEvidenceConfidence {
  READ_CONF_CERTAIN,    // 确定：直接的边界检查
  READ_CONF_PROBABLE,   // 可能：间接的边界检查
  READ_CONF_WEAK        // 弱：推测性关联
};

// ============================================================================
// 读容量证据
// ============================================================================
// (read-capacity-evidence
//   pointer_field    : tree                   ; 指针字段
//   integer_field    : tree                   ; 关联的整数字段
//   containing_type  : tree                   ; 所属类型
//   read_access      : array-read-access      ; 读访问
//   bound_condition  : read-bound-condition   ; 边界条件
//   confidence       : (or 'certain 'probable 'weak)
//   location         : location_t
//   description      : string)

struct ReadCapacityEvidence {
  // === 关联信息 ===
  tree pointer_field;               // 指针字段 (FIELD_DECL)
  tree integer_field;               // 关联的整数字段 (FIELD_DECL)
  tree containing_type;             // 所属类型

  // === 来源信息 ===
  ArrayReadAccess* read_access;     // 读访问
  ReadBoundCondition* bound_condition; // 边界条件

  // === 置信度 ===
  ReadEvidenceConfidence confidence;

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
ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<gimple*, va_gc>** result
);

// 分析数组读取的所有边界条件（一对多）
// 输入：access
// 输出：(listof read-bound-condition*)
ArrayDetectErrorCode analyzeReadBoundConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<ReadBoundCondition*, va_gc>** results
);

// 提取读容量证据
// 输入：pointer_field, bound_cond
// 输出：read-capacity-evidence* (如果有效)
ArrayDetectErrorCode extractReadCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  ReadBoundCondition* bound_cond,
  ReadCapacityEvidence** result
);

// 获取置信度名称
char const* readConfidenceToString (ReadEvidenceConfidence conf);

// 打印读边界条件
void printReadBoundCondition (
  AD_FUNC_ARGS,
  FILE* out,
  ReadBoundCondition* bound_cond
);

// 打印读容量证据
void printReadCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  ReadCapacityEvidence* evidence
);

// 打印所有读容量证据
void printAllReadCapacityEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ReadCapacityEvidence*, va_gc>* evidences
);

// ============================================================================
// 单项分析函数（新接口）
// ============================================================================
// 从单个 ArrayReadAccess 分析并生成所有 ReadCapacityEvidence
// 输入：access
// 输出：(listof ReadCapacityEvidence*)
// 数据流：array-read-access -> (listof read-capacity-evidence)
ArrayDetectErrorCode analyzeReadAccessToEvidences (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<ReadCapacityEvidence*, va_gc>** results
);

} // namespace array_detect_ns
