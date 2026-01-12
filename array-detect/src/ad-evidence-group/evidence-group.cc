#include "evidence-group.hh"
#include "field-write-capacity.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 内部辅助函数：查找或创建分组
// ============================================================================

static Wrapper_IntegerField_MallocEvidences* findOrCreateGroup (
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>** groups,
  tree integer_field
) {
  if (!integer_field) return nullptr;

  // 查找现有分组
  if (*groups) {
    for (unsigned i = 0; i < (*groups)->length (); i++) {
      Wrapper_IntegerField_MallocEvidences* g = (**groups)[i];
      if (g && g->integer_field == integer_field) {
        return g;
      }
    }
  }

  // 创建新分组
  Wrapper_IntegerField_MallocEvidences* group =
    ggc_alloc<Wrapper_IntegerField_MallocEvidences> ();
  group->integer_field = integer_field;
  group->malloc_evidences = nullptr;

  // 添加到列表
  vec_safe_push (*groups, group);

  return group;
}

// ============================================================================
// 将 malloc 证据按 integer_field 分组
// ============================================================================

ArrayDetectErrorCode groupMallocEvidencesByIntegerField (
  AD_FUNC_ARGS,
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences,
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>** result
) {
  (void) gcc_ctx;
  if (!result) return OK;
  *result = nullptr;

  if (!malloc_evidences || malloc_evidences->length () == 0) {
    return OK;
  }

  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>* groups = nullptr;

  for (unsigned i = 0; i < malloc_evidences->length (); i++) {
    MallocCapacityEvidence* ev = (*malloc_evidences)[i];
    if (!ev || !ev->integer_field) continue;

    Wrapper_IntegerField_MallocEvidences* group =
      findOrCreateGroup (&groups, ev->integer_field);
    if (!group) continue;

    vec_safe_push (group->malloc_evidences, ev);
  }

  unsigned int group_count = groups ? groups->length () : 0;
  AD_DEBUG_PRINT ("[groupMallocEvidences] %u evidences -> %u groups",
                  malloc_evidences->length (), group_count);

  *result = groups;
  return OK;
}

// ============================================================================
// 在 Wrapper 上执行分组
// ============================================================================

ArrayDetectErrorCode groupMallocEvidencesForWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* wrapper
) {
  if (!wrapper) return OK;

  return groupMallocEvidencesByIntegerField (
    AD_ARGS,
    wrapper->malloc_evidences,
    &wrapper->malloc_evidences_grouped
  );
}

// ============================================================================
// 查找特定 integer_field 的分组
// ============================================================================

Wrapper_IntegerField_MallocEvidences* findMallocEvidenceGroup (
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>* groups,
  tree integer_field
) {
  if (!groups || !integer_field) return nullptr;

  for (unsigned i = 0; i < groups->length (); i++) {
    Wrapper_IntegerField_MallocEvidences* g = (*groups)[i];
    if (g && g->integer_field == integer_field) {
      return g;
    }
  }

  return nullptr;
}

// ============================================================================
// 打印分组结果
// ============================================================================

void printMallocEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  vec<Wrapper_IntegerField_MallocEvidences*, va_gc>* groups
) {
  (void) ctx;
  (void) gcc_ctx;
  if (!out || !groups) return;

  fprintf (out, "=== Malloc Evidence Groups (%u) ===\n", groups->length ());

  for (unsigned i = 0; i < groups->length (); i++) {
    Wrapper_IntegerField_MallocEvidences* g = (*groups)[i];
    if (!g) continue;

    char const* field_name = "<unnamed>";
    if (g->integer_field && TREE_CODE (g->integer_field) == FIELD_DECL
        && DECL_NAME (g->integer_field)) {
      field_name = IDENTIFIER_POINTER (DECL_NAME (g->integer_field));
    }

    unsigned int ev_count = g->malloc_evidences ? g->malloc_evidences->length () : 0;
    fprintf (out, "  [%s]: %u evidences\n", field_name, ev_count);
  }

  fprintf (out, "==================================\n");
}

} // namespace array_detect_ns
