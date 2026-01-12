#pragma once

// ============================================================================
// ad-write-group 模块
// ============================================================================
// 批量分组 write 容量证据
//
// 数据流：
//   Wrapper.array_writes -> 分析 + 按 integer-field 分组
//
// 场景：从 Wrapper 的 array_writes 字段读取数组写入，分析并按整数字段分组证据
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "array-write-collect.hh"
#include "array-write-bound.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 函数声明
// ============================================================================

// 从 Wrapper 的 array_writes 中提取并分组证据（按 integer-field 分组）
// 输入：write_wrappers (Wrapper 的 array_writes 字段)
// 输出：WriteEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
ArrayDetectErrorCode groupWriteEvidencesForField (
  AD_FUNC_ARGS,
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>* write_wrappers,
  WriteEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（从 Wrapper 的 array_writes 读取）
ArrayDetectErrorCode groupWriteEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* field_wrapper
);

// 从分组结果中提取扁平列表（用于需要遍历所有证据的场景）
ArrayDetectErrorCode extractFlatWriteEvidences (
  AD_FUNC_ARGS,
  WriteEvidencesByIntegerFieldMap* evidences_map,
  vec<WriteCapacityEvidence*, va_gc>** out_flat_list
);

// 查找特定整数字段的证据列表
vec<WriteCapacityEvidence*, va_gc>* findWriteEvidencesForIntegerField (
  WriteEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
);

// 打印分组结果
void printWriteEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  WriteEvidencesByIntegerFieldMap* evidences_map
);

} // namespace array_detect_ns
