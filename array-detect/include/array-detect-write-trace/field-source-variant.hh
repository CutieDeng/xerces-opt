#pragma once

#include "gcc-common.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// 字段来源信息 Variant 类型（不使用 C++ variant）
// ============================================================================
// 使用 union + enum 实现类似 variant 的功能
// ============================================================================

// 字段来源类型枚举
enum FieldSourceType {
  SOURCE_UNKNOWN,        // 未知来源
  SOURCE_FUNCTION_CALL,  // 函数调用（包括虚函数、直接调用、间接调用）
  SOURCE_VARIABLE,       // 变量（SSA_NAME）
  SOURCE_CONSTANT,       // 常量
  SOURCE_FIELD_ACCESS,   // 字段访问（对另一对象的字段读取，如 b.ptr）
  SOURCE_COMPUTATION,    // 计算表达式
  SOURCE_PHI             // PHI 节点（分支合并点，多个来源）
};

// 函数调用来源信息
struct FunctionCallSource {
  gimple *call_stmt;           // GIMPLE_CALL 语句（GCC 内部管理）
  CallType call_type;          // 调用类型（CALL_VIRTUAL, CALL_DIRECT, CALL_INDIRECT, CALL_UNKNOWN）
  char const *function_name;   // 函数名（mangled，ggc_strdup 分配）
  location_t location;         // 调用位置（GCC 内部管理）
};

// 变量来源信息
struct VariableSource {
  tree ssa_name;               // SSA_NAME（GCC 内部管理）
  tree var_decl;               // 变量声明（VAR_DECL，GCC 内部管理，可能为 NULL）
  char const *var_name;        // 变量名（ggc_strdup 分配，可能为 NULL）
  location_t location;         // 变量定义位置（GCC 内部管理）
};

// 常量来源信息
struct ConstantSource {
  tree constant_value;         // 常量值（CONSTANT_CLASS_P，GCC 内部管理）
  char const *constant_str;    // 常量字符串表示（ggc_strdup 分配，用于调试）
};

// 字段访问来源信息（对另一对象的字段读取，如 b.ptr）
struct FieldAccessSource {
  gimple *access_stmt;         // 访问语句（GIMPLE_ASSIGN，GCC 内部管理）
  tree access_expr;            // 字段访问表达式（MEM_REF/COMPONENT_REF，GCC 内部管理）
  tree field_decl;             // 字段声明（FIELD_DECL，GCC 内部管理）
  char const *field_name;      // 字段名（ggc_strdup 分配）
  tree object_type;            // 对象类型（RECORD_TYPE，GCC 内部管理）
  char const *type_name;       // 类型名（ggc_strdup 分配）
  tree base_object;            // 基对象（可能是 SSA_NAME/VAR_DECL 等，GCC 内部管理）
  location_t location;         // 访问位置（GCC 内部管理）
};

// 计算表达式来源信息
struct ComputationSource {
  gimple *compute_stmt;        // 计算语句（GIMPLE_ASSIGN，GCC 内部管理）
  tree compute_expr;           // 计算表达式（GCC 内部管理）
  char const *description;     // 计算描述（ggc_strdup 分配）
  location_t location;         // 计算位置（GCC 内部管理）
};

// PHI 节点来源信息
struct PhiSource {
  gimple *phi_stmt;            // GIMPLE_PHI 语句（GCC 内部管理）
  tree ssa_name;               // PHI 的结果 SSA_NAME（GCC 内部管理）
  tree var_decl;               // 变量声明（VAR_DECL，GCC 内部管理，可能为 NULL）
  char const *var_name;        // 变量名（ggc_strdup 分配，可能为 NULL）
  location_t location;         // PHI 节点位置（GCC 内部管理）
};

// 字段来源信息 Variant（使用 union 实现）
struct FieldSourceInfo {
  FieldSourceType source_type;  // 来源类型（discriminator）
  union {
    FunctionCallSource function_call;  // 函数调用来源
    VariableSource variable;           // 变量来源
    ConstantSource constant;           // 常量来源
    FieldAccessSource field_access;    // 字段访问来源
    ComputationSource computation;     // 计算来源
    PhiSource phi;                     // PHI 节点来源
  } data;
};

// ============================================================================
// Variant 访问宏（安全访问，参考 CallMatchResult 的模式）
// ============================================================================

