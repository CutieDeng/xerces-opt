#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"
#include "source-escape-collection.hh"

namespace array_detector {

  class ArrayDetector;
  struct TypeFieldAnalysisData;

} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 逃逸综合分析模块 (Escape Synthesizer)
// ============================================================================
// 数据流关系：
// - (listof source-use) -> (listof escaped-use)       [EscapedUseResult]
// - source, uses, escaped-uses -> source-escape-conclude [SourceEscapeConclude]
// - field, (listof source) -> field-escape-conclude   [FieldEscapeConclude]
// ============================================================================

// ============================================================================
// 逃逸使用结果 (EscapedUseResult)
// ============================================================================
// 数据流位置：(listof source-use) -> (listof escaped-use)
// 从 SourceUseResult.all_uses 中提取所有逃逸使用
//
// (escaped-use-result
//   source-operand : tree
//   escapes        : (listof use-info*)  ; 指向 all-uses 中逃逸元素
//   escape-count   : nat)
// ============================================================================

struct EscapedUseResult {
  tree source_operand;              // 源操作数
  gimple * source_stmt;             // 源语句

  vec<SourceUseInfo const *> * escapes;  // 所有逃逸使用
  unsigned int escape_count;        // 逃逸数量

  void * aux;
  void * original_write_info;       // 原始 FieldWriteInfo*
};

// 向后兼容别名
typedef EscapedUseResult EscapeExtractionResult;

// ============================================================================
// 源逃逸结论 (SourceEscapeConclude)
// ============================================================================
// 数据流位置：source, uses, escaped-uses -> source-escape-conclude
// 对单个 write-original-source 的逃逸证据进行统计和判定
//
// (source-escape-conclude
//   type              : tree
//   field-decl        : tree
//   write-location    : location_t
//   total-escapes     : nat
//   safe-debug-escapes: nat
//   rejecting-escapes : nat          ; = total - safe-debug
//   has-rejecting     : bool)        ; rejecting > 0
// ============================================================================

struct SourceEscapeConclude {
  // 写入位置标识
  tree type;
  tree field_decl;
  location_t write_location;

  // 证据统计
  unsigned int total_escapes;           // 总逃逸数
  unsigned int safe_debug_escapes;      // 调试逃逸数
  unsigned int rejecting_escapes;       // 拒绝性逃逸数

  // 核心判定
  bool has_rejecting_evidence;          // rejecting_escapes > 0

  void * aux;
  void * original_write_info;           // 原始 FieldWriteInfo*
};

// 向后兼容别名
typedef SourceEscapeConclude EscapeEvidenceResult;

// ============================================================================
// 字段逃逸结论 (FieldEscapeConclude)
// ============================================================================
// 数据流位置：field, (listof write-original-source) -> field-escape-conclude
// 汇总单个 (type, field) 的所有写入操作的逃逸信息
//
// (field-escape-conclude
//   type                    : tree
//   field-decl              : tree
//   total-field-writes      : nat
//   writes-with-rejecting   : nat
//   has-rejecting-evidence  : bool
//   all-source-concludes    : (listof source-escape-conclude*))
// ============================================================================

struct FieldEscapeConclude {
  // === 标识 ===
  tree type;
  tree field_decl;

  // === 字段写入操作统计 ===
  unsigned int total_field_writes;              // 总写入操作数
  unsigned int field_writes_with_escape;        // 有逃逸的写入数
  unsigned int field_writes_with_rejecting;     // 有拒绝证据的写入数
  unsigned int field_writes_without_analysis;   // 未分析的写入数

  // === 逃逸统计（聚合）===
  unsigned int total_escapes;             // 总逃逸数
  unsigned int safe_debug_escapes;        // 调试逃逸数
  unsigned int rejecting_escapes;         // 拒绝性逃逸数

  // === 来源类型分布 ===
  unsigned int source_function_call;      // 函数调用来源数
  unsigned int source_field_access;       // 字段访问来源数
  unsigned int source_constant;           // 常量来源数
  unsigned int source_computation;        // 计算来源数
  unsigned int source_phi;                // PHI 节点来源数
  unsigned int source_unknown;            // 未知来源数

  // === 核心判定 ===
  bool has_rejecting_evidence;            // 存在拒绝证据
  float rejection_ratio;                  // 拒绝比例

  // === 所有源级结论 ===
  vec<SourceEscapeConclude*> * all_source_concludes;
};

// 向后兼容别名
typedef FieldEscapeConclude TypeFieldEscapeSummary;

// ============================================================================
// 接口函数
// ============================================================================

// 汇总字段级逃逸结论
// field_data -> FieldEscapeConclude
ArrayDetectErrorCode summarizeFieldEscape (
  AD_FUNC_ARGS,
  array_detector::TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
);

// 向后兼容别名
inline ArrayDetectErrorCode summarizeTypeFieldEscapes (
  AD_FUNC_ARGS_DECL,
  array_detector::TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
) { return summarizeFieldEscape(AD_FUNC_ARGS_CALL, field_data, result); }

// 提取逃逸使用
// SourceUseResult -> EscapedUseResult
ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  SourceUseResult * use_result,
  EscapedUseResult * &result
);

// 向后兼容别名
inline ArrayDetectErrorCode extractEscapes (
  AD_FUNC_ARGS_DECL,
  SourceUseResult * use_result,
  EscapedUseResult * &result
) { return extractEscapedUses(AD_FUNC_ARGS_CALL, use_result, result); }

// 生成源级逃逸结论
// EscapedUseResult -> SourceEscapeConclude
ArrayDetectErrorCode generateSourceEscapeConclude (
  AD_FUNC_ARGS,
  EscapedUseResult * escaped_uses,
  tree type,
  tree field_decl,
  location_t write_location,
  SourceEscapeConclude * &result
);

// 向后兼容别名
inline ArrayDetectErrorCode generateEscapeEvidence (
  AD_FUNC_ARGS_DECL,
  EscapedUseResult * escaped_uses,
  tree type,
  tree field_decl,
  location_t write_location,
  SourceEscapeConclude * &result
) { return generateSourceEscapeConclude(AD_FUNC_ARGS_CALL, escaped_uses, type, field_decl, write_location, result); }

// 综合所有字段的逃逸信息
ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<SourceEscapeConclude*> * &evidence_results,
  unsigned int &total_synthesized
);

// ============================================================================
// 调试逃逸识别（唯一允许字符串匹配的场景）
// ============================================================================

// 判断是否为已知安全调试函数
// 仅在此函数中允许使用字符串模糊匹配
bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
);

// 判断逃逸是否为安全调试逃逸
// 仅当逃逸为函数调用且函数为调试函数时返回 true
bool isSafeDebugEscape (
  AD_FUNC_ARGS,
  SourceUseInfo const * escape
);

// ============================================================================
// 调试输出
// ============================================================================

void printEscapedUseResult (
  EscapedUseResult const * result,
  FILE * output
);

void printSourceEscapeConclude (
  SourceEscapeConclude const * result,
  FILE * output
);

// 向后兼容别名
inline void printEscapeExtractionResult (
  EscapedUseResult const * result,
  FILE * output
) { printEscapedUseResult(result, output); }

inline void printEscapeEvidenceResult (
  SourceEscapeConclude const * result,
  FILE * output
) { printSourceEscapeConclude(result, output); }

} // namespace array_detect_ns
