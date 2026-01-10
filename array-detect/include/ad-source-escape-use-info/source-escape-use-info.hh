#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

// 前置声明
struct SourceUseInfo;

// ============================================================================
// 逃逸类型定义 (SourceUseEscapeKind)
// ============================================================================
// 描述源操作数使用时的逃逸类型

enum SourceUseEscapeKind {
  SU_ESCAPE_NONE = 0,           // 无逃逸
  SU_ESCAPE_RETURN,             // 通过返回值逃逸
  SU_ESCAPE_PARAMETER,          // 通过参数传递逃逸
  SU_ESCAPE_GLOBAL_STORE,       // 存储到全局变量
  SU_ESCAPE_HEAP_STORE,         // 存储到堆对象
  SU_ESCAPE_FIELD_STORE,        // 存储到对象字段
  SU_ESCAPE_INDIRECT_CALL,      // 通过间接调用逃逸
  SU_ESCAPE_VIRTUAL_CALL,       // 通过虚函数调用逃逸
  SU_ESCAPE_EXTERNAL_CALL,      // 传递给外部函数
  SU_ESCAPE_UNKNOWN             // 未知逃逸路径
};

// ============================================================================
// 源逃逸使用信息 (SourceEscapeUseInfo)
// ============================================================================
// 数据流：source-use-info -> source-escape-use-info
// 当 SourceUseInfo 被判定为逃逸时，产出详细的逃逸信息
//
// (source-escape-use-info
//   escape-kind   : escape-kind
//   escape-target : string
//   target-decl   : tree
//   is-safe-debug : bool)
// ============================================================================

struct SourceEscapeUseInfo {
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
// 输出：填充每个 wrapper 的 escape_use_info 字段
ArrayDetectErrorCode extractSourceEscapeUseInfo (
  AD_FUNC_ARGS,
  vec<field_analysis::Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses
);

// ============================================================================
// 辅助函数
// ============================================================================

// 判断是否为安全调试逃逸
bool isEscapeSafeDebug (
  AD_FUNC_ARGS,
  SourceUseInfo const* use_info
);

// 获取逃逸类型描述字符串
char const * getEscapeKindString (SourceUseEscapeKind kind);

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeUseInfo (
  SourceEscapeUseInfo const* info,
  FILE* output
);

} // namespace array_detect_ns
