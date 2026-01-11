#pragma once

// ============================================================================
// ad-array-write-collect 模块
// ============================================================================
// 收集指定 (type, field) 对应的所有数组写入访问
//
// 数据流：
//   (type, pointer-field) -> (listof array-write-access)
//
// 场景：分析 arr->data[i] = x 形式的数组写入
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-detect-context-gcc.hh"

namespace array_detect_ns {

// ============================================================================
// 数组写入访问
// ============================================================================
// (array-write-access
//   stmt             : gimple                 ; 写入语句
//   base_pointer     : tree                   ; 基指针
//   index_expr       : tree                   ; 索引表达式
//   value_written    : tree                   ; 写入的值
//   element_type     : tree                   ; 元素类型
//   is_field_based   : bool                   ; 是否基于字段访问
//   containing_type  : tree                   ; 所属类型
//   pointer_field    : tree                   ; 指针字段
//   fn               : function*              ; 所属函数
//   bb               : basic_block            ; 所属基本块
//   location         : location_t)            ; 位置信息

struct ArrayWriteAccess {
  // === 访问信息 ===
  gimple* stmt;                    // 写入语句
  tree base_pointer;               // 基指针表达式
  tree index_expr;                 // 索引表达式
  tree value_written;              // 写入的值
  tree element_type;               // 元素类型

  // === 字段关联 ===
  bool is_field_based;             // 是否基于字段访问
  tree containing_type;            // 所属结构体类型
  tree pointer_field_decl;         // 指针字段 FIELD_DECL

  // === 上下文信息 ===
  function* fn;                    // 所属函数
  basic_block bb;                  // 所属基本块
  location_t location;             // 位置信息
};

// ============================================================================
// 函数声明
// ============================================================================

// 检查表达式是否为数组写入
bool isArrayWriteExpr (tree expr);

// 收集单个语句中的数组写入访问
// 输入：stmt, fn, target_type (可选过滤), target_field (可选过滤)
// 输出：result (如果匹配则返回 ArrayWriteAccess*)
ArrayDetectErrorCode collectArrayWriteAccess (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  tree target_type,
  tree target_field,
  ArrayWriteAccess** result
);

// 收集指定 (type, field) 的所有数组写入访问
// 输入：type, field
// 输出：(listof ArrayWriteAccess*)
ArrayDetectErrorCode collectAllArrayWriteAccesses (
  AD_FUNC_ARGS,
  tree type,
  tree field,
  vec<ArrayWriteAccess*, va_gc>** results
);

// 打印数组写入访问
void printArrayWriteAccess (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayWriteAccess* access
);

// 打印所有数组写入访问
void printAllArrayWriteAccesses (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ArrayWriteAccess*, va_gc>* accesses
);

} // namespace array_detect_ns
