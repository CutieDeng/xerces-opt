// ============================================================================
// ad-malloc-group 模块实现
// ============================================================================
// 批量分组 malloc 容量证据
// ============================================================================

#include "malloc-group.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 批量收集并分组 malloc 容量证据
// ============================================================================

ArrayDetectErrorCode collectAndGroupMallocEvidences (
  AD_FUNC_ARGS,
  tree field_decl,
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo*, va_gc>* writes,
  tree containing_type,
  MallocEvidencesByIntegerFieldMap** out_map
) AD_FUNCTION_BEGIN {
  (void)field_decl;

  if (!out_map) {
    AD_RETURNE (OK);
  }

  // 获取类型名用于调试
  char const* type_name_dbg = NULL;
  gcc_ext_util::formatTypeNameWithTemplateArgs (AD_ARGS, containing_type, type_name_dbg);
  char const* field_name_dbg = safeGetFieldName (AD_ARGS, field_decl);
  AD_DEBUG_PRINT ("[malloc-group] === Analyzing field %s.%s ===",
                  type_name_dbg ? type_name_dbg : "?", field_name_dbg);

  // 初始化 hashmap
  if (!*out_map) {
    *out_map = new MallocEvidencesByIntegerFieldMap ();
  }

  if (!writes) {
    AD_DEBUG_PRINT ("[malloc-group]   SKIP: writes vector is NULL");
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[malloc-group]   writes count = %u", writes->length ());

  // 遍历所有写入
  for (unsigned i = 0; i < writes->length (); i++) {
    Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo* write_wrapper = (*writes)[i];
    if (!write_wrapper) {
      AD_DEBUG_PRINT ("[malloc-group]   write[%u]: write_wrapper is NULL", i);
      continue;
    }

    FieldWriteInfo* write_info = write_wrapper->write_info;
    WriteOriginalSource* write_source = write_wrapper->write_source;

    AD_DEBUG_PRINT ("[malloc-group]   write[%u]: write_info=%p, write_source=%p",
                    i, (void*)write_info, (void*)write_source);

    if (!write_info || !write_source) {
      AD_DEBUG_PRINT ("[malloc-group]   write[%u]: SKIP - missing write_info or write_source", i);
      continue;
    }

    // 调用单项分析
    vec<MallocCapacityEvidence*, va_gc>* evidences = NULL;
    AD_TRY (analyzeMallocCapacity (AD_ARGS, write_info, write_source, containing_type, &evidences));

    unsigned ev_count = evidences ? evidences->length () : 0;
    AD_DEBUG_PRINT ("[malloc-group]   write[%u]: analyzeMallocCapacity returned %u evidences", i, ev_count);

    // 将证据存入写入级 wrapper
    write_wrapper->malloc_evidences = evidences;

    // 按 integer_field 分组到 hashmap
    if (evidences) {
      for (unsigned j = 0; j < evidences->length (); j++) {
        MallocCapacityEvidence* ev = (*evidences)[j];
        if (!ev || !ev->integer_field) continue;

        // 获取或创建该 integer_field 的证据列表
        vec<MallocCapacityEvidence*, va_gc>** slot =
          &(*out_map)->get_or_insert (ev->integer_field);
        if (!*slot) {
          vec_alloc (*slot, 4);
        }
        vec_safe_push (*slot, ev);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 在字段级 Wrapper 上执行分组
// ============================================================================

ArrayDetectErrorCode groupMallocEvidencesForFieldWrapper (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_TransferStats* field_wrapper
) AD_FUNCTION_BEGIN {
  if (!field_wrapper) {
    AD_RETURNE (OK);
  }

  AD_TRY (collectAndGroupMallocEvidences (
    AD_ARGS,
    field_wrapper->field_decl,
    field_wrapper->writes,
    field_wrapper->type,
    &field_wrapper->malloc_evidences_map
  ));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 从分组结果中提取扁平列表
// ============================================================================

ArrayDetectErrorCode extractFlatMallocEvidences (
  AD_FUNC_ARGS,
  MallocEvidencesByIntegerFieldMap* evidences_map,
  vec<MallocCapacityEvidence*, va_gc>** out_flat_list
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
    vec<MallocCapacityEvidence*, va_gc>* evidences = (*iter).second;
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

vec<MallocCapacityEvidence*, va_gc>* findMallocEvidencesForIntegerField (
  MallocEvidencesByIntegerFieldMap* evidences_map,
  tree integer_field
) {
  if (!evidences_map || !integer_field) {
    return NULL;
  }

  vec<MallocCapacityEvidence*, va_gc>** slot = evidences_map->get (integer_field);
  return slot ? *slot : NULL;
}

// ============================================================================
// 打印分组结果
// ============================================================================

void printMallocEvidenceGroups (
  AD_FUNC_ARGS,
  FILE* out,
  MallocEvidencesByIntegerFieldMap* evidences_map
) {
  if (!out || !evidences_map) return;

  fprintf (out, "  [malloc-evidence-groups]\n");

  for (auto iter = evidences_map->begin ();
       iter != evidences_map->end ();
       ++iter) {
    tree integer_field = (*iter).first;
    vec<MallocCapacityEvidence*, va_gc>* evidences = (*iter).second;

    char const* field_name = safeGetFieldName (AD_ARGS, integer_field);
    unsigned count = evidences ? evidences->length () : 0;

    fprintf (out, "    integer_field='%s' count=%u\n", field_name, count);

    if (evidences) {
      for (unsigned i = 0; i < evidences->length (); i++) {
        MallocCapacityEvidence* ev = (*evidences)[i];
        if (!ev) continue;

        char const* ptr_name = safeGetFieldName (AD_ARGS, ev->pointer_field);
        fprintf (out, "      [%u] ptr='%s' alloc='%s'\n",
                 i, ptr_name,
                 ev->alloc_func_name ? ev->alloc_func_name : "?");
      }
    }
  }
}

} // namespace array_detect_ns
