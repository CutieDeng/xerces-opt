#pragma once

// ============================================================================
// Field 分析聚合数据结构
// ============================================================================
//
// 数据流总览（lisp 风格）：
//
// [写入级分析 - Wrapper 合并所有 1:1 关系]
// field-write + write-source + use-analysis + escape-conclude + move-analysis?
//   => FieldWriteAnalysisWrapper
//
// [字段级分析 - 汇总所有写入的分析结果]
// field, (listof field-write-analysis-wrapper)
//   -> field-conclude
//   => FieldAnalysis
//
// Wrapper 合并推导：
// - FieldWrite + WriteSource 能 wrapper (数据流相邻)
// - WriteSource + UseAnalysis 能 wrapper (数据流相邻)
// - UseAnalysis + EscapeConclude 能 wrapper (数据流相邻)
// - WriteSource + EscapeConclude 能 wrapper (用户指定)
// => 传递性合并: FieldWrite + WriteSource + UseAnalysis + EscapeConclude
//
// 模块职责：
// - collect-writes : field -> (listof wrapper), 填充 FieldWrite 部分
// - trace-source   : wrapper -> wrapper, 填充 WriteSource 部分
// - analyze-uses   : wrapper -> wrapper, 填充 UseAnalysis 部分
// - conclude-escape: wrapper -> wrapper, 填充 EscapeConclude 部分
// - analyze-move   : wrapper -> wrapper, 填充 MoveAnalysis 部分 (可选)
// - conclude-field : field, (listof wrapper) -> field-conclude
// ============================================================================

#include "gcc-common.hh"
#include "prelude.hh"

namespace field_analysis {

// ============================================================================
// 前置声明
// ============================================================================

struct FieldWriteAnalysisWrapper;  // 写入级完整分析 (统一 Wrapper)
struct FieldMoveAnalysis;          // 所有权转移分析 (可选组件)
struct FieldConclude;              // 字段级结论
struct FieldAnalysis;              // 字段级完整分析

// ============================================================================
// 基础枚举类型 (带 Field 前缀)
// ============================================================================

// 来源类型
enum FieldSourceKind {
  FIELD_SRC_UNKNOWN,         // 未知
  FIELD_SRC_FUNCTION_CALL,   // 函数调用
  FIELD_SRC_CONSTANT,        // 常量
  FIELD_SRC_FIELD_ACCESS,    // 字段访问（如 b.ptr）
  FIELD_SRC_COMPUTATION,     // 计算表达式
  FIELD_SRC_PHI              // PHI 节点
};

// 使用类型
enum FieldUseKind {
  FIELD_USE_LOAD,            // 读取
  FIELD_USE_STORE,           // 存储
  FIELD_USE_CALL_ARG,        // 函数参数
  FIELD_USE_RETURN,          // 返回值
  FIELD_USE_PHI,             // PHI 节点
  FIELD_USE_ASSIGN,          // 赋值
  FIELD_USE_ARITHMETIC,      // 算术运算
  FIELD_USE_COMPARISON,      // 比较
  FIELD_USE_ADDRESS,         // 取地址
  FIELD_USE_CONDITIONAL,     // 条件
  FIELD_USE_OTHER            // 其他
};

// 逃逸类型
enum FieldEscapeKind {
  FIELD_ESC_NONE = 0,        // 无逃逸
  FIELD_ESC_RETURN,          // 返回值逃逸
  FIELD_ESC_PARAMETER,       // 参数逃逸
  FIELD_ESC_GLOBAL,          // 全局变量
  FIELD_ESC_HEAP,            // 堆存储
  FIELD_ESC_FIELD,           // 字段存储
  FIELD_ESC_INDIRECT_CALL,   // 间接调用
  FIELD_ESC_VIRTUAL_CALL,    // 虚函数调用
  FIELD_ESC_EXTERNAL_CALL,   // 外部函数
  FIELD_ESC_UNKNOWN          // 未知
};

// 调用类型
enum FieldCallKind {
  FIELD_CALL_DIRECT,         // 直接调用
  FIELD_CALL_VIRTUAL,        // 虚函数
  FIELD_CALL_INDIRECT,       // 间接调用（函数指针）
  FIELD_CALL_UNKNOWN         // 未知
};

// 所有权转移结论
enum FieldMoveVerdict {
  FIELD_MOVE_CERTAIN,        // 必然转移（所有路径都销毁源）
  FIELD_MOVE_IMPOSSIBLE,     // 不可能转移（无销毁点）
  FIELD_MOVE_CONDITIONAL,    // 条件转移（部分路径销毁）
  FIELD_MOVE_NOT_APPLICABLE  // 不适用（非字段访问源）
};

// ============================================================================
// 来源数据变体 (带 Field 前缀)
// ============================================================================

struct FieldFunctionCallData {
  gimple* stmt;
  FieldCallKind call_kind;
  char const* name;
  location_t location;
};

struct FieldConstantData {
  tree value;
  char const* str;
};

struct FieldAccessData {
  gimple* stmt;
  tree field;
  tree object;
  tree object_type;
  char const* field_name;
  char const* type_name;
  location_t location;
};

struct FieldComputationData {
  gimple* stmt;
  tree expr;
  char const* desc;
  location_t location;
};

struct FieldPhiData {
  gimple* stmt;
  tree ssa_name;
  tree var;
  char const* var_name;
  location_t location;
};

// ============================================================================
// FieldUsePoint: 单个使用点信息
// ============================================================================

struct FieldUsePoint {
  FieldUseKind kind;
  gimple* stmt;
  tree operand;
  location_t location;
  unsigned int bb_index;

