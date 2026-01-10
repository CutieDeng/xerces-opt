#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "source-use.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

// ============================================================================
// 逃逸使用信息 (EscapedUseInfo)
// ============================================================================
// 数据流：write-original-source-use -> escaped-write-original-source-use
// 当 SourceUseInfo 被判定为逃逸时，产出详细的逃逸信息
//
// (escaped-write-original-source-use
//   escape-kind   : escape-kind
//   escape-target : string
//   target-decl   : tree
//   is-safe-debug : bool)
// ============================================================================

struct EscapedUseInfo {
  SourceUseEscapeKind escape_kind;   // 逃逸类型
  char const* escape_target;          // 逃逸目标描述（函数名/字段名等）
  tree target_decl;                   // 逃逸目标声明（如有）
  bool is_safe_debug;                 // 是否为安全调试逃逸
};

// ============================================================================
// 接口函数
// ============================================================================

// 提取逃逸使用信息
// 输入：wrapper 列表（已填充 use_info）
// 输出：填充每个 wrapper 的 escaped_info 字段
ArrayDetectErrorCode extractEscapedUses (
  AD_FUNC_ARGS,
  vec<field_analysis::Wrapper_SourceUseInfo_Escaped*, va_gc>* uses
);

// ============================================================================
// 辅助函数
// ============================================================================

// 判断是否为安全调试逃逸
bool isEscapeSafeDebug (
  AD_FUNC_ARGS,
  SourceUseInfo const* use_info
);

// ============================================================================
// 调试输出
// ============================================================================

void printEscapedUseInfo (
  EscapedUseInfo const* info,
  FILE* output
);

} // namespace array_detect_ns
