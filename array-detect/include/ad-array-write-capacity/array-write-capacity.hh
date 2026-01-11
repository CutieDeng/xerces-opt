#pragma once

// ============================================================================
// ad-array-write-capacity 模块
// ============================================================================
// 收集数组写入访问，分析写边界条件，寻找关联整数字段
//
// 此模块整合 ad-array-write-collect 和 ad-array-write-bound 模块功能
//
// 数据流：
//   pointer-field -> (listof array-write-access)
//   array-write-access -> (listof write-bound-condition)  ; 一对多
//   pointer-field, write-bound-condition -> (or write-capacity-evidence #f)
//
// 场景：分析 if (i < arr->cap) arr->data[i] = x 中 cap 字段关联
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-wrapper.hh"

// 包含子模块
#include "array-write-collect.hh"
#include "array-write-bound.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 函数声明
// ============================================================================

// 收集所有写容量证据
// 输入：TypeFieldAnalysisData
// 输出：填充 tfad->write_evidences 和 tfad->array_writes
ArrayDetectErrorCode collectAllWriteEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 打印所有写容量证据
void printAllWriteEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<WriteCapacityEvidence*, va_gc>* evidences
);

} // namespace array_detect_ns
