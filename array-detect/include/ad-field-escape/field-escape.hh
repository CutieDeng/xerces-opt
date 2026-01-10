#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-escape.hh"
#include "source-use.hh"
#include "escaped-use.hh"
#include "field-wrapper.hh"
#include "array-detector.hh"

namespace array_detector {
  class ArrayDetector;
  // TypeFieldAnalysisData is defined in type-field-hashmap-traits.hh which is
  // included after array-detector.hh. We use the full type name here.
} // namespace array_detector

namespace array_detect_ns {

// TypeFieldAnalysisData 从 array_detector 命名空间引入
using array_detector::TypeFieldAnalysisData;

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

// ============================================================================
// 接口函数
// ============================================================================

// 汇总字段级逃逸结论
// field_data -> FieldEscapeConclude
ArrayDetectErrorCode summarizeFieldEscape (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
);

// 向后兼容别名
inline ArrayDetectErrorCode summarizeTypeFieldEscapes (
  AD_FUNC_ARGS_DECL,
  TypeFieldAnalysisData * field_data,
  FieldEscapeConclude * &result
) { return summarizeFieldEscape(AD_FUNC_ARGS_CALL, field_data, result); }

// 综合所有字段的逃逸信息
ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<SourceEscapeConclude*> * &evidence_results,
  unsigned int &total_synthesized
);

} // namespace array_detect_ns
