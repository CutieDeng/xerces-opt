#pragma once

#include "array-detect-gcc/gcc-common.hh"

// ============================================================================
// 字段信息结构 - 垃圾回收说明
// ============================================================================
// 所有指针字段都使用 ggc_alloc 分配，由 GCC 垃圾回收系统自动管理
// 所有字符串使用 ggc_strdup 分配，由 GCC 垃圾回收系统自动管理
// 所有 vec 容器使用 ggc_alloc<vec<T>>() 分配，由 GCC 垃圾回收系统自动管理
// ============================================================================

// 单个赋值操作的详细信息
// 垃圾回收：所有字段由 GCC 自动管理
struct AssignmentDetail {
  const char* source;              // 赋值来源（ggc_strdup 分配）
  bool is_call;                    // 是否是函数调用
  int tree_code;                   // 右值表达式的树代码
  const char* location_info;       // 位置信息（ggc_strdup 分配）
  const char* rhs_description;     // 右值表达式描述（ggc_strdup 分配）
};

// 函数级别的赋值信息
// 垃圾回收：所有字段由 GCC 自动管理
struct FunctionAssignment {
  const char* function_name;      // 函数名称（ggc_strdup 分配）
  tree function_decl;              // 函数声明（GCC 内部管理）
  const char* function_id;        // 函数唯一标识字符串（ggc_strdup 分配）
  int assignment_count;            // 该函数中该字段的赋值次数
  vec<const char*>* sources;       // 赋值来源向量（ggc_alloc<vec<...>>() 分配）
  vec<AssignmentDetail*>* assignment_details; // 详细赋值信息向量（ggc_alloc<vec<...>>() 分配）
};

struct FieldInfo {
  const char* field_name;         // 字段名称（ggc_strdup 分配）
  const char* containing_type;    // 包含该字段的类型名称（ggc_strdup 分配）
  tree field_decl;                // 字段声明（GCC 内部管理）
  tree containing_type_tree;      // 包含该字段的类型树（GCC 内部管理）
  bool is_pointer;                // 是否是指针类型
  bool is_array_candidate;        // 是否是数组候选
  int source_count;               // 总来源数量（所有函数）
  vec<const char*>* sources;       // 赋值来源向量（ggc_alloc<vec<...>>() 分配）
  vec<const char*>* conflicting_assigns; // 冲突赋值向量（ggc_alloc<vec<...>>() 分配）
  vec<FunctionAssignment*>* function_assignments; // 函数级赋值向量（ggc_alloc<vec<...>>() 分配）
};
