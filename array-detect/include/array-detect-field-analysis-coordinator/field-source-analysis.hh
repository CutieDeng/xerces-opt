#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detect_ns {

// ============================================================================
// 源变量分析
// ============================================================================
// 从写入操作中提取源变量（SSA_NAME）
// 输入：map (field_decl -> list of FieldWriteCapture)
// 输出：map (field_decl -> list of source var (SSA_NAME))
// ============================================================================

// 从写入操作中提取源变量
// 返回值：ArrayDetectErrorCode
// 输入：field_decls - 字段声明列表
// 输入：field_writes - 对应的写入操作列表
// 输出：field_sources - 字段到源变量列表的映射（平行 vec）
ArrayDetectErrorCode extractSourceVariables (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<vec<FieldWriteCapture*>*> const &field_writes,
  vec<vec<tree>*> &field_sources
);

} // namespace array_detect_ns
