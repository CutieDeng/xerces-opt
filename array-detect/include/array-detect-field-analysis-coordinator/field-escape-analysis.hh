#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸分析
// ============================================================================
// 分析源变量的所有使用位置，检测是否逃逸
// 输入：map (field_decl -> list of source var)
// 输出：map (field_decl -> list of EscapeSite)
// ============================================================================

// 分析字段源变量的逃逸情况
// 返回值：ArrayDetectErrorCode
// 输入：field_decls - 字段声明列表
// 输入：field_sources - 对应的源变量列表
// 输入：target_field_index - 目标字段在列表中的索引
// 输出：escape_sites - 逃逸位置列表
ArrayDetectErrorCode analyzeFieldEscapes (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<vec<tree>*> const &field_sources,
  unsigned int target_field_index,
  vec<EscapeSite*> &escape_sites
);

// 追踪单个 SSA_NAME 的所有使用位置
// 返回值：ArrayDetectErrorCode
// 输入：ssa_name - 要追踪的 SSA_NAME
// 输入：target_field - 目标字段声明
// 输出：escape_sites - 逃逸位置列表
ArrayDetectErrorCode trackSSAUses (
  AD_FUNC_ARGS,
  tree ssa_name,
  tree target_field,
  vec<EscapeSite*> &escape_sites
);

} // namespace array_detect_ns
