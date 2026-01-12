#pragma once

// ============================================================================
// ad-array-write-collect 模块
// ============================================================================
// 全程序扫描，收集所有数组写入访问，直接填充到已有的 Wrapper 中
//
// 数据流：
//   m_type_field_writes -> 填充各 Wrapper 的 array_writes 字段
//
// 场景：扫描整个程序一次，收集所有 arr->data[i] = x 形式的数组写入
// 直接利用已有的 m_type_field_writes hashmap，将 access 填充到对应 Wrapper
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-detect-context-gcc.hh"
#include "array-detector.hh"

namespace array_detect_ns {

using ::array_detector::TypeFieldKey;
using ::array_detector::TypeFieldHashMapTraits;
using ::array_detector::TypeFieldAnalysisData;
using ::field_analysis::Wrapper_ArrayWriteAccess_WriteBoundConditions;

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

// 收集单个语句中的数组写入访问（内部函数）
// 输入：stmt, fn
// 输出：result (如果是数组写入则返回 ArrayWriteAccess*)
ArrayDetectErrorCode collectArrayWriteAccessFromStmt (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  ArrayWriteAccess** result
);

// 扫描整个程序，收集所有数组写入访问，直接填充到 Wrapper 的 array_writes 字段
// 输入：type_field_map - 现有的 (type, field) -> Wrapper hashmap
// 输出：直接填充各 Wrapper 的 array_writes 字段
ArrayDetectErrorCode scanAllArrayWriteAccesses (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* type_field_map
);

// 打印数组写入访问
void printArrayWriteAccess (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayWriteAccess* access
);

// 打印特定 (type, field) 的数组写入访问列表
void printArrayWriteAccessesForField (
  AD_FUNC_ARGS,
  FILE* out,
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>* wrappers
);

} // namespace array_detect_ns
