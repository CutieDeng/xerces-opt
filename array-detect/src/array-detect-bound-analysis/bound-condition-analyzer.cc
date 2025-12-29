#include "bound-condition-analyzer.hh"
#include "gcc-ext-util.hh"
#include "info-print.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：安全获取类型名
// ============================================================================

static const char* safeGetTypeName (AD_FUNC_ARGS, tree type) {
  (void)ctx;
  (void)gcc_ctx;

  if (!type) return "<null-type>";

  tree type_id = TYPE_IDENTIFIER (type);
  if (!type_id) return "<anonymous-type>";

  const char* id_ptr = IDENTIFIER_POINTER (type_id);
  if (!id_ptr) return "<unnamed-type>";

  return identifier_to_locale (id_ptr);
}

// ============================================================================
// 辅助函数：安全获取字段名
// ============================================================================

static const char* safeGetFieldName (AD_FUNC_ARGS, tree field_decl) {
  (void)ctx;
  (void)gcc_ctx;

  if (!field_decl) return "<null-field>";

  tree decl_name = DECL_NAME (field_decl);
  if (!decl_name) return "<anonymous-field>";

  const char* id_ptr = IDENTIFIER_POINTER (decl_name);
  if (!id_ptr) return "<unnamed-field>";

  return identifier_to_locale (id_ptr);
}

// ============================================================================
// 获取边界条件类型名称
// ============================================================================

const char* getBoundConditionTypeName (BoundConditionType type) {
  switch (type) {
    case BOUND_COND_NONE: return "NONE";
    case BOUND_COND_LT_FIELD: return "LT_FIELD";
    case BOUND_COND_LE_FIELD: return "LE_FIELD";
    case BOUND_COND_LT_CONSTANT: return "LT_CONSTANT";
    case BOUND_COND_LE_CONSTANT: return "LE_CONSTANT";
    case BOUND_COND_GT_FIELD: return "GT_FIELD";
    case BOUND_COND_GE_FIELD: return "GE_FIELD";
    case BOUND_COND_COMPLEX: return "COMPLEX";
    default: return "UNKNOWN";
  }
}

// ============================================================================
// 规范化比较方向
// ============================================================================

ComparisonDirection normalizeComparison (
  tree_code code,
  tree lhs,
  tree rhs,
  tree index_var
) {
  bool lhs_is_index = (lhs == index_var);
  bool rhs_is_index = (rhs == index_var);

  // 如果需要追溯 SSA_NAME 来确定是否涉及索引
  if (!lhs_is_index && lhs && TREE_CODE (lhs) == SSA_NAME) {
    // 简化：暂不追溯，后续可以扩展
  }
  if (!rhs_is_index && rhs && TREE_CODE (rhs) == SSA_NAME) {
    // 简化：暂不追溯
  }

  switch (code) {
    case LT_EXPR:
      if (lhs_is_index) return CMP_INDEX_LT_BOUND;   // i < bound
      if (rhs_is_index) return CMP_BOUND_GT_INDEX;   // bound > i -> i < bound
      break;

    case LE_EXPR:
      if (lhs_is_index) return CMP_INDEX_LE_BOUND;   // i <= bound
      if (rhs_is_index) return CMP_BOUND_GE_INDEX;   // bound >= i -> i <= bound
      break;

    case GT_EXPR:
      if (lhs_is_index) return CMP_INDEX_GT_BOUND;   // i > bound
      if (rhs_is_index) return CMP_BOUND_GT_INDEX;   // bound > i
      break;

    case GE_EXPR:
      if (lhs_is_index) return CMP_INDEX_GE_BOUND;   // i >= bound
      if (rhs_is_index) return CMP_BOUND_GE_INDEX;   // bound >= i
      break;

    default:
      break;
  }

  return CMP_UNKNOWN;
}

// ============================================================================
// 检查表达式是否引用了索引变量
// ============================================================================

