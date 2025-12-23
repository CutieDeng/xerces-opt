#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detect_ns {

// ============================================================================
// 内存持有者判定
// ============================================================================
// 综合所有分析结果，判定字段是否为内存持有者
// 输入：map (field_decl -> list of FieldWriteCapture)
// 输入：map (field_decl -> list of EscapeSite)
// 输入：map (field_decl -> list of SourceOperation)
// 输出：map (field_decl -> FieldAnalysisResult)
// ============================================================================

// 判定字段是否为内存持有者
// 返回值：ArrayDetectErrorCode
// 输入：field_decl - 字段声明
// 输入：write_ops - 写入操作列表
// 输入：escape_sites - 逃逸位置列表
// 输入：source_ops - 源操作列表
// 输出：result - 分析结果
ArrayDetectErrorCode determineMemoryOwner (
  AD_FUNC_ARGS,
  tree field_decl,
  vec<FieldWriteCapture*> const &write_ops,
  vec<EscapeSite*> const &escape_sites,
  vec<SourceOperation*> const &source_ops,
  FieldAnalysisResult &result
);

// 从源变量中提取源操作（函数调用）
// 返回值：ArrayDetectErrorCode
// 输入：source_vars - 源变量列表（SSA_NAME）
// 输出：source_ops - 源操作列表
ArrayDetectErrorCode extractSourceOperations (
  AD_FUNC_ARGS,
  vec<tree> const &source_vars,
  vec<SourceOperation*> &source_ops
);

} // namespace array_detect_ns
