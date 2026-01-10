#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "escaped-use.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

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

// ============================================================================
// 接口函数
// ============================================================================

// 生成源级逃逸结论
// 输入：escaped_uses (逃逸使用列表)
// 输出：直接写入 wrapper 成员地址
ArrayDetectErrorCode generateSourceEscapeConclude (
  AD_FUNC_ARGS,
  vec<field_analysis::FieldUsePoint const*>* escaped_uses,
  unsigned int* out_total_escapes,
  unsigned int* out_safe_debug_escapes,
  unsigned int* out_rejecting_escapes,
  bool* out_has_rejecting_evidence
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
  field_analysis::FieldUsePoint const * escape
);

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeConclude (
  SourceEscapeConclude const * result,
  FILE * output
);

} // namespace array_detect_ns
