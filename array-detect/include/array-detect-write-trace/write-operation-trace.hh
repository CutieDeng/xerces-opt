#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"
#include "array-detector.hh"
#include "field-source-variant.hh"

namespace array_detector {

using namespace ::array_detect_ns;

class ArrayDetector;

// ============================================================================
// 字段写入操作追踪
// ============================================================================
// 追踪字段赋值，分析字段的赋值来源，提取来源信息
// ============================================================================

// 追踪字段赋值：分析字段赋值来源
// 语义：遍历 hash_map 中的所有字段写入操作，提取每个写入操作的来源信息
// 前置条件：字段已通过 collectTypesAndFields 收集到 detector.m_type_field_writes
// 输入：detector - ArrayDetector 对象（包含 m_type_field_writes hash_map）
// 输出：在 FieldWriteCapture 的 next 字段中存储 FieldSourceInfo*（由后续阶段使用）
ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS);

// ============================================================================
// 字段来源信息提取（内部函数，高内聚）
// ============================================================================

// 从 RHS 表达式提取来源信息
// 输入：rhs - 右值表达式（tree，通常是 SSA_NAME 或其他表达式）
//       stmt - GIMPLE_ASSIGN 语句（用于上下文）
//       location - 源码位置
// 输出：source_info - 提取的来源信息（已分配内存，使用 ggc_alloc）
ArrayDetectErrorCode extractSourceFromRhs(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree rhs,
  gimple* stmt,
  location_t location,
  FieldSourceInfo** out_source_info
);

// 从函数调用提取来源信息
// 输入：call_stmt - GIMPLE_CALL 语句
//       return_ssa - 返回值的 SSA_NAME（如果存在）
// 输出：source_info - 提取的来源信息（已分配内存，使用 ggc_alloc）
ArrayDetectErrorCode extractSourceFromCall(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  gimple* call_stmt,
  tree return_ssa,
  FieldSourceInfo** out_source_info
);

// 从变量提取来源信息
// 输入：ssa_name - SSA_NAME
//       location - 源码位置
// 输出：source_info - 提取的来源信息（已分配内存，使用 ggc_alloc）
ArrayDetectErrorCode extractSourceFromVariable(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree ssa_name,
  location_t location,
  FieldSourceInfo** out_source_info
);

} // namespace array_detector
