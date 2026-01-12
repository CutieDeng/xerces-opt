#pragma once

// ============================================================================
// ad-write-group 模块
// ============================================================================
// 批量分组 write 容量证据
//
// 数据流：
//   (type, pointer-field) -> (mapof integer-field (listof write-capacity-evidence))
//
// 场景：对指定 (type, field) 进行完整程序扫描，收集所有数组写入的边界检查证据
//       并按整数字段分组
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

// 批量收集并分组 write 容量证据
// 输入：type, field_decl
// 输出：WriteEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
// 同时填充 array_writes wrapper 列表
ArrayDetectErrorCode collectAndGroupWriteEvidences (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>** out_array_writes,
  WriteEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（直接填充 wrapper 的 write_evidences_map）
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
