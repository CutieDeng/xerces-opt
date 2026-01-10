#pragma once

// ============================================================================
// ad-array-write-capacity 模块
// ============================================================================
// 收集数组写入访问，分析写边界条件，寻找关联整数字段
//
// 数据流：
//   pointer-field -> (listof array-write-access)
//   array-write-access -> (or write-bound-condition #f)
//   pointer-field, write-bound-condition -> (or write-capacity-evidence #f)
//
// 场景：分析 if (i < arr->cap) arr->data[i] = x 中 cap 字段关联
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

enum WriteEvidenceConfidence {
  WRITE_CONF_CERTAIN,    // 确定：直接边界检查
  WRITE_CONF_PROBABLE,   // 可能：间接边界检查
  WRITE_CONF_WEAK        // 弱：推测性关联
};

// ============================================================================
// 数组写入访问
// ============================================================================
// (array-write-access
//   stmt             : gimple                 ; 写入语句
//   base_pointer     : tree                   ; 基指针
//   index_expr       : tree                   ; 索引表达式
//   containing_func  : tree)                  ; 所属函数

struct ArrayWriteAccess {
  // === 访问信息 ===
  gimple* stmt;                   // 写入语句
  tree base_pointer;              // 基指针 (SSA_NAME 或表达式)
  tree index_expr;                // 索引表达式
  tree element_type;              // 元素类型
  tree written_value;             // 写入的值

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
// 写边界条件
// ============================================================================
// (write-bound-condition
//   access           : array-write-access     ; 原访问
//   condition_stmt   : gimple                 ; 条件语句
//   bound_field      : (or tree #f))          ; 边界字段

struct WriteBoundCondition {
  // === 关联的访问 ===
  ArrayWriteAccess* access;

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
// 写容量证据
// ============================================================================
// (write-capacity-evidence
//   pointer_field    : tree                   ; 指针字段
//   integer_field    : tree                   ; 关联的整数字段
//   write_access     : array-write-access     ; 写访问
//   bound_condition  : write-bound-condition  ; 边界条件
//   confidence       : (or 'certain 'probable 'weak))

struct WriteCapacityEvidence {
  // === 关联信息 ===
  tree pointer_field;             // 指针字段 (FIELD_DECL)
  tree integer_field;             // 关联的整数字段 (FIELD_DECL)
  tree containing_type;           // 所属类型

  // === 来源信息 ===
  ArrayWriteAccess* write_access; // 写访问
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

// 收集单个语句中的数组写入访问
// 输入：stmt, func
// 输出：ArrayWriteAccess* (如果是数组写入) 或 NULL
ArrayDetectErrorCode collectArrayWriteAccess (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  tree target_type,
  tree target_field,
  ArrayWriteAccess** result
);

// 分析数组写入的边界条件
// 输入：ArrayWriteAccess*
// 输出：WriteBoundCondition* (如果有边界检查) 或 NULL
ArrayDetectErrorCode analyzeWriteBoundCondition (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  WriteBoundCondition** result
);

// 提取写容量证据
// 输入：pointer_field, bound_condition
// 输出：WriteCapacityEvidence* (如果找到关联) 或 NULL
ArrayDetectErrorCode extractWriteCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  WriteBoundCondition* bound_cond,
  WriteCapacityEvidence** result
);

// 收集所有写容量证据
// 输入：TypeFieldAnalysisData
// 输出：填充 tfad->write_evidences
ArrayDetectErrorCode collectAllWriteEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 检查表达式是否为数组写入目标
bool isArrayWriteTarget (tree expr);

// 打印写容量证据
void printWriteCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  WriteCapacityEvidence* evidence
);

// 打印所有写容量证据
void printAllWriteEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<WriteCapacityEvidence*, va_gc>* evidences
);

// 获取置信度名称
char const* writeConfidenceToString (WriteEvidenceConfidence conf);

} // namespace array_detect_ns