bool expressionInvolvesIndex (
  AD_FUNC_ARGS,
  tree expr,
  tree index_var
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || !index_var) return false;

  if (expr == index_var) return true;

  // SSA_NAME: 追溯定义
  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def_stmt = SSA_NAME_DEF_STMT (expr);
    if (!def_stmt) return false;

    if (gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs1 = gimple_assign_rhs1 (def_stmt);
      if (expressionInvolvesIndex (AD_ARGS, rhs1, index_var)) {
        return true;
      }

      tree rhs2 = gimple_assign_rhs2 (def_stmt);
      if (rhs2 && expressionInvolvesIndex (AD_ARGS, rhs2, index_var)) {
        return true;
      }
    }
    else if (gimple_code (def_stmt) == GIMPLE_PHI) {
      // PHI 节点：检查所有输入
      gphi* phi = as_a<gphi*>(def_stmt);
      for (unsigned i = 0; i < gimple_phi_num_args (phi); i++) {
        tree arg = gimple_phi_arg_def (phi, i);
        if (arg == index_var) return true;
      }
    }
  }

  // 二元操作
  if (BINARY_CLASS_P (expr)) {
    if (expressionInvolvesIndex (AD_ARGS, TREE_OPERAND (expr, 0), index_var))
      return true;
    if (expressionInvolvesIndex (AD_ARGS, TREE_OPERAND (expr, 1), index_var))
      return true;
  }

  // 一元操作
  if (UNARY_CLASS_P (expr)) {
    if (expressionInvolvesIndex (AD_ARGS, TREE_OPERAND (expr, 0), index_var))
      return true;
  }

  // 类型转换
  if (CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    if (expressionInvolvesIndex (AD_ARGS, TREE_OPERAND (expr, 0), index_var))
      return true;
  }

  return false;
}

// ============================================================================
// 追溯表达式到字段访问
// ============================================================================

