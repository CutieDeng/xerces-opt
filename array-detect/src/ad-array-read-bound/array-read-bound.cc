// ============================================================================
// ad-array-read-bound 模块实现
// ============================================================================
// 分析数组读取访问的边界条件，寻找关联整数字段
// ============================================================================

#include "array-read-bound.hh"
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

ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<gimple*, va_gc>** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!access || !access->bb || !access->fn) {
    AD_RETURNE (OK);
  }

  // 获取源码位置用于调试
  char loc_buf[256];
  gcc_ext_util::get_source_location_string (AD_ARGS, access->location, loc_buf, sizeof(loc_buf));
  AD_DEBUG_PRINT ("[read-bound] findDominatingConditions: access at %s, bb=%d",
                  loc_buf, access->bb->index);

  // 简化实现：检查当前基本块和前驱块的条件
  basic_block bb = access->bb;

  // 检查前驱块
  edge e;
  edge_iterator ei;
  unsigned pred_count = 0;
  unsigned cond_count = 0;
  FOR_EACH_EDGE (e, ei, bb->preds) {
    pred_count++;
    basic_block pred_bb = e->src;
    gimple_stmt_iterator gsi = gsi_last_bb (pred_bb);
    if (!gsi_end_p (gsi)) {
      gimple* last_stmt = gsi_stmt (gsi);
      if (last_stmt && gimple_code (last_stmt) == GIMPLE_COND) {
        cond_count++;
        // 延迟初始化
        if (!*result) {
          vec_alloc (*result, 4);
        }
        vec_safe_push (*result, last_stmt);
      }
    }
  }

  AD_DEBUG_PRINT ("[read-bound] findDominatingConditions: %u predecessors, %u conditions found",
                  pred_count, cond_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析数组读取的所有边界条件（一对多）
// ============================================================================

ArrayDetectErrorCode analyzeReadBoundConditions (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<ReadBoundCondition*, va_gc>** results
) AD_FUNCTION_BEGIN {
  *results = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // 查找支配条件
  vec<gimple*, va_gc>* conditions = NULL;
  AD_TRY (findDominatingConditions (AD_ARGS, access, &conditions));

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
        // 创建 ReadBoundCondition
        ReadBoundCondition* bound_cond = ggc_alloc<ReadBoundCondition>();
        memset (bound_cond, 0, sizeof (ReadBoundCondition));

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
// 打印读边界条件
// ============================================================================

void printReadBoundCondition (
  AD_FUNC_ARGS,
  FILE* out,
  ReadBoundCondition* bound_cond
) {
  if (!out || !bound_cond) return;

  char const* bound_name = bound_cond->has_field_bound
    ? safeGetFieldName (AD_ARGS, bound_cond->bound_field_decl)
    : "(none)";

  fprintf (out, "  [read-bound] bound_field='%s' at %s:%d\n",
           bound_name,
           LOCATION_FILE (bound_cond->location) ? LOCATION_FILE (bound_cond->location) : "?",
           LOCATION_LINE (bound_cond->location));
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

void printAllReadCapacityEvidences (
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

// ============================================================================
// 单项分析函数：从单个 ArrayReadAccess 生成所有 ReadCapacityEvidence
// ============================================================================

ArrayDetectErrorCode analyzeReadAccessToEvidences (
  AD_FUNC_ARGS,
  ArrayReadAccess* access,
  vec<ReadCapacityEvidence*, va_gc>** results
) AD_FUNCTION_BEGIN {
  *results = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // Step 1: 分析所有边界条件
  vec<ReadBoundCondition*, va_gc>* bound_conds = NULL;
  AD_TRY (analyzeReadBoundConditions (AD_ARGS, access, &bound_conds));

  // Step 2: 为每个边界条件提取证据
  if (bound_conds) {
    for (unsigned i = 0; i < bound_conds->length (); i++) {
      ReadBoundCondition* bound_cond = (*bound_conds)[i];
      if (!bound_cond || !bound_cond->has_field_bound) continue;

      // 使用 access 中的 pointer_field_decl 作为 pointer_field
      ReadCapacityEvidence* evidence = NULL;
      AD_TRY (extractReadCapacityEvidence (
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
  }

  // Step 3: 检查 index 表达式是否直接引用整数字段
  // 例如: x = fElemList[fCurCount]; 其中 index 就是 fCurCount
  // 同时也检查 base_pointer，因为 GIMPLE 可能把 ptr[i] 转换为 *(ptr + i*size)
  tree index_to_check = access->index_expr;

  // 如果 index_expr 是常量，尝试从 base_pointer 的 POINTER_PLUS_EXPR 中提取真正的索引
  if (index_to_check && TREE_CODE (index_to_check) == INTEGER_CST) {
    // index_expr 是常量偏移，真正的索引在 base_pointer 的 SSA 链中
    tree base = access->base_pointer;
    int depth = 0;
    while (base && TREE_CODE (base) == SSA_NAME && depth < 10) {
      gimple* def = SSA_NAME_DEF_STMT (base);
      if (!def || !is_gimple_assign (def)) break;

      // 检查 gimple assign 的操作码
      enum tree_code rhs_code = gimple_assign_rhs_code (def);

      // 找到 POINTER_PLUS_EXPR
      if (rhs_code == POINTER_PLUS_EXPR) {
        index_to_check = gimple_assign_rhs2 (def);  // 获取偏移部分（第二个操作数）
        AD_DEBUG_PRINT ("[read-bound] found POINTER_PLUS_EXPR at depth=%d, index code=%d",
                        depth, TREE_CODE (index_to_check));
        break;
      }

      // 获取 rhs1 继续追溯
      tree rhs = gimple_assign_rhs1 (def);
      if (!rhs) break;

      // 继续追溯 SSA 链
      if (TREE_CODE (rhs) == SSA_NAME) {
        base = rhs;
        depth++;
        continue;
      }

      // 处理类型转换
      if (CONVERT_EXPR_P (rhs) || TREE_CODE (rhs) == NOP_EXPR) {
        tree inner = TREE_OPERAND (rhs, 0);
        if (inner && TREE_CODE (inner) == SSA_NAME) {
          base = inner;
          depth++;
          continue;
        }
      }

      break;
    }
  }

  if (index_to_check && access->containing_type) {
    AD_DEBUG_PRINT ("[read-bound] Step 3: checking index_to_check (code=%d) for field refs",
                    TREE_CODE (index_to_check));

    for (tree field = TYPE_FIELDS (access->containing_type); field; field = DECL_CHAIN (field)) {
      if (TREE_CODE (field) != FIELD_DECL) continue;
      if (!isIntegerType (TREE_TYPE (field))) continue;

      // 检查是否已经从边界条件中找到了这个字段
      bool already_found = false;
      if (*results) {
        for (unsigned i = 0; i < (*results)->length (); i++) {
          if ((**results)[i]->integer_field == field) {
            already_found = true;
            break;
          }
        }
      }
      if (already_found) continue;

      // 检查 index 表达式是否引用了这个字段
      if (exprReferencesField (AD_ARGS, index_to_check, field)) {
        AD_DEBUG_PRINT ("[read-capacity] ptr '%s' -> index field '%s' (direct index reference)",
                        safeGetFieldName (AD_ARGS, access->pointer_field_decl),
                        safeGetFieldName (AD_ARGS, field));

        ReadCapacityEvidence* evidence = ggc_alloc<ReadCapacityEvidence>();
        memset (evidence, 0, sizeof (ReadCapacityEvidence));

        evidence->pointer_field = access->pointer_field_decl;
        evidence->integer_field = field;
        evidence->containing_type = access->containing_type;
        evidence->read_access = access;
        evidence->bound_condition = NULL;  // 无边界条件，仅通过 index 引用
        evidence->confidence = READ_CONF_PROBABLE;  // 置信度稍低
        evidence->location = access->location;
        evidence->description = "Array index directly references integer field";

        if (!*results) {
          vec_alloc (*results, 4);
        }
        vec_safe_push (*results, evidence);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
