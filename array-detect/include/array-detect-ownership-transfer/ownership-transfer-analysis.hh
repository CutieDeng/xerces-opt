#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"
#include "array-detector.hh"
#include "field-source-variant.hh"

namespace array_detect_ns {

// ============================================================================
// 所有权转移分析模块 (Ownership Transfer Analysis)
// ============================================================================
// 分析字段复制情形（如 a.ptr = b.ptr）中源字段是否被销毁，
// 以判断这是"所有权转移"还是"所有权共享"
// ============================================================================

// ============================================================================
// 销毁类型定义
// ============================================================================

enum InvalidationKind {
  INVALIDATION_NONE = 0,           // 无销毁
  INVALIDATION_NULL_ASSIGN,        // 赋值为 NULL
  INVALIDATION_CLOBBER_FIELD,      // 字段 clobber
  INVALIDATION_CLOBBER_OBJECT,     // 对象 clobber
  INVALIDATION_OTHER_ASSIGN        // 其他赋值（覆盖）
};

// ============================================================================
// 销毁点信息
// ============================================================================

struct InvalidationPoint {
  gimple* stmt;                    // 销毁语句
  InvalidationKind kind;           // 销毁类型
  location_t location;             // 源码位置
  basic_block bb;                  // 所在基本块
  char const* description;         // 销毁描述（调试用）
};

// ============================================================================
// 所有权转移结论
// ============================================================================

enum OwnershipTransferVerdict {
  TRANSFER_CERTAIN,                // 必然转移（所有路径都有销毁点）
  TRANSFER_IMPOSSIBLE,             // 不可能转移（没有路径有销毁点）
  TRANSFER_CONDITIONAL,            // 条件转移（某些路径有销毁点）
  TRANSFER_NOT_APPLICABLE          // 不适用（非字段访问源）
};

// ============================================================================
// 所有权转移分析结果
// ============================================================================

struct OwnershipTransferAnalysisResult {
  // === 基本信息 ===
  tree source_field;                              // 源字段（b.ptr）
  tree source_object;                             // 源对象（b）
  tree source_field_decl;                         // 源字段声明
  char const* source_field_name;                  // 源字段名
  gimple* transfer_stmt;                          // 转移语句（a.ptr = b.ptr）
  location_t transfer_location;                   // 转移位置

  // === 销毁点信息 ===
  vec<InvalidationPoint*, va_gc>* invalidation_points; // 所有销毁点

  // === 分析结论 ===
  OwnershipTransferVerdict verdict;               // 转移结论

  // === 路径统计 ===
  unsigned int paths_with_invalidation;           // 有销毁点的路径数
  unsigned int paths_without_invalidation;        // 无销毁点的路径数
  unsigned int total_exit_paths;                  // 总出口路径数

  // === 元数据 ===
  bool is_analyzed;                               // 是否已分析
  char const* verdict_description;                // 结论描述
};

// ============================================================================
// 核心分析函数
// ============================================================================

// 分析单个字段写入操作的所有权转移情况
// 输入：write_capture - 字段写入捕获信息
//       source_info - 源操作数信息（需要是字段访问类型）
// 输出：result - 所有权转移分析结果
ArrayDetectErrorCode analyzeOwnershipTransfer (
  AD_FUNC_ARGS,
  array_detect_ns::FieldWriteCapture* write_capture,
  array_detector::FieldSourceInfo* source_info,
  OwnershipTransferAnalysisResult*& result
);

// 分析所有字段写入操作的所有权转移
// 输入：detector - ArrayDetector 对象
// 输出：total_analyzed - 分析的总数
//       total_transfers - 发现的转移数
ArrayDetectErrorCode analyzeAllOwnershipTransfers (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_certain_transfers
);

// ============================================================================
// 辅助函数
// ============================================================================

// 查找从当前语句到函数出口的所有销毁点
ArrayDetectErrorCode findInvalidationPoints (
  AD_FUNC_ARGS,
  gimple* start_stmt,
  basic_block start_bb,
  tree source_field,
  tree source_object,
  vec<InvalidationPoint*, va_gc>*& invalidation_points
);

// 判断语句是否为销毁语句
ArrayDetectErrorCode isInvalidationStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  tree source_field,
  tree source_object,
  InvalidationKind& kind
);

// 分析从当前基本块到函数出口的路径
ArrayDetectErrorCode analyzePathsToExit (
  AD_FUNC_ARGS,
  basic_block start_bb,
  vec<InvalidationPoint*, va_gc>* invalidation_points,
  unsigned int& paths_with_invalidation,
  unsigned int& paths_without_invalidation,
  unsigned int& total_paths
);

// 打印所有权转移分析结果
void printOwnershipTransferResult (
  AD_FUNC_ARGS,
  OwnershipTransferAnalysisResult* result
);

} // namespace array_detect_ns
