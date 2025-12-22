#pragma once

#include "gcc-common.hh"
#include "state.hh"
#include "prelude.hh"

namespace array_detect_ns {

// ============================================================================
// 数据结构：字段写入捕获（Pipeline 第一步输出）
// ============================================================================
// Pipeline 第一步：只抓取字段写入信息，不向下解析
// 垃圾回收：所有指针字段使用 ggc_alloc/ggc_strdup 分配，由 GCC 自动管理
// ============================================================================

struct FieldWriteCapture {
  // 核心信息：类型和字段
  tree type;                    // 类型（TYPE_MAIN_VARIANT，GCC 内部管理）
  tree field_decl;              // 字段声明（FIELD_DECL，GCC 内部管理）
  
  // 上下文信息：函数和基本块
  tree function_decl;           // 函数声明（GCC 内部管理）
  basic_block bb;               // 基本块（GCC 内部管理）
  const char* function_name;    // 所在函数名（ggc_strdup 分配，用于调试）
  
  // GIMPLE 语句信息
  gimple* stmt;                 // GIMPLE_ASSIGN 语句（GCC 内部管理）
  tree lhs;                     // 左值表达式（MEM，GCC 内部管理）
  tree rhs;                     // 右值表达式（SSA_NAME，GCC 内部管理）
  
  // 源码位置
  location_t location;          // 源码位置（GCC 内部管理）
  
  // 通用用途功能指针：用于存储有意义的结果（由后续阶段分配和管理）
  void* aux;
  
  // 调试和辅助字段
  int bb_index;                 // 基本块索引（用于调试）
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
  vec<FieldWriteCapture*>* write_ops; // 字段写入捕获列表（ggc_alloc<vec<...>>() 分配）
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
