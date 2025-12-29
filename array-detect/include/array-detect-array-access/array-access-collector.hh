#pragma once

#include "prelude.hh"
#include "context.hh"
#include "array-detector.hh"
#include "type-field-hash-traits.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// 前向声明
struct TypeFieldArrayAccesses;

// 为 TypeFieldArrayAccesses* 定义专用的 hash_map traits
typedef simple_hashmap_traits<default_hash_traits<TypeFieldKey>, TypeFieldArrayAccesses*>
  TypeFieldArrayAccessesHashMapTraits;

// ============================================================================
// 数组访问类型
// ============================================================================

enum ArrayAccessType {
  ACCESS_ARRAY_REF,      // ptr[i] 形式的 ARRAY_REF
  ACCESS_MEM_REF,        // *(ptr + offset) 形式的 MEM_REF
  ACCESS_POINTER_PLUS    // ptr + offset 形式（指针算术）
};

// ============================================================================
// 访问方向
// ============================================================================

enum AccessDirection {
  ACCESS_READ,           // 读取
  ACCESS_WRITE           // 写入
};

// ============================================================================
// 单次数组访问捕获
// ============================================================================

struct ArrayAccessCapture {
  // === 访问类型信息 ===
  ArrayAccessType access_type;      // 访问类型
  AccessDirection direction;        // 访问方向（读/写）

  // === 基础指针信息 ===
  tree base_pointer;                // 基础指针（SSA_NAME 或表达式）
  tree base_type;                   // 指针指向的类型
  tree element_type;                // 数组元素类型

  // === 偏移量信息 ===
  tree offset_expr;                 // 偏移量表达式（可能是 SSA_NAME、常量、或复杂表达式）
  bool is_constant_offset;          // 是否为常量偏移
  HOST_WIDE_INT constant_offset;    // 常量偏移值（仅当 is_constant_offset 为 true 时有效）

  // === 关联字段信息（如果基础指针来自字段访问）===
  bool is_field_based;              // 基础指针是否来自字段访问
  tree containing_type;             // 包含类型（RECORD_TYPE）
  tree pointer_field_decl;          // 指针字段声明（FIELD_DECL）

  // === 上下文信息 ===
  gimple* stmt;                     // GIMPLE 语句
  function* fn;                     // 所在函数
  basic_block bb;                   // 基本块
  location_t location;              // 源码位置

  // === 辅助信息 ===
  tree access_expr;                 // 完整的访问表达式
  char const* description;          // 描述信息

  // === 边界分析结果（由 bound-analysis 模块填充）===
  void* bound_analysis;             // ArrayAccessBoundAnalysis* (前向声明避免循环依赖)
};

// ============================================================================
// 按 (type, field) 聚合的数组访问列表
// ============================================================================

struct TypeFieldArrayAccesses {
  tree type;                                  // 类型
  tree pointer_field_decl;                    // 指针字段
  char const* type_name;                      // 类型名（缓存）
  char const* field_name;                     // 字段名（缓存）
  vec<ArrayAccessCapture*, va_gc>* accesses;  // 该字段的所有访问捕获
  unsigned int read_count;                    // 读访问次数
  unsigned int write_count;                   // 写访问次数
};

// ============================================================================
// 函数声明
// ============================================================================

// 收集函数内的所有数组访问
ArrayDetectErrorCode collectFunctionArrayAccesses (
  AD_FUNC_ARGS,
  function* fn,
  vec<ArrayAccessCapture*, va_gc>** out_accesses
);

// 判断表达式是否为数组访问，如果是则创建捕获
ArrayDetectErrorCode analyzeArrayAccess (
  AD_FUNC_ARGS,
  tree expr,
  gimple* stmt,
  function* fn,
  AccessDirection direction,
  ArrayAccessCapture** out_capture
);

// 追溯基础指针到字段访问
ArrayDetectErrorCode traceBasePointerToField (
  AD_FUNC_ARGS,
  tree base_pointer,
  tree* out_type,
  tree* out_field_decl
);

// 收集所有函数的数组访问并按 (type, field) 聚合
ArrayDetectErrorCode collectAllArrayAccessesByTypeField (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>** out_map
);

// 获取或创建 TypeFieldArrayAccesses 条目
ArrayDetectErrorCode getOrCreateTypeFieldAccesses (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* map,
  tree type,
  tree field_decl,
  TypeFieldArrayAccesses** out_entry
);

// 调试输出
void printArrayAccessCapture (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayAccessCapture* capture
);

void printAllArrayAccesses (
  AD_FUNC_ARGS,
  FILE* out,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* map
);

} // namespace array_detect_ns
