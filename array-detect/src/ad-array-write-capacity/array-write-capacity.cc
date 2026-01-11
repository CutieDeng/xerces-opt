// ============================================================================
// ad-array-write-capacity 模块实现
// ============================================================================
// 整合 ad-array-write-collect 和 ad-array-write-bound 模块
// ============================================================================

#include "array-write-capacity.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 收集所有写容量证据
// ============================================================================

ArrayDetectErrorCode collectAllWriteEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
) AD_FUNCTION_BEGIN {
  if (!tfad) {
    AD_RETURNE (OK);
  }

  // 初始化结果容器
  if (!tfad->write_evidences) {
    vec_alloc (tfad->write_evidences, 4);
  }
  if (!tfad->array_writes) {
    vec_alloc (tfad->array_writes, 4);
  }

  // Step 1: 收集所有数组写入访问
  vec<ArrayWriteAccess*, va_gc>* write_accesses = NULL;
  AD_TRY (collectAllArrayWriteAccesses (
    AD_ARGS, tfad->type, tfad->field_decl, &write_accesses
  ));

  if (!write_accesses || write_accesses->length () == 0) {
    AD_RETURNE (OK);
  }

  // Step 2: 对每个写入访问，分析边界条件并提取证据
  for (unsigned i = 0; i < write_accesses->length (); i++) {
    ArrayWriteAccess* access = (*write_accesses)[i];
    if (!access) continue;

    // 分析边界条件 (一对多)
    vec<WriteBoundCondition*, va_gc>* bound_conds = NULL;
    ArrayDetectErrorCode err = analyzeWriteBoundConditions (AD_ARGS, access, &bound_conds);
    if (err != OK) continue;

    // 创建 Wrapper 并添加到 array_writes
    Wrapper_ArrayWriteAccess_WriteBoundConditions* wrapper =
      ggc_alloc<Wrapper_ArrayWriteAccess_WriteBoundConditions>();
    wrapper->write_access = access;
    wrapper->bound_conditions = bound_conds;
    vec_safe_push (tfad->array_writes, wrapper);

    // 对每个边界条件提取证据
    if (bound_conds) {
      for (unsigned j = 0; j < bound_conds->length (); j++) {
        WriteBoundCondition* bound_cond = (*bound_conds)[j];
        if (!bound_cond) continue;

        WriteCapacityEvidence* evidence = NULL;
        err = extractWriteCapacityEvidence (
          AD_ARGS, tfad->field_decl, bound_cond, &evidence
        );

        if (err == OK && evidence) {
          vec_safe_push (tfad->write_evidences, evidence);
        }
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 打印所有写容量证据
// ============================================================================

void printAllWriteEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  vec<WriteCapacityEvidence*, va_gc>* evidences
) {
  if (!out || !evidences) return;

  fprintf (out, "=== Write Capacity Evidences (%u) ===\n",
           evidences->length ());

  for (unsigned i = 0; i < evidences->length (); i++) {
    printWriteCapacityEvidence (AD_ARGS, out, (*evidences)[i]);
  }
}

} // namespace array_detect_ns
