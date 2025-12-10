#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"
#include "array-detector.hh"

namespace array_detect_ns {

// ============================================================================
// 字段分析集成模块
// ============================================================================
// 整合所有分析阶段，逐个函数进行分析
// ============================================================================

// 分析单个函数的所有字段
// 返回值：ArrayDetectErrorCode
// 输入：fn - 函数对象
// 输入：function_name - 函数名
// 输入：function_decl - 函数声明
// 输出：function_result - 函数分析结果
ArrayDetectErrorCode analyzeFunctionFields(
  AD_FUNC_ARGS,
  function* fn,
  const char* function_name,
  tree function_decl,
  FunctionAnalysisResult &function_result
);

// 整合所有函数的分析结果，判定字段是否为内存持有者
// 返回值：ArrayDetectErrorCode
// 输入：all_function_results - 所有函数的分析结果列表
// 输出：field_decls - 字段声明列表
// 输出：field_results - 对应的最终分析结果列表
ArrayDetectErrorCode integrateFunctionResults(
  AD_FUNC_ARGS,
  vec<FunctionAnalysisResult*> const &all_function_results,
  vec<tree> &field_decls,
  vec<FieldAnalysisResult*> &field_results
);

} // namespace array_detect_ns