// 检查来源类型
#define FIELD_SOURCE_IS_FUNCTION_CALL(src) ((src).source_type == ::array_detector::SOURCE_FUNCTION_CALL)
#define FIELD_SOURCE_IS_VARIABLE(src) ((src).source_type == ::array_detector::SOURCE_VARIABLE)
#define FIELD_SOURCE_IS_CONSTANT(src) ((src).source_type == ::array_detector::SOURCE_CONSTANT)
#define FIELD_SOURCE_IS_FIELD_ACCESS(src) ((src).source_type == ::array_detector::SOURCE_FIELD_ACCESS)
#define FIELD_SOURCE_IS_COMPUTATION(src) ((src).source_type == ::array_detector::SOURCE_COMPUTATION)
#define FIELD_SOURCE_IS_PHI(src) ((src).source_type == ::array_detector::SOURCE_PHI)
#define FIELD_SOURCE_IS_UNKNOWN(src) ((src).source_type == ::array_detector::SOURCE_UNKNOWN)

// 安全访问函数调用来源
#define FIELD_SOURCE_GET_FUNCTION_CALL(src) \
  (FIELD_SOURCE_IS_FUNCTION_CALL (src) ? &((src).data.function_call) : nullptr)

// 安全访问变量来源
#define FIELD_SOURCE_GET_VARIABLE(src) \
  (FIELD_SOURCE_IS_VARIABLE (src) ? &((src).data.variable) : nullptr)

// 安全访问常量来源
#define FIELD_SOURCE_GET_CONSTANT(src) \
  (FIELD_SOURCE_IS_CONSTANT (src) ? &((src).data.constant) : nullptr)

// 安全访问字段访问来源
#define FIELD_SOURCE_GET_FIELD_ACCESS(src) \
  (FIELD_SOURCE_IS_FIELD_ACCESS (src) ? &((src).data.field_access) : nullptr)

// 安全访问计算来源
#define FIELD_SOURCE_GET_COMPUTATION(src) \
  (FIELD_SOURCE_IS_COMPUTATION (src) ? &((src).data.computation) : nullptr)

// 安全访问 PHI 来源
#define FIELD_SOURCE_GET_PHI(src) \
  (FIELD_SOURCE_IS_PHI (src) ? &((src).data.phi) : nullptr)

// ============================================================================
// Variant 模式匹配宏（类似 Rust 的 if let，使用引用）
// ============================================================================
// 用法：
//   LET_SOURCE_FUNCTION_CALL (func_call, source_info) {
//     // 使用 func_call，类型为 FunctionCallSource&
//     AD_DEBUG_PRINT ("Function: %s", func_call.function_name);
//   } END_LET ()
// 
// 展开为：
//   if (FIELD_SOURCE_IS_FUNCTION_CALL (source_info)) {
//     FunctionCallSource& func_call = source_info.data.function_call;
//     // 使用 func_call
//   }
// ============================================================================

// 函数调用来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_FUNCTION_CALL(VAR, SRC) \
  if (FIELD_SOURCE_IS_FUNCTION_CALL (SRC)) { \
    ::array_detector::FunctionCallSource& VAR = (SRC).data.function_call;

// 变量来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_VARIABLE(VAR, SRC) \
  if (FIELD_SOURCE_IS_VARIABLE (SRC)) { \
    ::array_detector::VariableSource& VAR = (SRC).data.variable;

// 常量来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_CONSTANT(VAR, SRC) \
  if (FIELD_SOURCE_IS_CONSTANT (SRC)) { \
    ::array_detector::ConstantSource& VAR = (SRC).data.constant;

// 字段访问来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_FIELD_ACCESS(VAR, SRC) \
  if (FIELD_SOURCE_IS_FIELD_ACCESS (SRC)) { \
    ::array_detector::FieldAccessSource& VAR = (SRC).data.field_access;

// 计算来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_COMPUTATION(VAR, SRC) \
  if (FIELD_SOURCE_IS_COMPUTATION (SRC)) { \
    ::array_detector::ComputationSource& VAR = (SRC).data.computation;

// PHI 来源模式匹配
// VAR: 变量名（引用类型）
// SRC: FieldSourceInfo 对象（值或引用）
#define LET_SOURCE_PHI(VAR, SRC) \
  if (FIELD_SOURCE_IS_PHI (SRC)) { \
    ::array_detector::PhiSource& VAR = (SRC).data.phi;

// 结束模式匹配块
// 展开为：}
#define END_LET() \
  }

} // namespace array_detector
