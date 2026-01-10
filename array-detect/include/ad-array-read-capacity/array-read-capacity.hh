#pragma once

// ============================================================================
// ad-array-read-capacity 模块
// ============================================================================
// 收集数组读取访问，分析读边界条件，寻找关联整数字段
//
// 数据流：
//   pointer-field -> (listof array-read-access)
//   array-read-access -> (or read-bound-condition #f)
//   pointer-field, read-bound-condition -> (or read-capacity-evidence #f)
//
// 场景：分析 if (i < arr->size) x = arr->data[i] 中 size 字段关联
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 置信度枚举
// ============================================================================

enum ReadEvidenceConfidence {
  READ_CONF_CERTAIN,    // 确定：直接边界检查
  READ_CONF_PROBABLE,   // 可能：间接边界检查
  READ_CONF_WEAK        // 弱：推测性关联
};

// ============================================================================
// 数组读取访问
// ============================================================================
// (array-read-access
//   stmt             : gimple                 ; 读取语句
//   base_pointer     : tree                   ; 基指针
//   index_expr       : tree                   ; 索引表达式
//   containing_func  : tree)                  ; 所属函数

struct ArrayReadAccess {
  // === 访问信息 ===
  gimple* stmt;                   // 读取语句
  tree base_pointer;              // 基指针 (SSA_NAME 或表达式)
  tree index_expr;                // 索引表达式
  tree element_type;              // 元素类型

  // === 字段关联 ===
  bool is_field_based;            // 基指针是否来自字段访问
  tree containing_type;           // 包含类型
  tree pointer_field_decl;        // 指针字段声明

  // === 上下文 ===
  function* fn;                   // 所在函数
  basic_block bb;                 // 基本块
  location_t location;            // 源码位置
};

// ============================================================================
// 读边界条件
// ============================================================================
// (read-bound-condition
//   access           : array-read-access      ; 原访问
//   condition_stmt   : gimple                 ; 条件语句
//   bound_field      : (or tree #f))          ; 边界字段

struct ReadBoundCondition {
  // === 关联的访问 ===
  ArrayReadAccess* access;

  // === 条件信息 ===
  gimple* condition_stmt;         // 条件语句 (GIMPLE_COND)
  tree_code comparison_code;      // 比较操作符 (LT_EXPR, LE_EXPR 等)
  tree index_operand;             // 索引操作数
  tree bound_operand;             // 边界操作数

  // === 边界字段 ===
  bool has_field_bound;           // 边界是否来自字段
  tree bound_field_decl;          // 边界字段 (FIELD_DECL)
  tree bound_type;                // 边界所属类型

  // === 支配关系 ===
  basic_block condition_bb;       // 条件所在基本块
  bool dominates_access;          // 条件是否支配访问

  // === 位置 ===
  location_t location;
};

// ============================================================================
// 读容量证据
// ============================================================================
// (read-capacity-evidence
//   pointer_field    : tree                   ; 指针字段
//   integer_field    : tree                   ; 关联的整数字段
//   read_access      : array-read-access      ; 读访问
//   bound_condition  : read-bound-condition   ; 边界条件
//   confidence       : (or 'certain 'probable 'weak))

struct ReadCapacityEvidence {
  // === 关联信息 ===
  tree pointer_field;             // 指针字段 (FIELD_DECL)
  tree integer_field;             // 关联的整数字段 (FIELD_DECL)
  tree containing_type;           // 所属类型

  // === 来源信息 ===
  ArrayReadAccess* read_access;   // 读访问
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

// 收集单个语句中的数组读取访问
// 输入：stmt, func
// 输出：ArrayReadAccess* (如果是数组读取) 或 NULL
ArrayDetectErrorCode collectArrayReadAccess (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  tree target_type,
  tree target_field,
  ArrayReadAccess** result
);

// 分析数组读取的边界条件
// 输入：ArrayReadAccess*
// 输出：ReadBoundCondition* (如果有边界检查) 或 NULL
ArrayDetectErrorCode analyzeReadBoundCondition (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  ReadBoundCondition** result
);

// 提取读容量证据
// 输入：pointer_field, bound_condition
// 输出：ReadCapacityEvidence* (如果找到关联) 或 NULL
ArrayDetectErrorCode extractReadCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  ReadBoundCondition* bound_cond,
  ReadCapacityEvidence** result
);

// 收集所有读容量证据
// 输入：TypeFieldAnalysisData
// 输出：填充 tfad->read_evidences
ArrayDetectErrorCode collectAllReadEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 检查表达式是否为数组读取
bool isArrayReadExpr (tree expr);

// 追溯基指针到字段访问
ArrayDetectErrorCode traceBasePointerToField (
  AD_FUNC_ARGS,
  tree base_pointer,
  tree* out_type,
  tree* out_field_decl
);

// 查找支配该访问的条件语句
ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<gimple*, va_gc>** result
);

// 打印读容量证据
void printReadCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  ReadCapacityEvidence* evidence
);

// 打印所有读容量证据
void printAllReadEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ReadCapacityEvidence*, va_gc>* evidences
);

// 获取置信度名称
char const* readConfidenceToString (ReadEvidenceConfidence conf);

} // namespace array_detect_ns
