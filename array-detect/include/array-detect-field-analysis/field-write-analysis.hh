#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"
#include "array-detector.hh"

namespace array_detect_ns {

// ============================================================================
// 函数级写入分析
// ============================================================================
// 分析单个函数中所有字段的写入操作
// 输出：map(field_decl -> list of FieldWriteCapture)
// ============================================================================

// 分析函数中的字段写入操作
// 返回值：ArrayDetectErrorCode
// 输出：通过 field_decls 和 field_writes 返回字段到写入操作列表的映射
//      使用两个平行的 vec 来模拟 map: field_decls[i] -> field_writes[i]
ArrayDetectErrorCode analyzeFunctionFieldWrites(
  AD_FUNC_ARGS,
  function* fn,
  const char* function_name,
  tree function_decl,
  vec<tree> &field_decls,
  vec<vec<FieldWriteCapture*>*> &field_writes
);

} // namespace array_detect_ns
