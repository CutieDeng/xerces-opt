#pragma once

#include "gcc-common.hh"
#include "prelude.hh"
#include "field-write.hh"
#include "field-wrapper.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// 写入原始来源 (WriteOriginalSource)
// ============================================================================
// 数据流位置：field-write-info -> write-original-source
// 分析 field-write-info.rhs 的语义来源
//
// (write-original-source
//   source-type : source-type-enum  ; discriminator
//   data        : (union             ; 根据 source-type 选择
//                   function-call-source
//                   constant-source
//                   field-access-source
//                   computation-source
//                   phi-source))
// ============================================================================

// 来源类型枚举
enum SourceType {
  SOURCE_UNKNOWN,        // 未知来源（追踪失败）
  SOURCE_FUNCTION_CALL,  // 函数调用
  SOURCE_CONSTANT,       // 常量
  SOURCE_FIELD_ACCESS,   // 字段访问（如 b.ptr）
  SOURCE_COMPUTATION,    // 计算表达式
  SOURCE_PHI             // PHI 节点
};

// 向后兼容别名
typedef SourceType FieldSourceType;

// 函数调用来源信息
struct FunctionCallSource {
  gimple *call_stmt;           // GIMPLE_CALL 语句（GCC 内部管理）
  CallType call_type;          // 调用类型（CALL_VIRTUAL, CALL_DIRECT, CALL_INDIRECT, CALL_UNKNOWN）
  char const *function_name;   // 函数名（mangled，ggc_strdup 分配）
  location_t location;         // 调用位置（GCC 内部管理）
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

// 写入原始来源 Variant（使用 union 实现）
struct WriteOriginalSource {
  SourceType source_type;  // 来源类型（discriminator）
  union {
    FunctionCallSource function_call;  // 函数调用来源
    ConstantSource constant;           // 常量来源
    FieldAccessSource field_access;    // 字段访问来源
    ComputationSource computation;     // 计算来源
    PhiSource phi;                     // PHI 节点来源
  } data;
};

// 向后兼容别名
typedef WriteOriginalSource FieldSourceInfo;

// ============================================================================
// Variant 访问宏
// ============================================================================

// 检查来源类型
#define SOURCE_IS_FUNCTION_CALL(src) ((src).source_type == ::array_detector::SOURCE_FUNCTION_CALL)
#define SOURCE_IS_CONSTANT(src) ((src).source_type == ::array_detector::SOURCE_CONSTANT)
#define SOURCE_IS_FIELD_ACCESS(src) ((src).source_type == ::array_detector::SOURCE_FIELD_ACCESS)
#define SOURCE_IS_COMPUTATION(src) ((src).source_type == ::array_detector::SOURCE_COMPUTATION)
#define SOURCE_IS_PHI(src) ((src).source_type == ::array_detector::SOURCE_PHI)
#define SOURCE_IS_UNKNOWN(src) ((src).source_type == ::array_detector::SOURCE_UNKNOWN)

// 向后兼容宏
#define FIELD_SOURCE_IS_FUNCTION_CALL(src) SOURCE_IS_FUNCTION_CALL(src)
#define FIELD_SOURCE_IS_CONSTANT(src) SOURCE_IS_CONSTANT(src)
#define FIELD_SOURCE_IS_FIELD_ACCESS(src) SOURCE_IS_FIELD_ACCESS(src)
#define FIELD_SOURCE_IS_COMPUTATION(src) SOURCE_IS_COMPUTATION(src)
#define FIELD_SOURCE_IS_PHI(src) SOURCE_IS_PHI(src)
#define FIELD_SOURCE_IS_UNKNOWN(src) SOURCE_IS_UNKNOWN(src)

// 安全访问来源数据
#define SOURCE_GET_FUNCTION_CALL(src) \
  (SOURCE_IS_FUNCTION_CALL(src) ? &((src).data.function_call) : nullptr)
#define SOURCE_GET_CONSTANT(src) \
  (SOURCE_IS_CONSTANT(src) ? &((src).data.constant) : nullptr)
#define SOURCE_GET_FIELD_ACCESS(src) \
  (SOURCE_IS_FIELD_ACCESS(src) ? &((src).data.field_access) : nullptr)
#define SOURCE_GET_COMPUTATION(src) \
  (SOURCE_IS_COMPUTATION(src) ? &((src).data.computation) : nullptr)
#define SOURCE_GET_PHI(src) \
  (SOURCE_IS_PHI(src) ? &((src).data.phi) : nullptr)

// 向后兼容 GET 宏
#define FIELD_SOURCE_GET_FUNCTION_CALL(src) SOURCE_GET_FUNCTION_CALL(src)
#define FIELD_SOURCE_GET_CONSTANT(src) SOURCE_GET_CONSTANT(src)
#define FIELD_SOURCE_GET_FIELD_ACCESS(src) SOURCE_GET_FIELD_ACCESS(src)
#define FIELD_SOURCE_GET_COMPUTATION(src) SOURCE_GET_COMPUTATION(src)
#define FIELD_SOURCE_GET_PHI(src) SOURCE_GET_PHI(src)

// ============================================================================
// Variant 模式匹配宏（类似 Rust 的 if let）
// ============================================================================
// 用法：
//   LET_SOURCE_FUNCTION_CALL(func_call, source) {
//     // 使用 func_call : FunctionCallSource&
//   } END_LET()
// ============================================================================

#define LET_SOURCE_FUNCTION_CALL(VAR, SRC) \
  if (SOURCE_IS_FUNCTION_CALL(SRC)) { \
    ::array_detector::FunctionCallSource& VAR = (SRC).data.function_call;

#define LET_SOURCE_CONSTANT(VAR, SRC) \
  if (SOURCE_IS_CONSTANT(SRC)) { \
    ::array_detector::ConstantSource& VAR = (SRC).data.constant;

#define LET_SOURCE_FIELD_ACCESS(VAR, SRC) \
  if (SOURCE_IS_FIELD_ACCESS(SRC)) { \
    ::array_detector::FieldAccessSource& VAR = (SRC).data.field_access;

#define LET_SOURCE_COMPUTATION(VAR, SRC) \
  if (SOURCE_IS_COMPUTATION(SRC)) { \
    ::array_detector::ComputationSource& VAR = (SRC).data.computation;

#define LET_SOURCE_PHI(VAR, SRC) \
  if (SOURCE_IS_PHI(SRC)) { \
    ::array_detector::PhiSource& VAR = (SRC).data.phi;

#define END_LET() \
  }

} // namespace array_detector

// ============================================================================
// 公开接口声明
// ============================================================================
// 数据流：field-write-info -> write-original-source
// 输出模式：直接写入 Wrapper 成员地址

namespace array_detector {

class ArrayDetector;

// 主入口：追踪写入来源
// 从 FieldWriteInfo.rhs 追踪到语义来源
// 输出：直接写入 out_kind 和 out_data 指向的地址
ArrayDetectErrorCode traceWriteSource (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  FieldWriteInfo* write_info,
  FieldSourceKind* out_kind,        // 直接写入来源类型
  FieldSourceDataUnion* out_data    // 直接写入来源数据
);

// Pipeline 接口：追踪所有字段赋值
ArrayDetectErrorCode traceFieldAssignments (
  AD_FUNC_ARGS,
  ArrayDetector& detector
);

} // namespace array_detector
