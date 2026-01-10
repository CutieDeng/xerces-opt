#pragma once

// ============================================================================
// ad-pipeline 模块
// ============================================================================
// 顶层控制流编排模块
// 定义分析阶段、配置、状态，协调各阶段执行
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// ============================================================================
// 分析阶段枚举
// ============================================================================

enum AnalysisPhase {
  PHASE_COLLECT_WRITES,     // 收集字段写入
  PHASE_TRACE_SOURCES,      // 追踪写入来源
  PHASE_ANALYZE_USES,       // 分析使用链
  PHASE_SYNTHESIZE_ESCAPES, // 合成逃逸证据
  PHASE_ANALYZE_OWNERSHIP,  // 分析所有权转移
  PHASE_GENERATE_VERDICT,   // 生成 owned 判定
  PHASE_COLLECT_ACCESSES,   // 收集数组访问
  PHASE_ANALYZE_BOUNDS,     // 分析边界条件
  PHASE_ASSOCIATE_CAPACITY, // 关联容量字段
  PHASE_AGGREGATE_RESULTS,  // 聚合结果
  PHASE_OUTPUT              // 输出结果
};

// ============================================================================
// Pipeline 配置
// ============================================================================

struct PipelineConfig {
  bool enable_ownership_analysis;
  bool enable_array_access;
  bool enable_debug_output;
  FILE* debug_file;
};

// ============================================================================
// Pipeline 状态
// ============================================================================

struct PipelineState {
  AnalysisPhase current_phase;
  unsigned int total_writes_collected;
  unsigned int total_sources_traced;
  unsigned int total_escapes_analyzed;
  unsigned int total_ownership_analyzed;
  unsigned int total_verdicts_generated;
};

// ============================================================================
// Pipeline 函数接口
// ============================================================================

// 执行完整 pipeline
ArrayDetectErrorCode runPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
);

// 获取 pipeline 状态
PipelineState* getPipelineState (AD_FUNC_ARGS);

// 顶层入口
ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS);

} // namespace array_detect_ns
