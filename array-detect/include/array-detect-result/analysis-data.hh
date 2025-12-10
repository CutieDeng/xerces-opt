#pragma once

#include "gcc-common.hh"
#include "state.hh"
#include "prelude.hh"

namespace array_detect_ns {

// ============================================================================
// 数据结构：写入操作信息
// ============================================================================
// 垃圾回收：所有指针字段使用 ggc_alloc/ggc_strdup 分配，由 GCC 自动管理
// ============================================================================

struct WriteOperation {
  gimple* stmt;                 // GIMPLE 赋值语句（GCC 内部管理）
  tree rhs_value;              // 右值表达式（SSA_NAME，GCC 内部管理）
  const char* function_name;    // 所在函数名（ggc_strdup 分配）
  tree function_decl;           // 函数声明（GCC 内部管理）
  location_t location;         // 源码位置（GCC 内部管理）
  bool is_from_call;           // 是否来自函数调用
  gimple* call_stmt;           // 如果是调用，记录调用语句（GCC 内部管理）
  const char* rhs_description; // 右值描述（ggc_strdup 分配）
};

// ============================================================================
// 数据结构：逃逸位置信息
// ============================================================================

enum EscapeType {
  ESCAPE_WRITE,      // 写入其他位置
  ESCAPE_ARGUMENT,  // 作为函数参数传递
  ESCAPE_COMPUTE,   // 参与计算
  ESCAPE_RETURN     // 作为返回值
};

struct EscapeSite {
  gimple* stmt;                 // 发生逃逸的语句（GCC 内部管理）
  EscapeType escape_type;      // 逃逸类型
  const char* function_name;    // 所在函数名（ggc_strdup 分配）
  location_t location;         // 源码位置（GCC 内部管理）
  const char* description;     // 逃逸描述（ggc_strdup 分配）
};

// ============================================================================
// 数据结构：源操作信息（函数调用）
// ============================================================================

enum CallType {
  CALL_VIRTUAL,     // C++ 虚函数调用
  CALL_DIRECT,      // 直接函数调用
  CALL_INDIRECT,    // 间接函数调用（函数指针）
  CALL_UNKNOWN      // 未知类型调用
};

struct SourceOperation {
  gimple* call_stmt;           // GIMPLE_CALL 语句（GCC 内部管理）
  CallType call_type;          // 调用类型
  const char* function_name;   // 函数名（mangled，ggc_strdup 分配）
  tree return_value_ssa;       // 返回值的 SSA_NAME（GCC 内部管理）
  tree vtable_ref;             // 虚表引用（如果是虚函数，GCC 内部管理）
  const char* signature;       // 调用签名（ggc_strdup 分配）
  location_t location;         // 调用位置（GCC 内部管理）
};

// ============================================================================
// 数据结构：字段分析结果
// ============================================================================

struct FieldAnalysisResult {
  tree field_decl;                    // 字段声明（GCC 内部管理）
  vec<WriteOperation*>* write_ops;   // 写入操作列表（ggc_alloc<vec<...>>() 分配）
  vec<tree>* source_vars;             // 源变量列表（SSA_NAME，ggc_alloc<vec<...>>() 分配）
  vec<EscapeSite*>* escape_sites;     // 逃逸位置列表（ggc_alloc<vec<...>>() 分配）
  vec<SourceOperation*>* sources;     // 源操作列表（ggc_alloc<vec<...>>() 分配）
  bool is_memory_owner;                // 是否为内存持有者
  const char* reason;                  // 判定原因（ggc_strdup 分配）
};

// ============================================================================
// 数据结构：函数级分析结果
// ============================================================================

// map: field_decl -> FieldAnalysisResult
// 使用 vec 配对实现映射关系
struct FunctionAnalysisResult {
  const char* function_name;           // 函数名（ggc_strdup 分配）
  tree function_decl;                  // 函数声明（GCC 内部管理）
  vec<tree>* field_decls;              // 字段声明列表
  vec<FieldAnalysisResult*>* field_results; // 对应的分析结果列表
};

} // namespace array_detect_ns
