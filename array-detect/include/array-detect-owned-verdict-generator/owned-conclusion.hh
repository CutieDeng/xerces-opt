#pragma once

#include "prelude.hh"
#include "context.hh"
#include "array-detector.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 字段 owned 结论判定
// ============================================================================

// 判定结果
enum OwnedConclusionVerdict {
  OWNED_YES,           // 可能是 owned 字段
  OWNED_NO,            // 不可能是 owned 字段
  OWNED_UNDETERMINED   // 无法确定（例如没有写入操作）
};

// 支持信息：记录一个支持 owned 的写入操作
struct OwnedSupportingEvidence {
  // === 基本信息 ===
  location_t location;                          // 写入位置
  gimple* stmt;                                 // 写入语句

  // === 源操作数信息 ===
  char const* source_description;               // 源描述信息

  // === 逃逸信息 ===
  unsigned int total_escapes;                   // 总逃逸次数
  unsigned int safe_debug_escapes;              // 安全调试逃逸次数

  // === 所有权转移信息（如果源是字段访问）===
  bool has_transfer_analysis;                   // 是否有所有权转移分析
  char const* transfer_verdict_str;             // 转移判定描述
};

// 拒绝信息：记录一个拒绝 owned 的写入操作
struct OwnedRejectingEvidence {
  // === 基本信息 ===
  location_t location;                          // 写入位置
  gimple* stmt;                                 // 写入语句

  // === 拒绝原因 ===
  char const* rejection_reason;                 // 拒绝原因描述

  // === 详细信息 ===
  // 源操作数问题
  bool has_invalid_source;                      // 源操作数不支持 owned
  char const* source_description;               // 源描述

  // 逃逸问题
  bool has_rejecting_escape;                    // 有拒绝性逃逸
  unsigned int rejecting_escapes;               // 拒绝性逃逸次数
  unsigned int total_escapes;                   // 总逃逸次数

  // 所有权转移问题
  bool has_transfer_issue;                      // 所有权转移问题
  char const* transfer_verdict_str;             // 转移判定描述
};

// 字段 owned 结论
struct FieldOwnedConclusion {
  // === 标识信息 ===
  tree type;                                    // 类型
  tree field_decl;                              // 字段声明
  char const* type_name;                        // 类型名称
  char const* field_name;                       // 字段名称

  // === 判定结果 ===
  OwnedConclusionVerdict verdict;               // 判定结果

  // === 统计信息 ===
  unsigned int total_writes;                    // 总写入操作数
  unsigned int supporting_writes_count;         // 支持 owned 的写入数
  unsigned int rejecting_writes_count;          // 拒绝 owned 的写入数

  // === 证据列表 ===
  vec<OwnedSupportingEvidence*, va_gc>* supporting_evidences;  // 支持证据列表
  vec<OwnedRejectingEvidence*, va_gc>* rejecting_evidences;    // 拒绝证据列表

  // === 结论描述 ===
  char const* conclusion_description;           // 结论描述
};

// ============================================================================
// 函数声明
// ============================================================================

// 分析所有字段的 owned 结论
ArrayDetectErrorCode analyzeAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<FieldOwnedConclusion*, va_gc>** out_conclusions
);

// 分析单个字段的 owned 结论
ArrayDetectErrorCode analyzeFieldOwnedConclusion (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData* field_data,
  FieldOwnedConclusion** out_conclusion
);

// 打印字段 owned 结论
void printFieldOwnedConclusion (
  AD_FUNC_ARGS,
  FILE* out,
  FieldOwnedConclusion* conclusion
);

// 打印所有字段 owned 结论
void printAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  FILE* out,
  vec<FieldOwnedConclusion*, va_gc>* conclusions
);

// 将结论写入 Racket datum 格式的结果文件
// 格式: ((type "TypeName")(field "FieldName")(result yes|no|maybe))
// 使用原子性写入以避免多进程并发问题
ArrayDetectErrorCode writeResultsToRacketDatum (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions
);

} // namespace array_detect_ns
