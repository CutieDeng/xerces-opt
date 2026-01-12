#pragma once

// ============================================================================
// ad-read-group 模块
// ============================================================================
// 批量分组 read 容量证据
//
// 数据流：
//   Wrapper.array_reads -> 分析 + 按 integer-field 分组
//
// 场景：从 Wrapper 的 array_reads 字段读取数组读取，分析并按整数字段分组证据
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-read-collect.hh"
#include "array-read-bound.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 函数声明
// ============================================================================

// 从 Wrapper 的 array_reads 中提取并分组证据（按 integer-field 分组）
// 输入：read_wrappers (Wrapper 的 array_reads 字段)
// 输出：ReadEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
ArrayDetectErrorCode groupReadEvidencesForField (
  AD_FUNC_ARGS,
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>* read_wrappers,
  ReadEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（从 Wrapper 的 array_reads 读取）
ArrayDetectErrorCode groupReadEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* field_wrapper
);

// 从分组结果中提取扁平列表（用于需要遍历所有证据的场景）
ArrayDetectErrorCode extractFlatReadEvidences (
  AD_FUNC_ARGS,
  ReadEvidencesByIntegerFieldMap* evidences_map,
  vec<ReadCapacityEvidence*, va_gc>** out_flat_list
);

// 查找特定整数字段的证据列表
vec<ReadCapacityEvidence*, va_gc>* findReadEvidencesForIntegerField (
  ReadEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
);

// 打印分组结果
void printReadEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  ReadEvidencesByIntegerFieldMap* evidences_map
);

} // namespace array_detect_ns
