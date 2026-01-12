// ============================================================================
// ad-write-group 模块实现
// ============================================================================
// 批量分组 write 容量证据
// ============================================================================

#include "write-group.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 从 Wrapper 的 array_writes 中提取并分组证据
// ============================================================================

ArrayDetectErrorCode groupWriteEvidencesForField (
  AD_FUNC_ARGS,
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>* write_wrappers,
  WriteEvidencesByIntegerFieldMap** out_map
) AD_FUNCTION_BEGIN {
  if (!out_map) {
    AD_RETURNE (OK);
  }

  if (!write_wrappers || write_wrappers->length () == 0) {
    AD_RETURNE (OK);
  }

  // 遍历所有 write wrappers
  for (unsigned i = 0; i < write_wrappers->length (); i++) {
    Wrapper_ArrayWriteAccess_WriteBoundConditions* write_wrapper = (*write_wrappers)[i];
    if (!write_wrapper || !write_wrapper->write_access) continue;

    ArrayWriteAccess* access = write_wrapper->write_access;

    // 单项分析：从 access 生成所有证据
    vec<WriteCapacityEvidence*, va_gc>* evidences = NULL;
    AD_TRY (analyzeWriteAccessToEvidences (AD_ARGS, access, &evidences));

    if (!evidences || evidences->length () == 0) {
      continue;
    }

    // 初始化 hashmap
    if (!*out_map) {
      *out_map = new WriteEvidencesByIntegerFieldMap ();
    }

    // 将证据按 integer_field 分组
    for (unsigned j = 0; j < evidences->length (); j++) {
      WriteCapacityEvidence* evidence = (*evidences)[j];
      if (!evidence || !evidence->integer_field) continue;

      vec<WriteCapacityEvidence*, va_gc>** slot =
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

ArrayDetectErrorCode groupWriteEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* field_wrapper
) AD_FUNCTION_BEGIN {
  if (!field_wrapper) {
    AD_RETURNE (OK);
  }

  AD_TRY (groupWriteEvidencesForField (
    AD_ARGS,
    field_wrapper->array_writes,
    &field_wrapper->write_evidences_map
  ));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 从分组结果中提取扁平列表
// ============================================================================

ArrayDetectErrorCode extractFlatWriteEvidences (
  AD_FUNC_ARGS,
  WriteEvidencesByIntegerFieldMap* evidences_map,
  vec<WriteCapacityEvidence*, va_gc>** out_flat_list
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
    vec<WriteCapacityEvidence*, va_gc>* evidences = (*iter).second;
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

vec<WriteCapacityEvidence*, va_gc>* findWriteEvidencesForIntegerField (
  WriteEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
) {
  if (!evidences_map || !integer_field) {
    return NULL;
  }

  vec<WriteCapacityEvidence*, va_gc>** slot = evidences_map->get (integer_field);
  return slot ? *slot : NULL;
}

// ============================================================================
// 打印分组结果
// ============================================================================

void printWriteEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  WriteEvidencesByIntegerFieldMap* evidences_map
) {
  if (!out || !evidences_map) return;

  fprintf (out, "  [write-evidence-groups]\n");

  for (auto iter = evidences_map->begin ();
       iter != evidences_map->end ();
       ++iter) {
    tree integer_field = (*iter).first;
    vec<WriteCapacityEvidence*, va_gc>* evidences = (*iter).second;

    char const* field_name = safeGetFieldName (AD_ARGS, integer_field);
    unsigned count = evidences ? evidences->length () : 0;

    fprintf (out, "    integer_field='%s' count=%u\n", field_name, count);

    if (evidences) {
      for (unsigned i = 0; i < evidences->length (); i++) {
        WriteCapacityEvidence* ev = (*evidences)[i];
        if (!ev) continue;

        char const* ptr_name = safeGetFieldName (AD_ARGS, ev->pointer_field);
        fprintf (out, "      [%u] ptr='%s' confidence='%s'\n",
                 i, ptr_name,
                 writeConfidenceToString (ev->confidence));
      }
    }
  }
}

} // namespace array_detect_ns
