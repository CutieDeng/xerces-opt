// ============================================================================
// ad-array-read-capacity 模块实现
// ============================================================================
// 收集数组读取访问，分析读边界条件，寻找关联整数字段
// ============================================================================

#include "array-read-capacity.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 辅助函数：判断类型是否为整数类型
// ============================================================================

static bool isIntegerType (tree type) {
  if (!type) return false;
  tree main_type = TYPE_MAIN_VARIANT (type);
  return INTEGRAL_TYPE_P (main_type);
}

// ============================================================================
// 追溯基础指针到字段访问
// ============================================================================

static ArrayDetectErrorCode traceBasePointerToFieldInternal (
  AD_FUNC_ARGS,
  tree base_pointer,
  tree* out_type,
  tree* out_field_decl
) AD_FUNCTION_BEGIN {
  *out_type = NULL_TREE;
  *out_field_decl = NULL_TREE;

  tree current = base_pointer;
  int depth = 0;
  int const MAX_DEPTH = 10;

  while (current && depth < MAX_DEPTH) {
    depth++;

    // 直接是 COMPONENT_REF (字段访问)
    if (TREE_CODE (current) == COMPONENT_REF) {
      tree field = TREE_OPERAND (current, 1);
      tree base_obj = TREE_OPERAND (current, 0);

      if (field && TREE_CODE (field) == FIELD_DECL) {
        tree base_type = TREE_TYPE (base_obj);
        if (base_type && TREE_CODE (base_type) == RECORD_TYPE) {
          *out_type = TYPE_MAIN_VARIANT (base_type);
          *out_field_decl = field;
          AD_RETURNE (OK);
        }
      }
    }

    // SSA_NAME: 追溯定义
    if (TREE_CODE (current) == SSA_NAME) {
      gimple* def_stmt = SSA_NAME_DEF_STMT (current);
      if (!def_stmt) break;

      if (gimple_code (def_stmt) == GIMPLE_ASSIGN) {
        tree rhs = gimple_assign_rhs1 (def_stmt);
        if (rhs && TREE_CODE (rhs) == COMPONENT_REF) {
          current = rhs;
          continue;
        }

        enum tree_code rhs_code = gimple_assign_rhs_code (def_stmt);
        if (rhs_code == SSA_NAME || CONVERT_EXPR_CODE_P (rhs_code) ||
            rhs_code == NOP_EXPR || rhs_code == VIEW_CONVERT_EXPR) {
          current = rhs;
          continue;
        }
      }
      break;
    }

    // MEM_REF: 检查基址
    if (TREE_CODE (current) == MEM_REF) {
      tree base = TREE_OPERAND (current, 0);
      current = base;
      continue;
    }

    // 类型转换
    if (CONVERT_EXPR_P (current) || TREE_CODE (current) == NOP_EXPR ||
        TREE_CODE (current) == VIEW_CONVERT_EXPR) {
      current = TREE_OPERAND (current, 0);
      continue;
    }

    // 地址取值
    if (TREE_CODE (current) == ADDR_EXPR) {
      current = TREE_OPERAND (current, 0);
      continue;
    }

    break;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 检查表达式是否为数组读取
// ============================================================================

bool isArrayReadExpr (tree expr) {
  if (!expr) return false;

  enum tree_code code = TREE_CODE (expr);
  return (code == ARRAY_REF || code == MEM_REF);
}

// ============================================================================
// 收集单个语句中的数组读取访问
// ============================================================================

ArrayDetectErrorCode collectArrayReadAccess (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  tree target_type,
  tree target_field,
  ArrayReadAccess** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!stmt || !is_gimple_assign (stmt)) {
    AD_RETURNE (OK);
  }

  // 只关注赋值语句的 RHS（读取）
  tree rhs = gimple_assign_rhs1 (stmt);
  if (!rhs) {
    AD_RETURNE (OK);
  }

  // 检查是否为数组访问
  if (!isArrayReadExpr (rhs)) {
    AD_RETURNE (OK);
  }

  tree base_pointer = NULL_TREE;
  tree index_expr = NULL_TREE;

  // ARRAY_REF: ptr[i]
  if (TREE_CODE (rhs) == ARRAY_REF) {
    base_pointer = TREE_OPERAND (rhs, 0);
    index_expr = TREE_OPERAND (rhs, 1);
  }
  // MEM_REF: *(ptr + offset)
  else if (TREE_CODE (rhs) == MEM_REF) {
    base_pointer = TREE_OPERAND (rhs, 0);
    index_expr = TREE_OPERAND (rhs, 1);
  }

  if (!base_pointer) {
    AD_RETURNE (OK);
  }

  // 追溯基指针到字段
  tree found_type = NULL_TREE;
  tree found_field = NULL_TREE;
  AD_TRY (traceBasePointerToFieldInternal (AD_ARGS, base_pointer, &found_type, &found_field));

  // 检查是否匹配目标字段
  if (!found_type || !found_field) {
    AD_RETURNE (OK);
  }

  if (target_type && TYPE_MAIN_VARIANT (found_type) != TYPE_MAIN_VARIANT (target_type)) {
    AD_RETURNE (OK);
  }

  if (target_field && found_field != target_field) {
    AD_RETURNE (OK);
  }

  // 创建 ArrayReadAccess
  ArrayReadAccess* access = ggc_alloc<ArrayReadAccess>();
  memset (access, 0, sizeof (ArrayReadAccess));

  access->stmt = stmt;
  access->base_pointer = base_pointer;
  access->index_expr = index_expr;
  access->element_type = TREE_TYPE (rhs);
  access->is_field_based = true;
  access->containing_type = found_type;
  access->pointer_field_decl = found_field;
  access->fn = fn;
  access->bb = gimple_bb (stmt);
  access->location = gimple_location (stmt);

  *result = access;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 检查表达式是否引用了指定字段
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

ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<gimple*, va_gc>** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!access || !access->bb || !access->fn) {
    AD_RETURNE (OK);
  }

  vec<gimple*, va_gc>* conditions = NULL;
  vec_alloc (conditions, 4);

  // 简化实现：检查当前基本块和前驱块的条件
  basic_block bb = access->bb;

  // 检查前驱块
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE (e, ei, bb->preds) {
    basic_block pred_bb = e->src;
    gimple_stmt_iterator gsi = gsi_last_bb (pred_bb);
    if (!gsi_end_p (gsi)) {
      gimple* last_stm = gsi_stmt (gsi);
      if (last_stm && gimple_code (last_stm) == GIMPLE_COND) {
        vec_safe_push (conditions, last_stm);
      }
    }
  }

  *result = conditions;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析数组读取的边界条件
// ============================================================================

ArrayDetectErrorCode analyzeReadBoundCondition (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  ReadBoundCondition** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // 查找支配条件
  vec<gimple*, va_gc>* conditions = NULL;
  AD_TRY (findDominatingConditions (AD_ARGS, access, &conditions));

  if (!conditions || conditions->length () == 0) {
    AD_RETURNE (OK);
  }

  // 分析每个条件
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
    tree bound_field = NULL_TREE;
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

    // 在 bound_op 中查找整数字段
    for (tree field = TYPE_FIELDS (access->containing_type); field; field = DECL_CHAIN (field)) {
      if (TREE_CODE (field) != FIELD_DECL) continue;
      if (!isIntegerType (TREE_TYPE (field))) continue;

      if (exprReferencesField (AD_ARGS, bound_op, field)) {
        bound_field = field;
        break;
      }
    }

    if (bound_field) {
      // 创建 ReadBoundCondition
      ReadBoundCondition* bound_cond = ggc_alloc<ReadBoundCondition>();
      memset (bound_cond, 0, sizeof (ReadBoundCondition));

      bound_cond->access = access;
      bound_cond->condition_stmt = cond_stmt;
      bound_cond->comparison_code = code;
      bound_cond->index_operand = index_op;
      bound_cond->bound_operand = bound_op;
      bound_cond->has_field_bound = true;
      bound_cond->bound_field_decl = bound_field;
      bound_cond->bound_type = access->containing_type;
      bound_cond->condition_bb = gimple_bb (cond_stmt);
      bound_cond->dominates_access = true;
      bound_cond->location = gimple_location (cond_stmt);

      *result = bound_cond;
      AD_RETURNE (OK);
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 提取读容量证据
// ============================================================================

ArrayDetectErrorCode extractReadCapacityEvidence (
  AD_FUNC_ARGS,
  tree pointer_field,
  ReadBoundCondition* bound_cond,
  ReadCapacityEvidence** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!pointer_field || !bound_cond || !bound_cond->has_field_bound) {
    AD_RETURNE (OK);
  }

  ReadCapacityEvidence* evidence = ggc_alloc<ReadCapacityEvidence>();
  memset (evidence, 0, sizeof (ReadCapacityEvidence));

  evidence->pointer_field = pointer_field;
  evidence->integer_field = bound_cond->bound_field_decl;
  evidence->containing_type = bound_cond->bound_type;
  evidence->read_access = bound_cond->access;
  evidence->bound_condition = bound_cond;
  evidence->confidence = READ_CONF_CERTAIN;
  evidence->location = bound_cond->location;
  evidence->description = "Array read with field-based bound check";

  AD_DEBUG_PRINT ("[read-capacity] ptr '%s' -> bound field '%s'",
                  safeGetFieldName (AD_ARGS, pointer_field),
                  safeGetFieldName (AD_ARGS, bound_cond->bound_field_decl));

  *result = evidence;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 收集所有读容量证据
// ============================================================================

ArrayDetectErrorCode collectAllReadEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
) AD_FUNCTION_BEGIN {
  if (!tfad) {
    AD_RETURNE (OK);
  }

  // 初始化证据列表
  if (!tfad->read_evidences) {
    vec_alloc (tfad->read_evidences, 4);
  }

  // 遍历所有函数收集数组读取
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    function* fn = DECL_STRUCT_FUNCTION (node->decl);
    if (!fn) continue;

    push_cfun (fn);

    basic_block bb;
    FOR_EACH_BB_FN (bb, fn) {
      for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
           !gsi_end_p (gsi);
           gsi_next (&gsi)) {
        gimple* stmt = gsi_stmt (gsi);

        // 尝试收集数组读取
        ArrayReadAccess* access = NULL;
        ArrayDetectErrorCode err = collectArrayReadAccess (
          AD_ARGS, stmt, fn, tfad->type, tfad->field_decl, &access
        );

        if (err != OK || !access) continue;

        // 分析边界条件
        ReadBoundCondition* bound_cond = NULL;
        err = analyzeReadBoundCondition (AD_ARGS, access, &bound_cond);

        if (err != OK || !bound_cond) continue;

        // 提取证据
        ReadCapacityEvidence* evidence = NULL;
        err = extractReadCapacityEvidence (
          AD_ARGS, tfad->field_decl, bound_cond, &evidence
        );

        if (err == OK && evidence) {
          vec_safe_push (tfad->read_evidences, evidence);
        }
      }
    }

    pop_cfun ();
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 获取置信度名称
// ============================================================================

char const* readConfidenceToString (ReadEvidenceConfidence conf) {
  switch (conf) {
    case READ_CONF_CERTAIN:  return "certain";
    case READ_CONF_PROBABLE: return "probable";
    case READ_CONF_WEAK:     return "weak";
    default:                 return "unknown";
  }
}

// ============================================================================
// 打印读容量证据
// ============================================================================

void printReadCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  ReadCapacityEvidence* evidence
) {
  if (!out || !evidence) return;

  char const* ptr_name = safeGetFieldName (AD_ARGS, evidence->pointer_field);
  char const* int_name = safeGetFieldName (AD_ARGS, evidence->integer_field);

  fprintf (out, "  [read-evidence] %s -> %s confidence=%s\n",
           ptr_name,
           int_name,
           readConfidenceToString (evidence->confidence));
}

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
