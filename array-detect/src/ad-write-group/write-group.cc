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
// 批量收集并分组 write 容量证据
// ============================================================================

ArrayDetectErrorCode collectAndGroupWriteEvidences (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>** out_array_writes,
  WriteEvidencesByIntegerFieldMap** out_map
) AD_FUNCTION_BEGIN {
  if (!out_map) {
    AD_RETURNE (OK);
  }

  // Step 1: 收集所有数组写入访问
  vec<ArrayWriteAccess*, va_gc>* write_accesses = NULL;
  AD_TRY (collectAllArrayWriteAccesses (AD_ARGS, type, field_decl, &write_accesses));

  if (!write_accesses || write_accesses->length () == 0) {
    AD_RETURNE (OK);
  }

  // 初始化 hashmap
  if (!*out_map) {
    *out_map = new WriteEvidencesByIntegerFieldMap ();
  }

  // Step 2: 为每个访问分析边界条件
  for (unsigned i = 0; i < write_accesses->length (); i++) {
    ArrayWriteAccess* access = (*write_accesses)[i];
    if (!access) continue;

    // 分析所有边界条件（一对多）
    vec<WriteBoundCondition*, va_gc>* bound_conds = NULL;
    AD_TRY (analyzeWriteBoundConditions (AD_ARGS, access, &bound_conds));

    // 创建 Wrapper（一对多）
    Wrapper_ArrayWriteAccess_WriteBoundConditions* wrapper =
      ggc_alloc<Wrapper_ArrayWriteAccess_WriteBoundConditions>();
    wrapper->write_access = access;
    wrapper->bound_conditions = bound_conds;

    // 添加到 array_writes 列表
    if (out_array_writes) {
      if (!*out_array_writes) {
        vec_alloc (*out_array_writes, write_accesses->length ());
      }
      vec_safe_push (*out_array_writes, wrapper);
    }

    // 为每个边界条件提取证据并分组
    if (bound_conds) {
      for (unsigned j = 0; j < bound_conds->length (); j++) {
        WriteBoundCondition* bound_cond = (*bound_conds)[j];
        if (!bound_cond) continue;

        WriteCapacityEvidence* evidence = NULL;
        AD_TRY (extractWriteCapacityEvidence (AD_ARGS, field_decl, bound_cond, &evidence));

        if (evidence && evidence->integer_field) {
          // 按 integer_field 分组到 hashmap
          vec<WriteCapacityEvidence*, va_gc>** slot =
            &(*out_map)->get_or_insert (evidence->integer_field);
          if (!*slot) {
            vec_alloc (*slot, 4);
          }
          vec_safe_push (*slot, evidence);
        }
      }
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

  AD_TRY (collectAndGroupWriteEvidences (
    AD_ARGS,
    field_wrapper->type,
    field_wrapper->field_decl,
    &field_wrapper->array_writes,
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
