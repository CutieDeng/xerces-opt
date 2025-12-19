#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detector.hh"

namespace array_detect_ns {

// ============================================================================
// Array Detection Pipeline
// ============================================================================
// 简单的 pipeline 抽象，作为调用下层模块的组装器
// 组织三个主要步骤：提取字段信息 -> 分析信息 -> 输出信息
// ============================================================================

// Pipeline 函数：执行完整的数组检测流程
// 返回值：ArrayDetectErrorCode
// 输入输出：detector - ArrayDetector 对象（会被填充和更新）
// 
// 流程：
//   1. 提取字段信息：收集所有类型和字段
//   2. 分析信息：追踪字段赋值，判断是否为数组候选
//   3. 输出信息：生成并输出分析报告
ArrayDetectErrorCode runArrayDetectionPipeline(
  AD_FUNC_ARGS,
  ArrayDetector &detector
);

} // namespace array_detect_ns