ArrayDetectErrorCode traceExpressionToField (
  AD_FUNC_ARGS,
  tree expr,
  tree* out_type,
  tree* out_field_decl
) AD_FUNCTION_BEGIN {
  *out_type = NULL_TREE;
  *out_field_decl = NULL_TREE;

  if (!expr) {
    AD_RETURNE (OK);
  }

  tree current = expr;
  int depth = 0;
  const int MAX_DEPTH = 10;

  while (current && depth < MAX_DEPTH) {
    depth++;

    // COMPONENT_REF: 字段访问
    if (TREE_CODE (current) == COMPONENT_REF) {
      tree field = TREE_OPERAND (current, 1);
      tree base_obj = TREE_OPERAND (current, 0);

      if (field && TREE_CODE (field) == FIELD_DECL) {
        tree base_type = TREE_TYPE (base_obj);
        if (base_type && TREE_CODE (base_type) == RECORD_TYPE) {
          *out_type = TYPE_MAIN_VARIANT (base_type);
          *out_field_decl = field;

          AD_DEBUG_PRINT ("[traceExpressionToField] Found field: %s::%s",
                          safeGetTypeName (AD_ARGS, *out_type),
                          safeGetFieldName (AD_ARGS, *out_field_decl));
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

        // 检查是否是从字段读取
        if (rhs && TREE_CODE (rhs) == COMPONENT_REF) {
          current = rhs;
          continue;
        }

        // 类型转换
        enum tree_code rhs_code = gimple_assign_rhs_code (def_stmt);
        if (CONVERT_EXPR_CODE_P (rhs_code) || rhs_code == NOP_EXPR) {
          current = rhs;
          continue;
        }
      }
      break;
    }

    // MEM_REF
    if (TREE_CODE (current) == MEM_REF) {
      tree base = TREE_OPERAND (current, 0);
      current = base;
      continue;
    }

    // 类型转换
    if (CONVERT_EXPR_P (current) || TREE_CODE (current) == NOP_EXPR) {
      current = TREE_OPERAND (current, 0);
      continue;
    }

    break;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 查找支配该访问的条件语句
// ============================================================================

ArrayDetectErrorCode findDominatingConditions (
  AD_FUNC_ARGS,
  ArrayAccessCapture* access,
  vec<gimple*, va_gc>** out_conditions
) AD_FUNCTION_BEGIN {
  *out_conditions = NULL;

  if (!access || !access->bb || !access->fn) {
    AD_RETURNE (OK);
  }

  // 确保在正确的函数上下文中
  if (!cfun || cfun != access->fn) {
    AD_DEBUG_PRINT ("[findDominatingConditions] Wrong function context, skipping");
    AD_RETURNE (OK);
  }

  vec<gimple*, va_gc>* conditions = NULL;
  vec_alloc (conditions, 8);

  basic_block access_bb = access->bb;

  // 检查 CFG 是否可用
  if (!access->fn->cfg) {
    AD_DEBUG_PRINT ("[findDominatingConditions] No CFG available");
    *out_conditions = conditions;
    AD_RETURNE (OK);
  }

  // 使用 GCC 的支配树
  // 注意：需要先计算支配树
  if (!dom_info_available_p (CDI_DOMINATORS)) {
    calculate_dominance_info (CDI_DOMINATORS);
  }

  // 从当前块向上遍历支配者
  basic_block current_bb = access_bb;
  int depth = 0;
  const int MAX_DEPTH = 20;

  while (current_bb && depth < MAX_DEPTH) {
    depth++;

    // 获取支配者
    basic_block dominator = get_immediate_dominator (CDI_DOMINATORS, current_bb);
    if (!dominator || dominator == current_bb) break;

    // 检查支配者的最后一条语句
    gimple_stmt_iterator gsi = gsi_last_bb (dominator);
    if (!gsi_end_p (gsi)) {
      gimple* last_stmt = gsi_stmt (gsi);
      if (last_stmt && gimple_code (last_stmt) == GIMPLE_COND) {
        vec_safe_push (conditions, last_stmt);

        AD_DEBUG_PRINT ("[findDominatingConditions] Found condition at bb%d -> %s:%d",
                        dominator->index,
                        LOCATION_FILE (gimple_location (last_stmt)) ?
                          LOCATION_FILE (gimple_location (last_stmt)) : "<unknown>",
                        LOCATION_LINE (gimple_location (last_stmt)));
      }
    }

    current_bb = dominator;
  }

  *out_conditions = conditions;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析条件是否为边界检查
// ============================================================================

ArrayDetectErrorCode analyzeBoundCondition (
  AD_FUNC_ARGS,
  gimple* cond_stmt,
  tree index_var,
  ArrayAccessCapture* access,
  BoundConditionAssociation** out_association
) AD_FUNCTION_BEGIN {
  *out_association = NULL;

  if (!cond_stmt || gimple_code (cond_stmt) != GIMPLE_COND) {
    AD_RETURNE (OK);
  }

  tree_code cmp_code = gimple_cond_code (cond_stmt);
  tree lhs = gimple_cond_lhs (cond_stmt);
  tree rhs = gimple_cond_rhs (cond_stmt);

  if (!lhs || !rhs) {
    AD_RETURNE (OK);
  }

  // 检查是否是比较操作
  if (cmp_code != LT_EXPR && cmp_code != LE_EXPR &&
      cmp_code != GT_EXPR && cmp_code != GE_EXPR) {
    AD_RETURNE (OK);
  }

  // 检查是否涉及索引变量
  bool lhs_involves_index = expressionInvolvesIndex (AD_ARGS, lhs, index_var);
  bool rhs_involves_index = expressionInvolvesIndex (AD_ARGS, rhs, index_var);

  // 必须有一边涉及索引
  if (!lhs_involves_index && !rhs_involves_index) {
    AD_RETURNE (OK);
  }

  // 两边都涉及索引的情况暂不处理
  if (lhs_involves_index && rhs_involves_index) {
    AD_RETURNE (OK);
  }

  // 确定边界表达式（不涉及索引的一边）
  tree bound_expr = lhs_involves_index ? rhs : lhs;
  tree index_expr = lhs_involves_index ? lhs : rhs;

  // 创建关联结构
  BoundConditionAssociation* assoc = ggc_alloc<BoundConditionAssociation>();
  memset (assoc, 0, sizeof (BoundConditionAssociation));

  assoc->condition_stmt = cond_stmt;
  assoc->condition_expr = gimple_cond_lhs (cond_stmt); // 整个条件
  assoc->comparison_code = cmp_code;
  assoc->index_var = index_var;
  assoc->index_origin = index_expr;
  assoc->bound_expr = bound_expr;
  assoc->condition_bb = gimple_bb (cond_stmt);
  assoc->location = gimple_location (cond_stmt);
  assoc->dominates_access = true; // 已知是支配者

  // 检查边界是否为常量
  if (TREE_CODE (bound_expr) == INTEGER_CST) {
    assoc->is_field_bound = false;
    assoc->constant_bound = TREE_INT_CST_LOW (bound_expr);

    // 确定条件类型
    ComparisonDirection dir = normalizeComparison (cmp_code, lhs, rhs, index_var);
    switch (dir) {
      case CMP_INDEX_LT_BOUND:
      case CMP_BOUND_GT_INDEX:
        assoc->condition_type = BOUND_COND_LT_CONSTANT;
        break;
      case CMP_INDEX_LE_BOUND:
      case CMP_BOUND_GE_INDEX:
        assoc->condition_type = BOUND_COND_LE_CONSTANT;
        break;
      default:
        assoc->condition_type = BOUND_COND_COMPLEX;
    }

    AD_DEBUG_PRINT ("[analyzeBoundCondition] Found constant bound: %ld, type: %s",
                    (long)assoc->constant_bound,
                    getBoundConditionTypeName (assoc->condition_type));
  }
  else {
    // 尝试追溯到字段
    tree bound_type = NULL_TREE;
    tree bound_field = NULL_TREE;
    AD_TRY (traceExpressionToField (AD_ARGS, bound_expr, &bound_type, &bound_field));

    if (bound_type && bound_field) {
      assoc->is_field_bound = true;
      assoc->bound_type = bound_type;
      assoc->bound_field_decl = bound_field;

      // 确定条件类型
      ComparisonDirection dir = normalizeComparison (cmp_code, lhs, rhs, index_var);
      switch (dir) {
        case CMP_INDEX_LT_BOUND:
        case CMP_BOUND_GT_INDEX:
          assoc->condition_type = BOUND_COND_LT_FIELD;
          break;
        case CMP_INDEX_LE_BOUND:
        case CMP_BOUND_GE_INDEX:
          assoc->condition_type = BOUND_COND_LE_FIELD;
          break;
        case CMP_INDEX_GT_BOUND:
          assoc->condition_type = BOUND_COND_GT_FIELD;
          break;
        case CMP_INDEX_GE_BOUND:
          assoc->condition_type = BOUND_COND_GE_FIELD;
          break;
        default:
          assoc->condition_type = BOUND_COND_COMPLEX;
      }

      AD_DEBUG_PRINT ("[analyzeBoundCondition] Found field bound: %s::%s, type: %s",
                      safeGetTypeName (AD_ARGS, bound_type),
                      safeGetFieldName (AD_ARGS, bound_field),
                      getBoundConditionTypeName (assoc->condition_type));

      // 检查边界字段是否与访问的指针字段属于同一类型
      if (access && access->is_field_based && access->containing_type) {
        if (TYPE_MAIN_VARIANT (access->containing_type) ==
            TYPE_MAIN_VARIANT (bound_type)) {
          assoc->description = "Same-object bound check";
          AD_DEBUG_PRINT ("[analyzeBoundCondition] Bound is from same object type");
        }
      }
    }
    else {
      assoc->is_field_bound = false;
      assoc->condition_type = BOUND_COND_COMPLEX;
    }
  }

  *out_association = assoc;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析单个数组访问的边界条件
// ============================================================================

ArrayDetectErrorCode analyzeAccessBoundConditions (
  AD_FUNC_ARGS,
  ArrayAccessCapture* access,
  ArrayAccessBoundAnalysis** out_analysis
) AD_FUNCTION_BEGIN {
  *out_analysis = NULL;

  if (!access) {
    AD_RETURNE (OK);
  }

  // 创建分析结果
  ArrayAccessBoundAnalysis* analysis = ggc_alloc<ArrayAccessBoundAnalysis>();
  memset (analysis, 0, sizeof (ArrayAccessBoundAnalysis));

  analysis->access = access;
  vec_alloc (analysis->bounds, 4);
  vec_alloc (analysis->related_fields, 4);

  // 获取偏移量表达式作为索引变量
  tree index_var = access->offset_expr;
  if (!index_var) {
    // 没有索引变量，无法分析边界
    access->bound_analysis = analysis;
    *out_analysis = analysis;
    AD_RETURNE (OK);
  }

  // 查找支配条件
  vec<gimple*, va_gc>* conditions = NULL;
  AD_TRY (findDominatingConditions (AD_ARGS, access, &conditions));

  if (!conditions || conditions->length () == 0) {
    AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] No dominating conditions found");
    access->bound_analysis = analysis;
    *out_analysis = analysis;
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] Found %u dominating conditions",
                  conditions->length ());

  // 分析每个条件
  BoundConditionAssociation* best_field_bound = NULL;

  for (unsigned int i = 0; i < conditions->length (); i++) {
    gimple* cond = (*conditions)[i];

    BoundConditionAssociation* assoc = NULL;
    AD_TRY (analyzeBoundCondition (AD_ARGS, cond, index_var, access, &assoc));

    if (assoc) {
      vec_safe_push (analysis->bounds, assoc);

      if (assoc->is_field_bound) {
        analysis->field_bound_count++;
        vec_safe_push (analysis->related_fields, assoc->bound_field_decl);

        // 选择最佳字段边界（优先选择同对象类型的）
        if (!best_field_bound) {
          best_field_bound = assoc;
        }
        else if (assoc->description && !best_field_bound->description) {
          best_field_bound = assoc;
        }
      }
      else if (assoc->condition_type == BOUND_COND_LT_CONSTANT ||
               assoc->condition_type == BOUND_COND_LE_CONSTANT) {
        analysis->constant_bound_count++;
      }
    }
  }

  // 设置分析结果
  analysis->has_valid_bound = (analysis->field_bound_count > 0 ||
                               analysis->constant_bound_count > 0);
  analysis->primary_bound = best_field_bound;

  // 关联到访问捕获
  access->bound_analysis = analysis;

  AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] Result: %u field bounds, %u constant bounds",
                  analysis->field_bound_count, analysis->constant_bound_count);

  *out_analysis = analysis;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析所有数组访问的边界条件
// ============================================================================

ArrayDetectErrorCode analyzeAllBoundConditions (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("[analyzeAllBoundConditions] Starting bound condition analysis");

  if (!array_accesses) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits> MapType;

  unsigned int total_analyzed = 0;
  unsigned int with_bounds = 0;

  // 按函数分组分析，确保正确的函数上下文
  for (MapType::iterator iter = array_accesses->begin ();
       iter != array_accesses->end ();
       ++iter) {
    TypeFieldArrayAccesses* entry = (*iter).second;
    if (!entry || !entry->accesses) continue;

    for (unsigned int i = 0; i < entry->accesses->length (); i++) {
      ArrayAccessCapture* access = (*entry->accesses)[i];
      if (!access || !access->fn) continue;

      // 设置正确的函数上下文
      function* current_fn = access->fn;
      if (current_fn && current_fn->cfg) {
        push_cfun (current_fn);

        ArrayAccessBoundAnalysis* analysis = NULL;
        ArrayDetectErrorCode err = analyzeAccessBoundConditions (AD_ARGS, access, &analysis);

        pop_cfun ();

        if (err == OK) {
          total_analyzed++;
          if (analysis && analysis->has_valid_bound) {
            with_bounds++;
          }
        }
      } else {
        // 函数没有 CFG，创建一个空的分析结果
        ArrayAccessBoundAnalysis* analysis = ggc_alloc<ArrayAccessBoundAnalysis>();
        memset (analysis, 0, sizeof (ArrayAccessBoundAnalysis));
        analysis->access = access;
        access->bound_analysis = analysis;
        total_analyzed++;
      }
    }
  }

  AD_DEBUG_PRINT ("[analyzeAllBoundConditions] Analyzed %u accesses, %u with bounds",
                  total_analyzed, with_bounds);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出：打印边界条件关联
// ============================================================================

void printBoundConditionAssociation (
  AD_FUNC_ARGS,
  FILE* out,
  BoundConditionAssociation* assoc
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out || !assoc) return;

  fprintf (out, "    [%s] at %s:%d\n",
           getBoundConditionTypeName (assoc->condition_type),
           LOCATION_FILE (assoc->location) ? LOCATION_FILE (assoc->location) : "<unknown>",
           LOCATION_LINE (assoc->location));

  if (assoc->is_field_bound) {
    fprintf (out, "      Bound: %s::%s\n",
             safeGetTypeName (AD_ARGS, assoc->bound_type),
             safeGetFieldName (AD_ARGS, assoc->bound_field_decl));
  }
  else if (assoc->condition_type == BOUND_COND_LT_CONSTANT ||
           assoc->condition_type == BOUND_COND_LE_CONSTANT) {
    fprintf (out, "      Bound: %ld (constant)\n", (long)assoc->constant_bound);
  }

  if (assoc->description) {
    fprintf (out, "      Note: %s\n", assoc->description);
  }
}

// ============================================================================
// 调试输出：打印数组访问边界分析
// ============================================================================

void printArrayAccessBoundAnalysis (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayAccessBoundAnalysis* analysis
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out || !analysis) return;

  fprintf (out, "  Bound Analysis:\n");
  fprintf (out, "    Has valid bound: %s\n", analysis->has_valid_bound ? "yes" : "no");
  fprintf (out, "    Field bounds: %u\n", analysis->field_bound_count);
  fprintf (out, "    Constant bounds: %u\n", analysis->constant_bound_count);

  if (analysis->bounds) {
    fprintf (out, "    Conditions (%u):\n", analysis->bounds->length ());
    for (unsigned int i = 0; i < analysis->bounds->length (); i++) {
      printBoundConditionAssociation (AD_ARGS, out, (*analysis->bounds)[i]);
    }
  }

  if (analysis->primary_bound) {
    fprintf (out, "    Primary bound: %s::%s\n",
             analysis->primary_bound->is_field_bound ?
               safeGetTypeName (AD_ARGS, analysis->primary_bound->bound_type) : "-",
             analysis->primary_bound->is_field_bound ?
               safeGetFieldName (AD_ARGS, analysis->primary_bound->bound_field_decl) : "-");
  }
}

// ============================================================================
// 调试输出：打印所有边界分析
// ============================================================================

void printAllBoundAnalyses (
  AD_FUNC_ARGS,
  FILE* out,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out) return;

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "                    BOUND CONDITION ANALYSIS RESULTS\n");
  fprintf (out, "================================================================================\n");

  if (!array_accesses) {
    fprintf (out, "No array accesses to analyze.\n");
    return;
  }

  typedef hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits> MapType;

  unsigned int total_with_bounds = 0;
  unsigned int total_without_bounds = 0;

  for (MapType::iterator iter = array_accesses->begin ();
       iter != array_accesses->end ();
       ++iter) {
    TypeFieldArrayAccesses* entry = (*iter).second;
    if (!entry) continue;

    fprintf (out, "\n=== %s::%s ===\n", entry->type_name, entry->field_name);

    if (!entry->accesses) {
      fprintf (out, "No accesses.\n");
      continue;
    }

    for (unsigned int i = 0; i < entry->accesses->length (); i++) {
      ArrayAccessCapture* access = (*entry->accesses)[i];
      if (!access) continue;

      fprintf (out, "\n[%u] %s access at %s:%d\n",
               i,
               access->direction == ACCESS_READ ? "READ" : "WRITE",
               LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "<unknown>",
               LOCATION_LINE (access->location));

      ArrayAccessBoundAnalysis* analysis =
        (ArrayAccessBoundAnalysis*)access->bound_analysis;

      if (analysis) {
        printArrayAccessBoundAnalysis (AD_ARGS, out, analysis);
        if (analysis->has_valid_bound) {
          total_with_bounds++;
        } else {
          total_without_bounds++;
        }
      } else {
        fprintf (out, "  No bound analysis available.\n");
        total_without_bounds++;
      }
    }
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "Summary:\n");
  fprintf (out, "  Accesses with bounds: %u\n", total_with_bounds);
  fprintf (out, "  Accesses without bounds: %u\n", total_without_bounds);
  fprintf (out, "================================================================================\n");
  fprintf (out, "\n");
}

} // namespace array_detect_ns
