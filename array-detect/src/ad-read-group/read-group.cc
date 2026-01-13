// ============================================================================
// ad-read-group 模块实现
// ============================================================================
// 批量分组 read 容量证据
// ============================================================================

#include "read-group.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 从 Wrapper 的 array_reads 中提取并分组证据
// ============================================================================

ArrayDetectErrorCode groupReadEvidencesForField (
  AD_FUNC_ARGS,
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>* read_wrappers,
  ReadEvidencesByIntegerFieldMap** out_map
) AD_FUNCTION_BEGIN {
  if (!out_map) {
    AD_RETURNE (OK);
  }

  if (!read_wrappers || read_wrappers->length () == 0) {
    AD_RETURNE (OK);
  }

  // 遍历所有 read wrappers
  for (unsigned i = 0; i < read_wrappers->length (); i++) {
    Wrapper_ArrayReadAccess_ReadBoundConditions* read_wrapper = (*read_wrappers)[i];
    if (!read_wrapper || !read_wrapper->read_access) continue;

    ArrayReadAccess* access = read_wrapper->read_access;

    // 单项分析：从 access 生成所有证据
    vec<ReadCapacityEvidence*, va_gc>* evidences = NULL;
    AD_TRY (analyzeReadAccessToEvidences (AD_ARGS, access, &evidences));

    if (!evidences || evidences->length () == 0) {
      continue;
    }

    // 初始化 hashmap
    if (!*out_map) {
      *out_map = new ReadEvidencesByIntegerFieldMap ();
    }

    // 将证据按 integer_field 分组
    for (unsigned j = 0; j < evidences->length (); j++) {
      ReadCapacityEvidence* evidence = (*evidences)[j];
      if (!evidence || !evidence->integer_field) continue;

      vec<ReadCapacityEvidence*, va_gc>** slot =
        &(*out_map)->get_or_insert (evidence->integer_field);
      if (!*slot) {
        vec_alloc (*slot, 4);
      }
      vec_safe_push (*slot, evidence);
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 在字段级 Wrapper 上执行分组
// ============================================================================

ArrayDetectErrorCode groupReadEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_TransferStats* field_wrapper
) AD_FUNCTION_BEGIN {
  if (!field_wrapper) {
    AD_RETURNE (OK);
  }

  AD_TRY (groupReadEvidencesForField (
    AD_ARGS,
    field_wrapper->array_reads,
    &field_wrapper->read_evidences_map
  ));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 从分组结果中提取扁平列表
// ============================================================================

ArrayDetectErrorCode extractFlatReadEvidences (
  AD_FUNC_ARGS,
  ReadEvidencesByIntegerFieldMap* evidences_map,
  vec<ReadCapacityEvidence*, va_gc>** out_flat_list
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!out_flat_list) {
    AD_RETURNE (OK);
  }

  if (!evidences_map) {
    AD_RETURNE (OK);
  }

  // 遍历 hashmap 中所有条目
  for (auto iter = evidences_map->begin ();
       iter != evidences_map->end ();
       ++iter) {
    vec<ReadCapacityEvidence*, va_gc>* evidences = (*iter).second;
    if (!evidences) continue;

    for (unsigned i = 0; i < evidences->length (); i++) {
      vec_safe_push (*out_flat_list, (*evidences)[i]);
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 查找特定整数字段的证据列表
// ============================================================================

vec<ReadCapacityEvidence*, va_gc>* findReadEvidencesForIntegerField (
  ReadEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
) {
  if (!evidences_map || !integer_field) {
    return NULL;
  }

  vec<ReadCapacityEvidence*, va_gc>** slot = evidences_map->get (integer_field);
  return slot ? *slot : NULL;
}

// ============================================================================
// 打印分组结果
// ============================================================================

void printReadEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  ReadEvidencesByIntegerFieldMap* evidences_map
) {
  if (!out || !evidences_map) return;

  fprintf (out, "  [read-evidence-groups]\n");

  for (auto iter = evidences_map->begin ();
       iter != evidences_map->end ();
       ++iter) {
    tree integer_field = (*iter).first;
    vec<ReadCapacityEvidence*, va_gc>* evidences = (*iter).second;

    char const* field_name = safeGetFieldName (AD_ARGS, integer_field);
    unsigned count = evidences ? evidences->length () : 0;

    fprintf (out, "    integer_field='%s' count=%u\n", field_name, count);

    if (evidences) {
      for (unsigned i = 0; i < evidences->length (); i++) {
        ReadCapacityEvidence* ev = (*evidences)[i];
        if (!ev) continue;

        char const* ptr_name = safeGetFieldName (AD_ARGS, ev->pointer_field);
        fprintf (out, "      [%u] ptr='%s' confidence='%s'\n",
                 i, ptr_name,
                 readConfidenceToString (ev->confidence));
      }
    }
  }
}

} // namespace array_detect_ns
