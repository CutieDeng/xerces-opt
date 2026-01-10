#pragma once

// ============================================================================
// Field 分析 Wrapper 数据结构
// ============================================================================
//
// 三层 Wrapper 结构：
//
// 层级2：单个使用的 Wrapper (per source-use)
//   (wrapper-source-use-info-source-escape-use-info
//     use-info            : source-use-info
//     escape-use-info     : source-escape-use-info or #f)
//
// 层级1：写入级 Wrapper (per write operation)
//   (wrapper-write-info-write-source-source-escape-conclude-ownership-move
//     write-info      : field-write-info
//     write-source    : write-original-source
//     escape-conclude : source-escape-conclude
//     ownership-move  : ownership-move or #f
//     uses            : (listof wrapper-source-use-info-source-escape-use-info*))
//
// 层级3：字段级 Wrapper (per field)
//   (wrapper-field-escape-conclude-ownership-conclude
//     type              : tree
//     field-decl        : tree
//     writes            : (listof wrapper-write-info-write-source*)
//     escape-conclude   : field-escape-conclude
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
  struct SourceEscapeUseInfo;
  struct SourceEscapeConclude;
  struct FieldEscapeConclude;
  struct OwnershipConclude;
  struct OwnershipMove;
}

namespace array_detector {
  struct WriteOriginalSource;
}

// ============================================================================
// Wrapper 定义
// ============================================================================

namespace field_analysis {

// ----------------------------------------------------------------------------
// 层级2：单个使用的 Wrapper (per source-use)
// ----------------------------------------------------------------------------
// (wrapper-source-use-info-source-escape-use-info
//   use-info         : source-use-info           ; source-use-info 模块产出
//   escape-use-info  : source-escape-use-info or #f)  ; source-escape-use-info 模块产出

struct Wrapper_SourceUseInfo_SourceEscapeUseInfo {
  ::array_detect_ns::SourceUseInfo* use_info;
  ::array_detect_ns::SourceEscapeUseInfo* escape_use_info;  // 非逃逸时为 NULL
};

// ----------------------------------------------------------------------------
// 层级1：写入级 Wrapper
// ----------------------------------------------------------------------------
// (wrapper-write-info-write-source-source-escape-conclude-ownership-move
//   write-info      : field-write-info
//   write-source    : write-original-source
//   escape-conclude : source-escape-conclude
//   ownership-move  : ownership-move or #f
//   uses            : (listof wrapper-source-use-info-source-escape-use-info*))

struct Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove {
  ::array_detect_ns::FieldWriteInfo* write_info;
  ::array_detector::WriteOriginalSource* write_source;
  ::array_detect_ns::SourceEscapeConclude* escape_conclude;
  ::array_detect_ns::OwnershipMove* ownership_move;
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses;
};

// ----------------------------------------------------------------------------
// 层级3：字段级 Wrapper
// ----------------------------------------------------------------------------

struct Wrapper_FieldEscapeConclude_OwnershipConclude {
  tree type;
  tree field_decl;
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove*, va_gc>* writes;
  ::array_detect_ns::FieldEscapeConclude* escape_conclude;
  ::array_detect_ns::OwnershipConclude* ownership_conclude;
};

} // namespace field_analysis
