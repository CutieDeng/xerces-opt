// ============================================================================
// ad-array-write-bound 模块实现
// ============================================================================
// 分析数组写入访问的边界条件，寻找关联整数字段
// ============================================================================

#include "array-write-bound.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

// ============================================================================
// 辅助函数：判断类型是否为整数类型
// ============================================================================

static bool isIntegerType (tree type) {
  if (!type) return false;
  tree main_type = TYPE_MAIN_VARIANT (type);
  return INTEGRAL_TYPE_P (main_type);
}

// ============================================================================
// 辅助函数：检查表达式是否引用了指定字段
// ============================================================================

static bool exprReferencesField (
  AD_FUNC_ARGS,
  tree expr,
  tree field_decl
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || !field_decl) {
    return false;
  }

  if (TREE_CODE (expr) == COMPONENT_REF) {
    tree accessed_field = TREE_OPERAND (expr, 1);
    if (accessed_field == field_decl) {
      return true;
    }
    tree base = TREE_OPERAND (expr, 0);
    if (exprReferencesField (AD_ARGS, base, field_decl)) {
      return true;
    }
  }

  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def_stmt = SSA_NAME_DEF_STMT (expr);
    if (def_stmt && is_gimple_assign (def_stmt)) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (exprReferencesField (AD_ARGS, rhs, field_decl)) {
        return true;
      }
      if (gimple_assign_rhs2 (def_stmt)) {
        if (exprReferencesField (AD_ARGS, gimple_assign_rhs2 (def_stmt), field_decl)) {
          return true;
        }
      }
    }
  }

  if (BINARY_CLASS_P (expr)) {
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl)) {
      return true;
    }
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 1), field_decl)) {
      return true;
    }
  }

  if (CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl)) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// 查找支配该访问的条件语句
// ============================================================================

