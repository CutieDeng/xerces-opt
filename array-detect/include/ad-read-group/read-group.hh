#pragma once

// ============================================================================
// ad-read-group 模块
// ============================================================================
// 批量分组 read 容量证据
//
// 数据流：
//   (type, pointer-field) -> (mapof integer-field (listof read-capacity-evidence))
//
// 场景：对指定 (type, field) 进行完整程序扫描，收集所有数组读取的边界检查证据
//       并按整数字段分组
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

// 批量收集并分组 read 容量证据
// 输入：type, field_decl
// 输出：ReadEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
// 同时填充 array_reads wrapper 列表
ArrayDetectErrorCode collectAndGroupReadEvidences (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>** out_array_reads,
  ReadEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（直接填充 wrapper 的 read_evidences_map）
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
