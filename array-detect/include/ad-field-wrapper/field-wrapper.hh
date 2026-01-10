#pragma once

// ============================================================================
// Field 分析 Wrapper 数据结构
// ============================================================================
//
// 三层 Wrapper 结构（保守扁平化：直接嵌入子模块结果类型）：
//
// 层级2：使用分析级 Wrapper (per write 的详细使用分析)
//   (wrapper-source-use-escaped-use
//     all-uses       : (listof write-original-source-use)
//     escaped-result : escaped-use-result)
//
// 层级1：写入级 Wrapper (per write operation)
//   (wrapper-write-info-write-source-source-escape-conclude
//     write-info     : field-write-info
//     write-source   : write-original-source
//     escape-conclude: source-escape-conclude
//     uses           : (listof wrapper-source-use-escaped-use*))
//
// 层级3：字段级 Wrapper (per field)
//   (wrapper-field-escape-conclude-ownership-conclude
//     type             : tree
//     field-decl       : tree
//     writes           : (listof wrapper-write-info-write-source*)
//     escape-conclude  : field-escape-conclude
//     ownership-conclude: ownership-conclude)
//
// ============================================================================

#include "gcc-common.hh"
#include "state.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "prelude.hh"

// ============================================================================
// 前置声明：各子模块结果类型
// ============================================================================

namespace array_detect_ns {
  struct FieldWriteInfo;
  struct SourceUseInfo;
  struct EscapedUseResult;
  struct SourceEscapeConclude;
  struct FieldEscapeConclude;
  struct OwnershipConclude;
}

namespace array_detector {
  struct WriteOriginalSource;
}

// ============================================================================
// Wrapper 定义
// ============================================================================

namespace field_analysis {

// ----------------------------------------------------------------------------
// 层级2：使用分析级 Wrapper
// ----------------------------------------------------------------------------

struct Wrapper_SourceUse_EscapedUse {
  vec<::array_detect_ns::SourceUseInfo>* all_uses;
  ::array_detect_ns::EscapedUseResult* escaped_result;
};

// ----------------------------------------------------------------------------
// 层级1：写入级 Wrapper
// ----------------------------------------------------------------------------

struct Wrapper_WriteInfo_WriteSource_SourceEscapeConclude {
  ::array_detect_ns::FieldWriteInfo* write_info;
  ::array_detector::WriteOriginalSource* write_source;
  ::array_detect_ns::SourceEscapeConclude* escape_conclude;
  vec<Wrapper_SourceUse_EscapedUse*, va_gc>* uses;
};

// ----------------------------------------------------------------------------
// 层级3：字段级 Wrapper
// ----------------------------------------------------------------------------

struct Wrapper_FieldEscapeConclude_OwnershipConclude {
  tree type;
  tree field_decl;
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude*, va_gc>* writes;
  ::array_detect_ns::FieldEscapeConclude* escape_conclude;
  ::array_detect_ns::OwnershipConclude* ownership_conclude;
};

} // namespace field_analysis