ArrayDetectErrorCode findWriteDominatingConditions (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  vec<gimple*, va_gc>** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!access || !access->bb || !access->fn) {
    AD_RETURNE (OK);
  }

  // 简化实现：检查当前基本块和前驱块的条件
  basic_block bb = access->bb;

  // 检查前驱块
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE (e, ei, bb->preds) {
    basic_block pred_bb = e->src;
    gimple_stmt_iterator gsi = gsi_last_bb (pred_bb);
    if (!gsi_end_p (gsi)) {
      gimple* last_stmt = gsi_stmt (gsi);
      if (last_stmt && gimple_code (last_stmt) == GIMPLE_COND) {
        // 延迟初始化
        if (!*result) {
          vec_alloc (*result, 4);
        }
        vec_safe_push (*result, last_stmt);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析数组写入的所有边界条件（一对多）
// ============================================================================

ArrayDetectErrorCode analyzeWriteBoundConditions (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  vec<WriteBoundCondition*, va_gc>** results
) AD_FUNCTION_BEGIN {
  *results = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // 查找支配条件
  vec<gimple*, va_gc>* conditions = NULL;
  AD_TRY (findWriteDominatingConditions (AD_ARGS, access, &conditions));

  if (!conditions || conditions->length () == 0) {
    AD_RETURNE (OK);
  }

  // 分析每个条件，收集所有边界条件
  for (unsigned i = 0; i < conditions->length (); i++) {
    gimple* cond_stmt = (*conditions)[i];
    if (!cond_stmt || gimple_code (cond_stmt) != GIMPLE_COND) continue;

    tree_code code = gimple_cond_code (cond_stmt);
    tree lhs = gimple_cond_lhs (cond_stmt);
    tree rhs = gimple_cond_rhs (cond_stmt);

    // 检查是否为边界比较 (index < bound)
    if (code != LT_EXPR && code != LE_EXPR && code != GT_EXPR && code != GE_EXPR) {
      continue;
    }

    // 查找边界操作数中的整数字段
    tree index_op = NULL_TREE;
    tree bound_op = NULL_TREE;

    // 尝试匹配 index < bound
    if (code == LT_EXPR || code == LE_EXPR) {
      index_op = lhs;
      bound_op = rhs;
    } else {
      index_op = rhs;
      bound_op = lhs;
    }

    // 在 bound_op 中查找所有引用的整数字段
    for (tree field = TYPE_FIELDS (access->containing_type); field; field = DECL_CHAIN (field)) {
      if (TREE_CODE (field) != FIELD_DECL) continue;
      if (!isIntegerType (TREE_TYPE (field))) continue;

      if (exprReferencesField (AD_ARGS, bound_op, field)) {
        // 创建 WriteBoundCondition
        WriteBoundCondition* bound_cond = ggc_alloc<WriteBoundCondition>();
        memset (bound_cond, 0, sizeof (WriteBoundCondition));

        bound_cond->access = access;
        bound_cond->condition_stmt = cond_stmt;
        bound_cond->comparison_code = code;
        bound_cond->index_operand = index_op;
        bound_cond->bound_operand = bound_op;
        bound_cond->has_field_bound = true;
        bound_cond->bound_field_decl = field;
        bound_cond->bound_type = access->containing_type;
        bound_cond->condition_bb = gimple_bb (cond_stmt);
        bound_cond->dominates_access = true;
        bound_cond->location = gimple_location (cond_stmt);

        // 延迟初始化并添加到结果列表
        if (!*results) {
          vec_alloc (*results, 4);
        }
        vec_safe_push (*results, bound_cond);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 提取写容量证据
// ============================================================================

ArrayDetectErrorCode extractWriteCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  WriteBoundCondition* bound_cond,
  WriteCapacityEvidence** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!pointer_field || !bound_cond || !bound_cond->has_field_bound) {
    AD_RETURNE (OK);
  }

  WriteCapacityEvidence* evidence = ggc_alloc<WriteCapacityEvidence>();
  memset (evidence, 0, sizeof (WriteCapacityEvidence));

  evidence->pointer_field = pointer_field;
  evidence->integer_field = bound_cond->bound_field_decl;
  evidence->containing_type = bound_cond->bound_type;
  evidence->write_access = bound_cond->access;
  evidence->bound_condition = bound_cond;
  evidence->confidence = WRITE_CONF_CERTAIN;
  evidence->location = bound_cond->location;
  evidence->description = "Array write with field-based bound check";

  AD_DEBUG_PRINT ("[write-capacity] ptr '%s' -> bound field '%s'",
                  safeGetFieldName (AD_ARGS, pointer_field),
                  safeGetFieldName (AD_ARGS, bound_cond->bound_field_decl));

  *result = evidence;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 获取置信度名称
// ============================================================================

char const* writeConfidenceToString (WriteEvidenceConfidence conf) {
  switch (conf) {
    case WRITE_CONF_CERTAIN:  return "certain";
    case WRITE_CONF_PROBABLE: return "probable";
    case WRITE_CONF_WEAK:     return "weak";
    default:                  return "unknown";
  }
}

// ============================================================================
// 打印写边界条件
// ============================================================================

void printWriteBoundCondition (
  AD_FUNC_ARGS,
  FILE* out,
  WriteBoundCondition* bound_cond
) {
  if (!out || !bound_cond) return;

  char const* bound_name = bound_cond->has_field_bound
    ? safeGetFieldName (AD_ARGS, bound_cond->bound_field_decl)
    : "(none)";

  fprintf (out, "  [write-bound] bound_field='%s' at %s:%d\n",
           bound_name,
           LOCATION_FILE (bound_cond->location) ? LOCATION_FILE (bound_cond->location) : "?",
           LOCATION_LINE (bound_cond->location));
}

// ============================================================================
// 打印写容量证据
// ============================================================================

void printWriteCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  WriteCapacityEvidence* evidence
) {
  if (!out || !evidence) return;

  char const* ptr_name = safeGetFieldName (AD_ARGS, evidence->pointer_field);
  char const* int_name = safeGetFieldName (AD_ARGS, evidence->integer_field);

  fprintf (out, "  [write-evidence] %s -> %s confidence=%s\n",
           ptr_name,
           int_name,
           writeConfidenceToString (evidence->confidence));
}

// ============================================================================
// 打印所有写容量证据
// ============================================================================

void printAllWriteCapacityEvidences (
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

// ============================================================================
// 单项分析函数：从单个 ArrayWriteAccess 生成所有 WriteCapacityEvidence
// ============================================================================

ArrayDetectErrorCode analyzeWriteAccessToEvidences (
  AD_FUNC_ARGS,
  ArrayWriteAccess* access,
  vec<WriteCapacityEvidence*, va_gc>** results
) AD_FUNCTION_BEGIN {
  *results = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // Step 1: 分析所有边界条件
  vec<WriteBoundCondition*, va_gc>* bound_conds = NULL;
  AD_TRY (analyzeWriteBoundConditions (AD_ARGS, access, &bound_conds));

  if (!bound_conds || bound_conds->length () == 0) {
    AD_RETURNE (OK);
  }

  // Step 2: 为每个边界条件提取证据
  for (unsigned i = 0; i < bound_conds->length (); i++) {
    WriteBoundCondition* bound_cond = (*bound_conds)[i];
    if (!bound_cond || !bound_cond->has_field_bound) continue;

    // 使用 access 中的 pointer_field_decl 作为 pointer_field
    WriteCapacityEvidence* evidence = NULL;
    AD_TRY (extractWriteCapacityEvidence (
      AD_ARGS,
      access->pointer_field_decl,
      bound_cond,
      &evidence
    ));

    if (evidence) {
      if (!*results) {
        vec_alloc (*results, 4);
      }
      vec_safe_push (*results, evidence);
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
