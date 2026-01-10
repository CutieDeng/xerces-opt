#pragma once

// ============================================================================
// Field 分析数据流总览
// ============================================================================
//
// 数据流变换：
//
// [写入级分析]
// collect-writes : field -> (listof FieldWriteInfo)
// trace-source   : FieldWriteInfo -> WriteSource
// analyze-uses   : WriteSource -> UseAnalysis
// conclude-escape: UseAnalysis -> EscapeConclude
// analyze-move   : FieldWriteInfo, WriteSource -> MoveAnalysis
//
// [字段级分析]
// conclude-field : (listof WriteAnalysis) -> FieldConclude
//
// 聚合结构：
// - WriteAnalysis : 单次写入的完整分析（聚合所有一对一关系）
// - FieldAnalysis : 字段级分析（聚合多个 WriteAnalysis）
// ============================================================================

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"

namespace array_detect_ns {

// ============================================================================
// 字段写入信息 (FieldWriteInfo)
// ============================================================================
// 数据流位置：field -> (listof field-write-info)
// 表示单个字段写入操作的 GIMPLE 层信息
//
// (field-write-info
//   type           : tree           ; TYPE_MAIN_VARIANT
//   field-decl     : tree           ; FIELD_DECL
//   stmt           : gimple*        ; GIMPLE_ASSIGN
//   lhs            : tree           ; 左值 (MEM_REF/COMPONENT_REF)
//   rhs            : tree           ; 右值 (SSA_NAME)
//   location       : location_t     ; 源码位置)
// ============================================================================

struct FieldWriteInfo {
  // 核心信息：类型和字段
  tree type;                    // 类型（TYPE_MAIN_VARIANT）
  tree field_decl;              // 字段声明（FIELD_DECL）

  // 上下文信息：函数和基本块
  tree function_decl;           // 函数声明
  basic_block bb;               // 基本块
  char const * function_name;   // 所在函数名（ggc_strdup）

  // GIMPLE 语句信息
  gimple * stmt;                // GIMPLE_ASSIGN 语句
  tree lhs;                     // 左值表达式
  tree rhs;                     // 右值表达式

  // 源码位置
  location_t location;          // 源码位置

  // 辅助指针：后续阶段填充 WriteOriginalSource*
  void * aux;

  // 调试字段
  int bb_index;                 // 基本块索引
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
  gimple * call_stmt;           // GIMPLE_CALL 语句（GCC 内部管理）
  CallType call_type;          // 调用类型
  char const * function_name;   // 函数名（mangled，ggc_strdup 分配）
  tree return_value_ssa;       // 返回值的 SSA_NAME（GCC 内部管理）
  tree vtable_ref;             // 虚表引用（如果是虚函数，GCC 内部管理）
  char const * signature;       // 调用签名（ggc_strdup 分配）
  location_t location;         // 调用位置（GCC 内部管理）
};

} // namespace array_detect_ns

// ============================================================================
// 收集函数声明
// ============================================================================
// 数据流：遍历编译单元 -> (type, field) -> (listof FieldWriteInfo)

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// ============================================================================
// 公开接口
// ============================================================================

// 主入口：收集编译单元中所有字段写入操作
// 语义：whole-program -> (mapof (type, field) (listof wrapper))
// 结果存入 detector.m_type_field_writes
ArrayDetectErrorCode collectAllFieldWrites (
  AD_FUNC_ARGS,
  ArrayDetector& detector
);

// 检查语句是否为字段写入
// 若是则返回 FieldWriteInfo，否则 result 为 NULL
ArrayDetectErrorCode collectAllFieldWrites_checkStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
);

} // namespace array_detect_ns
