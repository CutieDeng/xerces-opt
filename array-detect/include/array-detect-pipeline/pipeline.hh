#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

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

// 顶层入口：创建检测器并执行分析
// 返回值：ArrayDetectErrorCode
// 这个函数负责创建 ArrayDetector，初始化，调用 pipeline，然后清理
// 这是插件的主要入口点
ArrayDetectErrorCode runArrayDetectorAnalysis(AD_FUNC_ARGS);

} // namespace array_detect_ns

