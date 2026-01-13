#pragma once

// ============================================================================
// ad-malloc-group 模块
// ============================================================================
// 批量分组 malloc 容量证据
//
// 数据流：
//   (field, (listof field-write-info)) -> (mapof integer-field (listof malloc-capacity-evidence))
//
// 场景：将单个字段的所有写入收集的 malloc 证据按整数字段分组
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "malloc-capacity.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 函数声明
// ============================================================================

// 批量收集并分组 malloc 容量证据
// 输入：field_decl, writes (所有写入的 wrapper 列表), containing_type
// 输出：MallocEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
ArrayDetectErrorCode collectAndGroupMallocEvidences (
  AD_FUNC_ARGS,
  tree field_decl,
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo*, va_gc>* writes,
  tree containing_type,
  MallocEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（直接填充 wrapper 的 malloc_evidences_map）
ArrayDetectErrorCode groupMallocEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_TransferStats* field_wrapper
);

// 从分组结果中提取扁平列表（用于需要遍历所有证据的场景）
ArrayDetectErrorCode extractFlatMallocEvidences (
  AD_FUNC_ARGS,
  MallocEvidencesByIntegerFieldMap* evidences_map,
  vec<MallocCapacityEvidence*, va_gc>** out_flat_list
);

// 查找特定整数字段的证据列表
vec<MallocCapacityEvidence*, va_gc>* findMallocEvidencesForIntegerField (
  MallocEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
);

// 打印分组结果
void printMallocEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  MallocEvidencesByIntegerFieldMap* evidences_map
);

} // namespace array_detect_ns
