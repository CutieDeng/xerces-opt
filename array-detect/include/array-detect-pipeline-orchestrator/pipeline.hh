#pragma once

// ============================================================================
// 向后兼容转发头文件
// ============================================================================
// 此头文件内容已移至 ad-pipeline 模块
// ============================================================================

// 由于 ad-pipeline 目录在 include 路径中，直接包含该模块的头文件
// 注意：实现已移至 ad-pipeline 模块

#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// 前向声明 ad-pipeline 中的类型和函数

// 分析阶段枚举
enum AnalysisPhase {
  PHASE_COLLECT_WRITES,
  PHASE_TRACE_SOURCES,
  PHASE_ANALYZE_USES,
  PHASE_SYNTHESIZE_ESCAPES,
  PHASE_ANALYZE_OWNERSHIP,
  PHASE_GENERATE_VERDICT,
  PHASE_COLLECT_ACCESSES,
  PHASE_ANALYZE_BOUNDS,
  PHASE_ASSOCIATE_CAPACITY,
  PHASE_AGGREGATE_RESULTS,
  PHASE_OUTPUT
};

struct PipelineConfig;
struct PipelineState;

// Pipeline 函数（实现在 ad-pipeline/pipeline.cc）
ArrayDetectErrorCode runPipeline (AD_FUNC_ARGS, ArrayDetector &detector);
ArrayDetectErrorCode runArrayDetectionPipeline (AD_FUNC_ARGS, ArrayDetector &detector);
ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS);

} // namespace array_detect_ns

// 注意：新代码请直接使用 ad-pipeline/pipeline.hh
// 此文件保留用于向后兼容

// 以下内容由 ad-pipeline/pipeline.hh 提供:
// - AnalysisPhase 枚举
// - PipelineConfig, PipelineState 结构
// - runPipeline(), runPhase(), getPipelineState() 函数
// - runArrayDetectionPipeline(), runArrayDetectorAnalysis() 向后兼容函数
