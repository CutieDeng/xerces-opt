#pragma once

// ============================================================================
// ad-write-group 模块
// ============================================================================
// 批量分组 write 容量证据
//
// 数据流：
//   (listof array-write-access) -> 按 (type, field) 分组后按 integer-field 分组
//
// 场景：从全程序扫描得到的所有数组写入，分析并按字段分组证据
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

// 从所有收集的 writes 中提取并分组特定 (type, field) 的证据
// 输入：all_writes (全程序扫描结果), type, field_decl
// 输出：WriteEvidencesByIntegerFieldMap* - 按 integer_field 分组的 hashmap
ArrayDetectErrorCode groupWriteEvidencesForField (
  AD_FUNC_ARGS,
  vec<ArrayWriteAccess*, va_gc>* all_writes,
  tree type,
  tree field_decl,
  WriteEvidencesByIntegerFieldMap** out_map
);

// 在字段级 Wrapper 上执行分组（从全程序扫描结果中过滤）
// 需要传入全程序扫描的所有 writes
ArrayDetectErrorCode groupWriteEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  vec<ArrayWriteAccess*, va_gc>* all_writes,
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