  FieldEscapeKind escape_kind;
  char const* escape_target;

  inline bool is_escape() const { return escape_kind != FIELD_ESC_NONE; }
};

// ============================================================================
// FieldInvalidationPoint: 所有权销毁点
// ============================================================================

struct FieldInvalidationPoint {
  gimple* stmt;
  location_t location;
  basic_block bb;
  char const* desc;
};

// ============================================================================
// FieldMoveAnalysis: 所有权转移分析（可选组件）
// ============================================================================
// 仅当 source_kind == FIELD_SRC_FIELD_ACCESS 时存在

struct FieldMoveAnalysis {
  tree source_field;
  tree source_object;
  gimple* transfer_stmt;
  location_t transfer_location;

  vec<FieldInvalidationPoint*>* invalidations;
  FieldMoveVerdict verdict;

  unsigned int paths_with;
  unsigned int paths_without;
  unsigned int total_paths;

  char const* verdict_desc;
};

// ============================================================================
// FieldWriteAnalysisWrapper: 写入级完整分析（统一 Wrapper）
// ============================================================================
// 合并: FieldWrite + WriteSource + UseAnalysis + EscapeConclude
// 扁平化结构，消除冗余字段

struct FieldWriteAnalysisWrapper {
  // ========== FieldWrite 部分: 字段写入基本信息 ==========
  tree type;                    // TYPE_MAIN_VARIANT
  tree field;                   // FIELD_DECL
  tree func;                    // 所在函数
  basic_block bb;               // 所在基本块
  gimple* stmt;                 // GIMPLE_ASSIGN 语句
  tree lhs;                     // 左值表达式
  tree rhs;                     // 右值表达式 (SSA_NAME)
  location_t write_location;    // 写入位置

  // ========== FieldWriteSource 部分: 来源分析 ==========
  FieldSourceKind source_kind;
  union {
    FieldFunctionCallData function_call;
    FieldConstantData constant;
    FieldAccessData field_access;
    FieldComputationData computation;
    FieldPhiData phi;
  } source_data;

  // ========== FieldUseAnalysis 部分: 使用分析 (合并 SourceUse + EscapedUse) ==========
  tree source_operand;          // 追踪的源操作数
  gimple* source_stmt;          // 源定义语句

  // 所有使用点
  vec<FieldUsePoint>* all_uses;
  unsigned int total_use_count;
  unsigned int max_use_depth;
  bool is_fully_analyzed;

  // 逃逸使用子集 (指向 all_uses 中的元素)
  vec<FieldUsePoint const*>* escape_uses;
  unsigned int escape_count;
  bool has_escape;

  // ========== FieldEscapeConclude 部分: 逃逸结论 ==========
  unsigned int total_escapes;
  unsigned int safe_debug_escapes;
  unsigned int rejecting_escapes;
  bool has_rejecting_evidence;

  // ========== FieldMoveAnalysis 部分: 所有权转移 (可选) ==========
  FieldMoveAnalysis* move;      // NULL if source_kind != FIELD_SRC_FIELD_ACCESS
};

// ============================================================================
// FieldConclude: 字段级结论
// ============================================================================

struct FieldConclude {
  unsigned int total_writes;
  unsigned int writes_with_escape;
  unsigned int writes_with_rejecting;
  unsigned int writes_without_analysis;

  // 来源类型分布
  unsigned int src_function_call;
  unsigned int src_field_access;
  unsigned int src_constant;
  unsigned int src_computation;
  unsigned int src_phi;
  unsigned int src_unknown;

  // 逃逸汇总
  unsigned int total_escapes;
  unsigned int safe_escapes;
  unsigned int rejecting_escapes;

  // 结论
  bool has_rejecting;
  float rejection_ratio;
};

// ============================================================================
// FieldAnalysis: 字段级完整分析（聚合结构）
// ============================================================================

struct FieldAnalysis {
  tree type;
  tree field;

  vec<FieldWriteAnalysisWrapper*>* writes;  // 所有字段写入分析
  FieldConclude conclude;                    // 汇总结论
};

} // namespace field_analysis

// ============================================================================
// 向后兼容：枚举值别名
// ============================================================================

