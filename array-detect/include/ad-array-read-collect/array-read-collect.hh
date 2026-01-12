#pragma once

// ============================================================================
// ad-array-read-collect 模块
// ============================================================================
// 全程序扫描，收集所有数组读取访问，直接填充到已有的 Wrapper 中
//
// 数据流：
//   m_type_field_writes -> 填充各 Wrapper 的 array_reads 字段
//
// 场景：扫描整个程序一次，收集所有 x = arr->data[i] 形式的数组读取
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
using ::field_analysis::Wrapper_ArrayReadAccess_ReadBoundConditions;

// ============================================================================
// 数组读取访问
// ============================================================================
// (array-read-access
//   stmt             : gimple                 ; 读取语句
//   base_pointer     : tree                   ; 基指针
//   index_expr       : tree                   ; 索引表达式
//   element_type     : tree                   ; 元素类型
//   is_field_based   : bool                   ; 是否基于字段访问
//   containing_type  : tree                   ; 所属类型
//   pointer_field    : tree                   ; 指针字段
//   fn               : function*              ; 所属函数
//   bb               : basic_block            ; 所属基本块
//   location         : location_t)            ; 位置信息

struct ArrayReadAccess {
  // === 访问信息 ===
  gimple* stmt;                    // 读取语句
  tree base_pointer;               // 基指针表达式
  tree index_expr;                 // 索引表达式
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

// 检查表达式是否为数组读取
bool isArrayReadExpr (tree expr);

// 收集单个语句中的数组读取访问（内部函数）
// 输入：stmt, fn
// 输出：result (如果是数组读取则返回 ArrayReadAccess*)
ArrayDetectErrorCode collectArrayReadAccessFromStmt (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  ArrayReadAccess** result
);

// 扫描整个程序，收集所有数组读取访问，直接填充到 Wrapper 的 array_reads 字段
// 输入：type_field_map - 现有的 (type, field) -> Wrapper hashmap
// 输出：直接填充各 Wrapper 的 array_reads 字段
ArrayDetectErrorCode scanAllArrayReadAccesses (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* type_field_map
);

// 打印数组读取访问
void printArrayReadAccess (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayReadAccess* access
);

// 打印特定 (type, field) 的数组读取访问列表
void printArrayReadAccessesForField (
  AD_FUNC_ARGS,
  FILE* out,
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>* wrappers
);

} // namespace array_detect_ns
