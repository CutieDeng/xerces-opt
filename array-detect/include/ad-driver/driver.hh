#pragma once

// ============================================================================
// ad-driver 模块
// ============================================================================
// 细节驱动逻辑模块
// 展开 FieldWriteAnalysisWrapper，驱动单个写入的完整分析流程
// 调用各子分析模块（8个数据结构模块）
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "source-use.hh"
#include "escaped-use.hh"
#include "source-escape.hh"
#include "ownership-move.hh"
#include "field-wrapper.hh"
#include "array-detector.hh"

namespace array_detect_ns {

using array_detector::ArrayDetector;
using array_detector::FieldWriteAnalysisRecord;
using array_detector::WriteOriginalSource;

// ============================================================================
// 写入分析驱动上下文
// ============================================================================
// 用于在驱动分析过程中传递中间结果

struct WriteAnalysisDriverContext {
  // 输入：字段写入信息
  FieldWriteInfo* write_info;

  // 阶段 1 输出：写入来源
  WriteOriginalSource* source;

  // 阶段 2 输出：使用分析
  SourceUseResult* use_result;

  // 阶段 3 输出：逃逸使用
  EscapedUseResult* escaped_uses;

  // 阶段 4 输出：源级逃逸结论
  SourceEscapeConclude* escape_conclude;

  // 阶段 5 输出：所有权转移分析（可选，仅当来源是字段访问时）
  OwnershipMoveResult* move_result;

  // 标记
  bool is_analyzed;
  bool has_error;
};

// ============================================================================
// 驱动函数接口
// ============================================================================

// 驱动单个写入的完整分析
// 从 FieldWriteInfo 开始，执行完整分析流程，生成 FieldWriteAnalysisWrapper
ArrayDetectErrorCode driveWriteAnalysis (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  FieldWriteAnalysisRecord*& result
);

// 驱动所有写入的分析（Pipeline 接口）
// 遍历 detector 中所有字段写入，执行完整分析
ArrayDetectErrorCode driveAllWriteAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector
);

// 驱动单个字段的所有写入分析
// 对指定 (type, field_decl) 的所有写入执行分析
ArrayDetectErrorCode driveFieldAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree type,
  tree field_decl,
  vec<FieldWriteAnalysisRecord*>*& records
);

// 展开 wrapper 获取各部分
// 从 FieldWriteAnalysisRecord 提取各分析阶段的结果
ArrayDetectErrorCode unwrapAnalysis (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  WriteAnalysisDriverContext& driver_ctx
);

// ============================================================================
// 辅助函数
// ============================================================================

// 初始化驱动上下文
void initDriverContext (WriteAnalysisDriverContext& ctx);

// 清理驱动上下文
void cleanupDriverContext (WriteAnalysisDriverContext& ctx);

// 打印驱动上下文（调试用）
void printDriverContext (
  AD_FUNC_ARGS,
  FILE* out,
  WriteAnalysisDriverContext const& driver_ctx
);

} // namespace array_detect_ns
