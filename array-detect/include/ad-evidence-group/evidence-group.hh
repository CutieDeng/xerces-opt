#pragma once

// ============================================================================
// ad-evidence-group 模块
// ============================================================================
// 按 integer_field 分组 malloc 证据
//
// 数据流：
//   (listof malloc-capacity-evidence) -> (listof wrapper-integer-field-malloc-evidences)
//
// 将扁平的 malloc 证据列表按关联的整数字段进行分组
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 函数声明
// ============================================================================

// 将 malloc 证据按 integer_field 分组
// 输入：malloc_evidences (扁平列表)
// 输出：(listof Wrapper_IntegerField_MallocEvidences*)
ArrayDetectErrorCode groupMallocEvidencesByIntegerField (
  AD_FUNC_ARGS,
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences,
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>** result
);

// 在 Wrapper 上执行分组
// 输入：tfad (包含 malloc_evidences)
// 输出：填充 tfad->malloc_evidences_grouped
ArrayDetectErrorCode groupMallocEvidencesForWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 查找特定 integer_field 的分组
Wrapper_IntegerField_MallocEvidences* findMallocEvidenceGroup (
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>* groups,
  tree integer_field
);

// 打印分组结果
void printMallocEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>* groups
);

} // namespace array_detect_ns
