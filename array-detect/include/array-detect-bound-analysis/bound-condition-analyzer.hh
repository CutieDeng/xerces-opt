#pragma once

#include "prelude.hh"
#include "context.hh"
#include "array-access-collector.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 边界条件类型
// ============================================================================

enum BoundConditionType {
  BOUND_COND_NONE,              // 无边界条件
  BOUND_COND_LT_FIELD,          // i < obj->size
  BOUND_COND_LE_FIELD,          // i <= obj->size
  BOUND_COND_LT_CONSTANT,       // i < CONST
  BOUND_COND_LE_CONSTANT,       // i <= CONST
  BOUND_COND_GT_FIELD,          // i > obj->size (反向条件)
  BOUND_COND_GE_FIELD,          // i >= obj->size (反向条件)
  BOUND_COND_COMPLEX            // 复杂条件（需要进一步分析）
};

// ============================================================================
// 比较方向（用于规范化条件）
// ============================================================================

enum ComparisonDirection {
  CMP_INDEX_LT_BOUND,           // index < bound
  CMP_INDEX_LE_BOUND,           // index <= bound
  CMP_INDEX_GT_BOUND,           // index > bound
  CMP_INDEX_GE_BOUND,           // index >= bound
  CMP_BOUND_GT_INDEX,           // bound > index (等价于 index < bound)
  CMP_BOUND_GE_INDEX,           // bound >= index (等价于 index <= bound)
  CMP_UNKNOWN
};

// ============================================================================
// 边界条件关联
// ============================================================================

struct BoundConditionAssociation {
  // === 条件信息 ===
  BoundConditionType condition_type;
  tree condition_expr;            // 条件表达式
  gimple* condition_stmt;         // 条件语句（GIMPLE_COND）
  tree_code comparison_code;      // 比较操作符（LT_EXPR, LE_EXPR 等）

  // === 索引变量信息 ===
  tree index_var;                 // 索引变量（SSA_NAME）
  tree index_origin;              // 索引变量的来源（如果可追溯）

  // === 边界来源信息 ===
  bool is_field_bound;            // 边界是否来自字段
  tree bound_field_decl;          // 边界字段（如果 is_field_bound 为 true）
  tree bound_type;                // 边界所属类型
  tree bound_expr;                // 边界表达式
  HOST_WIDE_INT constant_bound;   // 常量边界值（如果是常量）

  // === 支配关系 ===
  basic_block condition_bb;       // 条件所在基本块
  bool dominates_access;          // 条件是否支配访问
  int dominance_depth;            // 支配深度（条件到访问的路径长度）

  // === 位置信息 ===
  location_t location;
  char const* description;
};

// ============================================================================
// 单次数组访问的边界分析结果
// ============================================================================

struct ArrayAccessBoundAnalysis {
  // === 关联的访问 ===
  ArrayAccessCapture* access;

  // === 边界条件列表 ===
  vec<BoundConditionAssociation*, va_gc>* bounds;

  // === 统计信息 ===
  bool has_valid_bound;                       // 是否有有效边界条件
  unsigned int field_bound_count;             // 字段边界数量
  unsigned int constant_bound_count;          // 常量边界数量

  // === 最佳匹配 ===
  BoundConditionAssociation* primary_bound;   // 主要边界条件（最相关的）

  // === 关联的字段（用于后续归约）===
  vec<tree, va_gc>* related_fields;           // 所有关联的边界字段
};

// ============================================================================
// 函数声明
// ============================================================================

// 分析所有数组访问的边界条件
ArrayDetectErrorCode analyzeAllBoundConditions (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses
);

// 分析单个数组访问的边界条件
ArrayDetectErrorCode analyzeAccessBoundConditions (
  AD_FUNC_ARGS,
  ArrayAccessCapture* access,
  ArrayAccessBoundAnalysis** out_analysis
);

// 查找支配该访问的条件语句
ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayAccessCapture* access,
  vec<gimple*, va_gc>** out_conditions
);

// 分析条件是否为边界检查
ArrayDetectErrorCode analyzeBoundCondition (
  AD_FUNC_ARGS,
  gimple* cond_stmt,
  tree index_var,
  ArrayAccessCapture* access,
  BoundConditionAssociation** out_association
);

// 检查表达式是否引用了索引变量
bool expressionInvolvesIndex (
  AD_FUNC_ARGS,
  tree expr,
  tree index_var
);

// 追溯表达式到字段访问
ArrayDetectErrorCode traceExpressionToField (
  AD_FUNC_ARGS,
  tree expr,
  tree* out_type,
  tree* out_field_decl
);

// 规范化比较方向
ComparisonDirection normalizeComparison (
  tree_code code,
  tree lhs,
  tree rhs,
  tree index_var
);

// 调试输出
void printBoundConditionAssociation (
  AD_FUNC_ARGS,
  FILE* out,
  BoundConditionAssociation* assoc
);

void printArrayAccessBoundAnalysis (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayAccessBoundAnalysis* analysis
);

void printAllBoundAnalyses (
  AD_FUNC_ARGS,
  FILE* out,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses
);

// 获取边界条件类型名称
const char* getBoundConditionTypeName (BoundConditionType type);

} // namespace array_detect_ns
