#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-use-info.hh"
#include "field-wrapper.hh"

// 前置声明
namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

// ============================================================================
// 源逃逸结论 (SourceEscapeConclude)
// ============================================================================
// 数据流：(listof wrapper-source-use-info-source-escape-use-info) -> source-escape-conclude
// 对单个写入的逃逸证据进行统计和判定
//
// (source-escape-conclude
//   total-escapes     : nat
//   safe-debug-escapes: nat
//   rejecting-escapes : nat          ; = total - safe-debug
//   has-rejecting     : bool)        ; rejecting > 0
// ============================================================================

struct SourceEscapeConclude {
  // 证据统计
  unsigned int total_escapes;           // 总逃逸数
  unsigned int safe_debug_escapes;      // 调试逃逸数
  unsigned int rejecting_escapes;       // 拒绝性逃逸数

  // 核心判定
  bool has_rejecting_evidence;          // rejecting_escapes > 0
  bool is_fully_analyzed;               // 是否完全分析
};

// ============================================================================
// 接口函数
// ============================================================================

// 生成源级逃逸结论
// 输入：uses (wrapper 列表，escape_use_info 已填充)
// 输出：SourceEscapeConclude*
ArrayDetectErrorCode synthesizeSourceEscapeConclude (
  AD_FUNC_ARGS,
  vec<field_analysis::Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses,
  SourceEscapeConclude** out_conclude
);

// ============================================================================
// Pipeline 接口
// ============================================================================

// 为所有写入生成源级逃逸结论
// 前置条件：escape_use_info 已由 extractAllSourceEscapeUseInfo 填充
ArrayDetectErrorCode synthesizeAllSourceEscapeConclude (
  AD_FUNC_ARGS,
  ::array_detector::ArrayDetector &detector,
  unsigned int &total_synthesized
);

// ============================================================================
// 调试逃逸识别
// ============================================================================

// 判断是否为已知安全调试函数
bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
);

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeConclude (
  SourceEscapeConclude const * result,
  FILE * output
);

} // namespace array_detect_ns
