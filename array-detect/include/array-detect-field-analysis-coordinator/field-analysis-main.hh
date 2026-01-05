#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// ============================================================================
// 主分析入口
// ============================================================================
// 整合所有分析阶段，逐个函数进行分析，最终判定字段是否为内存持有者
// ============================================================================

// 执行完整的字段分析流程
// 返回值：ArrayDetectErrorCode
// 输入：detector - ArrayDetector 对象（包含字段列表）
// 输出：field_decls - 字段声明列表
// 输出：field_results - 对应的分析结果列表
ArrayDetectErrorCode performFieldAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector const &detector,
  vec<tree> &field_decls,
  vec<FieldAnalysisResult*> &field_results
);

// 更新 FieldInfo 的 is_array_candidate 字段
// 返回值：ArrayDetectErrorCode
// 输入：field_results - 分析结果列表
// 输入输出：detector - ArrayDetector 对象
ArrayDetectErrorCode updateFieldInfoFromResults (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<FieldAnalysisResult*> const &field_results,
  ArrayDetector &detector
);

} // namespace array_detect_ns
