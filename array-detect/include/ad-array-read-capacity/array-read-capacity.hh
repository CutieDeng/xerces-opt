#pragma once

// ============================================================================
// ad-array-read-capacity 模块 (兼容层)
// ============================================================================
// 此模块现已拆分为两个子模块：
//   - ad-array-read-collect: 收集数组读取访问
//   - ad-array-read-bound: 分析边界条件
//
// 此头文件保留作为兼容层，重导出新模块的类型和函数
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-wrapper.hh"
#include "array-read-collect.hh"
#include "array-read-bound.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 兼容函数：收集所有读容量证据
// ============================================================================
// 此函数保留用于向后兼容，内部使用新的分模块实现

ArrayDetectErrorCode collectAllReadEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// ============================================================================
// 打印函数：保留用于输出
// ============================================================================

void printAllReadEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ReadCapacityEvidence*, va_gc>* evidences
);

} // namespace array_detect_ns
