// ============================================================================
// ad-array-read-capacity 模块实现 (兼容层)
// ============================================================================
// 此模块现已拆分为两个子模块，此文件提供兼容函数
// ============================================================================

#include "array-read-capacity.hh"
#include "array-read-collect.hh"
#include "array-read-bound.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 兼容函数：收集所有读容量证据
// ============================================================================
// 此函数保留用于向后兼容，现在调用 pipeline 中的分步实现

ArrayDetectErrorCode collectAllReadEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
) AD_FUNCTION_BEGIN {
  if (!tfad) {
    AD_RETURNE (OK);
  }

  // Step 1: 收集数组读取访问
  vec<ArrayReadAccess*, va_gc>* read_accesses = NULL;
  AD_TRY (collectAllArrayReadAccesses (AD_ARGS, tfad->type, tfad->field_decl, &read_accesses));

  if (!read_accesses || read_accesses->length () == 0) {
    AD_RETURNE (OK);
  }

  // Step 2: 为每个访问分析边界条件并创建 Wrapper (一对多)
  for (unsigned i = 0; i < read_accesses->length (); i++) {
    ArrayReadAccess* access = (*read_accesses)[i];
    if (!access) continue;

    // 分析所有边界条件 (一对多)
    vec<ReadBoundCondition*, va_gc>* bound_conds = NULL;
    AD_TRY (analyzeReadBoundConditions (AD_ARGS, access, &bound_conds));

    // 创建 Wrapper (一对多)
    Wrapper_ArrayReadAccess_ReadBoundConditions* wrapper =
      ggc_alloc<Wrapper_ArrayReadAccess_ReadBoundConditions>();
    wrapper->read_access = access;
    wrapper->bound_conditions = bound_conds;

    // 延迟初始化 array_reads
    if (!tfad->array_reads) {
      vec_alloc (tfad->array_reads, read_accesses->length ());
    }
    vec_safe_push (tfad->array_reads, wrapper);

    // 为每个边界条件提取证据
    if (bound_conds) {
      for (unsigned j = 0; j < bound_conds->length (); j++) {
        ReadBoundCondition* bound_cond = (*bound_conds)[j];
        if (!bound_cond) continue;

        ReadCapacityEvidence* evidence = NULL;
        AD_TRY (extractReadCapacityEvidence (AD_ARGS, tfad->field_decl, bound_cond, &evidence));

        if (evidence) {
          // 延迟初始化 read_evidences
          if (!tfad->read_evidences) {
            vec_alloc (tfad->read_evidences, 4);
          }
          vec_safe_push (tfad->read_evidences, evidence);
        }
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 打印所有读容量证据
// ============================================================================

void printAllReadEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ReadCapacityEvidence*, va_gc>* evidences
) {
  if (!out || !evidences) return;

  fprintf (out, "=== Read Capacity Evidences (%u) ===\n",
           evidences->length ());

  for (unsigned i = 0; i < evidences->length (); i++) {
    printReadCapacityEvidence (AD_ARGS, out, (*evidences)[i]);
  }
}

} // namespace array_detect_ns