namespace field_analysis {
  // 旧枚举值 -> 新枚举值
  constexpr FieldSourceKind SRC_UNKNOWN = FIELD_SRC_UNKNOWN;
  constexpr FieldSourceKind SRC_FUNCTION_CALL = FIELD_SRC_FUNCTION_CALL;
  constexpr FieldSourceKind SRC_CONSTANT = FIELD_SRC_CONSTANT;
  constexpr FieldSourceKind SRC_FIELD_ACCESS = FIELD_SRC_FIELD_ACCESS;
  constexpr FieldSourceKind SRC_COMPUTATION = FIELD_SRC_COMPUTATION;
  constexpr FieldSourceKind SRC_PHI = FIELD_SRC_PHI;

  constexpr FieldEscapeKind ESC_NONE = FIELD_ESC_NONE;
  constexpr FieldEscapeKind ESC_RETURN = FIELD_ESC_RETURN;
  constexpr FieldEscapeKind ESC_PARAMETER = FIELD_ESC_PARAMETER;
  constexpr FieldEscapeKind ESC_GLOBAL = FIELD_ESC_GLOBAL;
  constexpr FieldEscapeKind ESC_HEAP = FIELD_ESC_HEAP;
  constexpr FieldEscapeKind ESC_FIELD = FIELD_ESC_FIELD;
  constexpr FieldEscapeKind ESC_INDIRECT_CALL = FIELD_ESC_INDIRECT_CALL;
  constexpr FieldEscapeKind ESC_VIRTUAL_CALL = FIELD_ESC_VIRTUAL_CALL;
  constexpr FieldEscapeKind ESC_EXTERNAL_CALL = FIELD_ESC_EXTERNAL_CALL;
  constexpr FieldEscapeKind ESC_UNKNOWN = FIELD_ESC_UNKNOWN;

  constexpr FieldMoveVerdict MOVE_CERTAIN = FIELD_MOVE_CERTAIN;
  constexpr FieldMoveVerdict MOVE_IMPOSSIBLE = FIELD_MOVE_IMPOSSIBLE;
  constexpr FieldMoveVerdict MOVE_CONDITIONAL = FIELD_MOVE_CONDITIONAL;
  constexpr FieldMoveVerdict MOVE_NOT_APPLICABLE = FIELD_MOVE_NOT_APPLICABLE;

  // 旧类型别名
  using SourceKind = FieldSourceKind;
  using UseKind = FieldUseKind;
  using EscapeKind = FieldEscapeKind;
  using CallKind = FieldCallKind;
  using MoveVerdict = FieldMoveVerdict;
  using UsePoint = FieldUsePoint;
  using MoveAnalysis = FieldMoveAnalysis;
  using InvalidationPoint = FieldInvalidationPoint;

  // 旧结构别名 (指向 Wrapper 的分段)
  // 注意：这些只是为了编译兼容，语义上已被 Wrapper 替代
  using FieldWrite = FieldWriteAnalysisWrapper;
  using WriteSource = FieldWriteAnalysisWrapper;
  using UseAnalysis = FieldWriteAnalysisWrapper;
  using EscapeConclude = FieldWriteAnalysisWrapper;
  using FieldWriteAnalysis = FieldWriteAnalysisWrapper;

  // 旧 Data 结构别名
  using FunctionCallData = FieldFunctionCallData;
  using ConstantData = FieldConstantData;
  using ComputationData = FieldComputationData;
  using PhiData = FieldPhiData;
}

// ============================================================================
// 向后兼容：映射到旧命名空间
// ============================================================================

namespace array_detect_ns {
  using FieldWriteAnalysisWrapper = field_analysis::FieldWriteAnalysisWrapper;
  using FieldMoveAnalysis = field_analysis::FieldMoveAnalysis;
  using FieldConclude = field_analysis::FieldConclude;
  using FieldAnalysis = field_analysis::FieldAnalysis;
  using FieldUsePoint = field_analysis::FieldUsePoint;

  // 枚举别名
  using FieldSourceKind = field_analysis::FieldSourceKind;
  using FieldUseKind = field_analysis::FieldUseKind;
  using FieldEscapeKind = field_analysis::FieldEscapeKind;
  using FieldMoveVerdict = field_analysis::FieldMoveVerdict;

  // 旧名称别名
  using SourceKind = field_analysis::FieldSourceKind;
  using UseKind = field_analysis::FieldUseKind;
  using EscapeKind = field_analysis::FieldEscapeKind;
  using MoveVerdict = field_analysis::FieldMoveVerdict;
}

namespace array_detector {
  using FieldWriteAnalysisWrapper = field_analysis::FieldWriteAnalysisWrapper;
  using FieldAnalysis = field_analysis::FieldAnalysis;

  // 旧名称别名
  using WriteSource = field_analysis::FieldWriteAnalysisWrapper;
  using FieldWrite = field_analysis::FieldWriteAnalysisWrapper;
  using FieldWriteAnalysis = field_analysis::FieldWriteAnalysisWrapper;
}
