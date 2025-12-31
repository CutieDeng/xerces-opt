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
// 判断字段是否是合理的容量/索引边界字段
// ============================================================================

static bool isCapacityField (tree field_decl) {
  if (!field_decl || TREE_CODE (field_decl) != FIELD_DECL) {
    return false;
  }

  // 获取字段名
  tree decl_name = DECL_NAME (field_decl);
  if (!decl_name) {
    return false;
  }
  const char* field_name = IDENTIFIER_POINTER (decl_name);
  if (!field_name) {
    return false;
  }

  // 排除 vptr 字段（虚表指针）
  if (strstr (field_name, "_vptr") || strstr (field_name, "vptr")) {
    return false;
  }

  // 获取字段类型
  tree field_type = TREE_TYPE (field_decl);
  if (!field_type) {
    return false;
  }

  // 只接受整数类型作为容量字段
  if (!INTEGRAL_TYPE_P (field_type)) {
    return false;
  }

  // 排除布尔类型（如 fCallDestructor）
  if (TREE_CODE (field_type) == BOOLEAN_TYPE) {
    return false;
  }

  // 进一步检查：如果字段名包含某些关键词，更可能是容量字段
  // 但我们不强制要求，因为可能有各种命名约定

  return true;
}

// ============================================================================
// 辅助函数：从 COMPONENT_REF 表达式中提取基础对象
// 返回最内层的基础对象（去掉所有 COMPONENT_REF 链）
// ============================================================================

static tree extractBaseObject (tree expr, int depth = 0) {
  if (!expr || depth > 10) {
    return NULL_TREE;
  }

  // COMPONENT_REF: 递归获取基础对象
  if (TREE_CODE (expr) == COMPONENT_REF) {
    return extractBaseObject (TREE_OPERAND (expr, 0), depth + 1);
  }

  // SSA_NAME: 追溯定义
  if (TREE_CODE (expr) == SSA_NAME) {
    // 如果有基础变量声明，直接返回
    tree base_var = SSA_NAME_VAR (expr);
    if (base_var) {
      return base_var;
    }

    // 追溯定义语句
    gimple* def = SSA_NAME_DEF_STMT (expr);
    if (def && gimple_code (def) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def);
      enum tree_code rhs_code = gimple_assign_rhs_code (def);

      // 类型转换或简单复制
      if (rhs_code == SSA_NAME || CONVERT_EXPR_CODE_P (rhs_code) ||
          rhs_code == NOP_EXPR || rhs_code == VIEW_CONVERT_EXPR) {
        return extractBaseObject (rhs, depth + 1);
      }

      // COMPONENT_REF 赋值
      if (TREE_CODE (rhs) == COMPONENT_REF) {
        return extractBaseObject (rhs, depth + 1);
      }

      // MEM_REF: 内存引用
      if (TREE_CODE (rhs) == MEM_REF) {
        return extractBaseObject (TREE_OPERAND (rhs, 0), depth + 1);
      }
    }
    return expr; // 返回 SSA_NAME 本身
  }

  // MEM_REF: 继续追溯基址
  if (TREE_CODE (expr) == MEM_REF) {
    return extractBaseObject (TREE_OPERAND (expr, 0), depth + 1);
  }

  // ADDR_EXPR: 取地址表达式
  if (TREE_CODE (expr) == ADDR_EXPR) {
    return extractBaseObject (TREE_OPERAND (expr, 0), depth + 1);
  }

  // 类型转换
  if (CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR ||
      TREE_CODE (expr) == VIEW_CONVERT_EXPR) {
    return extractBaseObject (TREE_OPERAND (expr, 0), depth + 1);
  }

  // VAR_DECL, PARM_DECL, INDIRECT_REF 等：直接返回
  return expr;
}

// ============================================================================
// 辅助函数：检查两个对象是否同一或可能别名
// 用于判断边界字段是否与目标指针字段属于同一对象
// ============================================================================

static bool objectsAreSameOrAliased (AD_FUNC_ARGS, tree obj1, tree obj2) {
  AD_ARGS_WARN_DENY;
  if (!obj1 || !obj2) {
    return false;
  }

  // 完全相等
  if (obj1 == obj2) {
    return true;
  }

  // 提取基础对象
  tree base1 = extractBaseObject (obj1);
  tree base2 = extractBaseObject (obj2);

  if (!base1 || !base2) {
    return false;
  }

  // 基础对象相等
  if (base1 == base2) {
    return true;
  }

  // 检查是否来自同一变量声明
  // 对于 SSA_NAME，检查 SSA_NAME_VAR
  tree var1 = (TREE_CODE (base1) == SSA_NAME) ? SSA_NAME_VAR (base1) : base1;
  tree var2 = (TREE_CODE (base2) == SSA_NAME) ? SSA_NAME_VAR (base2) : base2;

  if (var1 && var2 && var1 == var2) {
    return true;
  }

  // 检查类型匹配（同一类型的 this 指针通常指向同一对象）
  // 对于成员函数中的 this 指针，类型相同时认为可能是同一对象
  tree type1 = TREE_TYPE (base1);
  tree type2 = TREE_TYPE (base2);

  if (type1 && type2) {
    // 处理指针类型
    if (TREE_CODE (type1) == POINTER_TYPE) type1 = TREE_TYPE (type1);
    if (TREE_CODE (type2) == POINTER_TYPE) type2 = TREE_TYPE (type2);

    // 处理引用类型
    if (type1 && TREE_CODE (type1) == REFERENCE_TYPE) type1 = TREE_TYPE (type1);
    if (type2 && TREE_CODE (type2) == REFERENCE_TYPE) type2 = TREE_TYPE (type2);

    if (type1 && type2) {
      // 去掉 const/volatile 限定符
      type1 = TYPE_MAIN_VARIANT (type1);
      type2 = TYPE_MAIN_VARIANT (type2);

      // 类型相同，检查是否都是 PARM_DECL（函数参数，如 this）
      if (type1 == type2) {
        // 如果两者都是同一函数的参数（如 this 指针），认为同一
        if ((TREE_CODE (var1) == PARM_DECL && TREE_CODE (var2) == PARM_DECL) ||
            (TREE_CODE (base1) == PARM_DECL && TREE_CODE (base2) == PARM_DECL)) {
          return true;
        }
      }
    }
  }

  return false;
}

// ============================================================================
// 检查表达式是否涉及指定的索引变量
// ============================================================================

static bool expressionInvolvesVar (tree expr, tree index_var, int depth = 0) {
  if (!expr || !index_var || depth > 10) {
    return false;
  }

  // 直接匹配
  if (expr == index_var) {
    return true;
  }

  // SSA_NAME: 追溯定义
  if (TREE_CODE (expr) == SSA_NAME) {
    if (TREE_CODE (index_var) == SSA_NAME) {
      // 检查是否来自同一基础变量
      tree expr_var = SSA_NAME_VAR (expr);
      tree index_base = SSA_NAME_VAR (index_var);
      if (expr_var && index_base && expr_var == index_base) {
        return true;
      }
    }

    // 追溯定义语句
    gimple* def = SSA_NAME_DEF_STMT (expr);
    if (def && gimple_code (def) == GIMPLE_ASSIGN) {
      if (expressionInvolvesVar (gimple_assign_rhs1 (def), index_var, depth + 1)) {
        return true;
      }
      tree rhs2 = gimple_assign_rhs2 (def);
      if (rhs2 && expressionInvolvesVar (rhs2, index_var, depth + 1)) {
        return true;
      }
    }
    return false;
  }

  // 二元操作
  if (BINARY_CLASS_P (expr)) {
    return expressionInvolvesVar (TREE_OPERAND (expr, 0), index_var, depth + 1) ||
           expressionInvolvesVar (TREE_OPERAND (expr, 1), index_var, depth + 1);
  }

  // 一元操作或类型转换
  if (UNARY_CLASS_P (expr) || CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    return expressionInvolvesVar (TREE_OPERAND (expr, 0), index_var, depth + 1);
  }

  return false;
}

// ============================================================================
// 从表达式中收集边界字段（仅收集非索引侧的字段）
// 增强：添加目标对象检查，只收集与目标对象同一的字段
// ============================================================================

static void collectBoundFieldsFromExpression (
  AD_FUNC_ARGS,
  tree expr,
  tree index_var,
  tree target_base_object,           // 目标对象（待分析指针字段的基础对象）
  vec<tree, va_gc>** out_fields,
  int depth = 0
) {
  if (!expr || depth > 10) return;

  // 如果这个表达式涉及索引变量，不应从中收集字段
  // （因为它是"索引侧"，不是"边界侧"）
  if (expressionInvolvesVar (expr, index_var, 0)) {
    return;
  }

  // COMPONENT_REF: 直接的字段访问
  if (TREE_CODE (expr) == COMPONENT_REF) {
    tree field = TREE_OPERAND (expr, 1);
    tree base_obj = TREE_OPERAND (expr, 0);

    if (field && TREE_CODE (field) == FIELD_DECL) {
      // 只收集可能作为容量的字段
      if (isCapacityField (field)) {
        // 检查字段所属对象是否与目标对象同一
        bool same_object = true;  // 默认允许（向后兼容）
        if (target_base_object) {
          same_object = objectsAreSameOrAliased (AD_ARGS, base_obj, target_base_object);
          if (!same_object) {
            AD_DEBUG_PRINT ("[collectBoundFieldsFromExpression] Skipping field %s: different object",
                            safeGetFieldName (AD_ARGS, field));
          }
        }

        if (same_object) {
          vec_safe_push (*out_fields, field);
          AD_DEBUG_PRINT ("[collectBoundFieldsFromExpression] Collected field %s",
                          safeGetFieldName (AD_ARGS, field));
        }
      }
    }
    // 继续检查基础对象（可能有嵌套字段访问）
    collectBoundFieldsFromExpression (AD_ARGS, base_obj, index_var, target_base_object, out_fields, depth + 1);
    return;
  }

  // SSA_NAME: 追溯定义
  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def = SSA_NAME_DEF_STMT (expr);
    if (!def) return;

    if (gimple_code (def) == GIMPLE_ASSIGN) {
      collectBoundFieldsFromExpression (AD_ARGS, gimple_assign_rhs1 (def), index_var, target_base_object, out_fields, depth + 1);
      tree rhs2 = gimple_assign_rhs2 (def);
      if (rhs2) {
        collectBoundFieldsFromExpression (AD_ARGS, rhs2, index_var, target_base_object, out_fields, depth + 1);
      }
    }
    else if (gimple_code (def) == GIMPLE_PHI) {
      // PHI 节点：从所有输入中收集字段（循环变量的情况）
      gphi* phi = as_a<gphi*>(def);
      for (unsigned i = 0; i < gimple_phi_num_args (phi); i++) {
        tree arg = gimple_phi_arg_def (phi, i);
        if (arg) {
          collectBoundFieldsFromExpression (AD_ARGS, arg, index_var, target_base_object, out_fields, depth + 1);
        }
      }
    }
    return;
  }

  // 二元操作
  if (BINARY_CLASS_P (expr)) {
    collectBoundFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), index_var, target_base_object, out_fields, depth + 1);
    collectBoundFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 1), index_var, target_base_object, out_fields, depth + 1);
    return;
  }

  // 一元操作或类型转换
  if (UNARY_CLASS_P (expr) || CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    collectBoundFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), index_var, target_base_object, out_fields, depth + 1);
    return;
  }

  // MEM_REF: 内存引用
  if (TREE_CODE (expr) == MEM_REF) {
    collectBoundFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), index_var, target_base_object, out_fields, depth + 1);
    return;
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

// 辅助函数：获取 SSA_NAME 的基础变量（追溯到原始 PARM_DECL 或 VAR_DECL）
static tree getSSABaseVar (tree ssa) {
  if (!ssa) return NULL_TREE;
  if (TREE_CODE (ssa) != SSA_NAME) return ssa;

  // 首先尝试获取 SSA_NAME_VAR
  tree base = SSA_NAME_VAR (ssa);
  if (base) return base;

  // 如果没有基础变量，追溯定义语句
  gimple* def = SSA_NAME_DEF_STMT (ssa);
  if (!def) return NULL_TREE;

  // 赋值语句：追溯 RHS
  if (gimple_code (def) == GIMPLE_ASSIGN) {
    tree rhs = gimple_assign_rhs1 (def);
    if (rhs && TREE_CODE (rhs) == SSA_NAME) {
      return getSSABaseVar (rhs);
    }
    // NOP_EXPR 或类型转换
    if (CONVERT_EXPR_CODE_P (gimple_assign_rhs_code (def))) {
      return getSSABaseVar (rhs);
    }
  }

  return NULL_TREE;
}

// 辅助函数：比较两个 SSA_NAME 是否来自同一基础变量
static bool sameSSABaseVar (tree a, tree b) {
  if (!a || !b) return false;
  if (a == b) return true;

  tree base_a = getSSABaseVar (a);
  tree base_b = getSSABaseVar (b);

  // 两边都有基础变量且相同
  if (base_a && base_b && base_a == base_b) return true;

  // 一边有基础变量，另一边是 SSA，检查 SSA 是否来自该基础变量
  if (base_a && TREE_CODE (b) == SSA_NAME) {
    tree b_base = getSSABaseVar (b);
    if (b_base && b_base == base_a) return true;
  }
  if (base_b && TREE_CODE (a) == SSA_NAME) {
    tree a_base = getSSABaseVar (a);
    if (a_base && a_base == base_b) return true;
  }

  return false;
}

bool expressionInvolvesIndex (
  AD_FUNC_ARGS,
  tree expr,
  tree index_var
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || !index_var) return false;

  // 直接相等
  if (expr == index_var) return true;

  // 比较 SSA 基础变量
  if (sameSSABaseVar (expr, index_var)) return true;

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
// 辅助函数：从表达式中收集所有引用的字段
// ============================================================================

static void collectFieldsFromExpression (
  AD_FUNC_ARGS,
  tree expr,
  vec<tree, va_gc>** out_fields,
  int depth = 0
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || depth > 10) return;

  // COMPONENT_REF: 直接的字段访问
  if (TREE_CODE (expr) == COMPONENT_REF) {
    tree field = TREE_OPERAND (expr, 1);
    if (field && TREE_CODE (field) == FIELD_DECL) {
      vec_safe_push (*out_fields, field);
    }
    // 继续检查基础对象
    collectFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), out_fields, depth + 1);
    return;
  }

  // SSA_NAME: 追溯定义
  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def = SSA_NAME_DEF_STMT (expr);
    if (def && gimple_code (def) == GIMPLE_ASSIGN) {
      collectFieldsFromExpression (AD_ARGS, gimple_assign_rhs1 (def), out_fields, depth + 1);
      tree rhs2 = gimple_assign_rhs2 (def);
      if (rhs2) {
        collectFieldsFromExpression (AD_ARGS, rhs2, out_fields, depth + 1);
      }
    }
    return;
  }

  // 二元操作
  if (BINARY_CLASS_P (expr)) {
    collectFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), out_fields, depth + 1);
    collectFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 1), out_fields, depth + 1);
    return;
  }

  // 一元操作或类型转换
  if (UNARY_CLASS_P (expr) || CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    collectFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), out_fields, depth + 1);
    return;
  }

  // MEM_REF: 内存引用
  if (TREE_CODE (expr) == MEM_REF) {
    collectFieldsFromExpression (AD_ARGS, TREE_OPERAND (expr, 0), out_fields, depth + 1);
    return;
  }
}

// ============================================================================
// 分析条件中引用的字段（仅收集与索引语义相关的边界字段）
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

  // 检查是否是比较操作（边界检查通常是 <, <=, >, >=）
  if (cmp_code != LT_EXPR && cmp_code != LE_EXPR &&
      cmp_code != GT_EXPR && cmp_code != GE_EXPR &&
      cmp_code != NE_EXPR && cmp_code != EQ_EXPR) {
    AD_RETURNE (OK);
  }

  // 检查条件是否涉及索引变量
  bool lhs_involves_index = index_var && expressionInvolvesVar (lhs, index_var, 0);
  bool rhs_involves_index = index_var && expressionInvolvesVar (rhs, index_var, 0);

  if (!lhs_involves_index && !rhs_involves_index) {
    // 条件不涉及索引变量，不是边界检查
    AD_RETURNE (OK);
  }

  // 提取目标对象（用于过滤不属于同一对象的边界字段）
  tree target_base_object = NULL_TREE;
  if (access && access->is_field_based && access->base_pointer) {
    target_base_object = extractBaseObject (access->base_pointer);
  }

  // 收集边界字段（从非索引侧收集，并过滤非容量字段）
  vec<tree, va_gc>* fields = NULL;
  vec_alloc (fields, 4);

  if (!lhs_involves_index) {
    collectBoundFieldsFromExpression (AD_ARGS, lhs, index_var, target_base_object, &fields);
  }
  if (!rhs_involves_index) {
    collectBoundFieldsFromExpression (AD_ARGS, rhs, index_var, target_base_object, &fields);
  }

  // 如果没有找到任何字段引用，跳过
  if (vec_safe_length (fields) == 0) {
    AD_RETURNE (OK);
  }

  // 创建关联结构
  BoundConditionAssociation* assoc = ggc_alloc<BoundConditionAssociation>();
  memset (assoc, 0, sizeof (BoundConditionAssociation));

  assoc->condition_stmt = cond_stmt;
  assoc->condition_expr = lhs;
  assoc->comparison_code = cmp_code;
  assoc->condition_bb = gimple_bb (cond_stmt);
  assoc->location = gimple_location (cond_stmt);
  assoc->dominates_access = true;

  // 使用第一个找到的字段作为主要边界字段
  assoc->is_field_bound = true;
  assoc->bound_field_decl = (*fields)[0];

  // 追溯字段所属类型
  tree first_field = (*fields)[0];
  if (first_field && DECL_CONTEXT (first_field)) {
    assoc->bound_type = DECL_CONTEXT (first_field);
  }

  // 设置条件类型
  assoc->condition_type = BOUND_COND_LT_FIELD;  // 简化：统一标记为字段边界

  AD_DEBUG_PRINT ("[analyzeBoundCondition] Found %u fields in condition at %s:%d",
                  vec_safe_length (fields),
                  LOCATION_FILE (gimple_location (cond_stmt)) ?
                    LOCATION_FILE (gimple_location (cond_stmt)) : "<unknown>",
                  LOCATION_LINE (gimple_location (cond_stmt)));

  // 检查是否与访问的指针字段属于同一类型
  if (access && access->is_field_based && access->containing_type && assoc->bound_type) {
    if (TYPE_MAIN_VARIANT (access->containing_type) ==
        TYPE_MAIN_VARIANT (assoc->bound_type)) {
      assoc->description = "Same-object bound check";
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

  // 提取目标对象（待分析指针字段的基础对象）
  // 用于过滤不属于同一对象的边界字段
  tree target_base_object = NULL_TREE;
  if (access->is_field_based && access->base_pointer) {
    target_base_object = extractBaseObject (access->base_pointer);
  }

  // 查找支配条件
  vec<gimple*, va_gc>* conditions = NULL;
  AD_TRY (findDominatingConditions (AD_ARGS, access, &conditions));

  if (!conditions || conditions->length () == 0) {
    AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] No dominating conditions found");

    // 即使没有支配条件，也检查索引表达式是否引用了容量字段
    // 例如 fElemList[fCurCount] 中，fCurCount 本身就是边界字段
    if (index_var) {
      vec<tree, va_gc>* index_fields = NULL;
      vec_alloc (index_fields, 4);
      collectBoundFieldsFromExpression (AD_ARGS, index_var, NULL_TREE, target_base_object, &index_fields);

      // 将索引表达式中的字段添加到 related_fields
      for (unsigned int j = 0; j < vec_safe_length (index_fields); j++) {
        tree field = (*index_fields)[j];
        vec_safe_push (analysis->related_fields, field);
        analysis->field_bound_count++;
      }

      if (analysis->field_bound_count > 0) {
        analysis->has_valid_bound = true;
        AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] Found %u fields from index expression",
                        analysis->field_bound_count);
      }
    }

    access->bound_analysis = analysis;
    *out_analysis = analysis;
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] Found %u dominating conditions",
                  conditions->length ());

  // 分析每个条件，收集所有引用的字段
  BoundConditionAssociation* best_field_bound = NULL;

  // 检查 index_var 是否是变量（SSA_NAME）还是常量
  // 如果是常量，则采用宽松策略，从所有条件中收集边界字段
  bool index_is_variable = (index_var && TREE_CODE (index_var) == SSA_NAME);

  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "    index_var code=%s, is_variable=%d\n",
             index_var ? get_tree_code_name (TREE_CODE (index_var)) : "NULL",
             index_is_variable);
  }

  for (unsigned int i = 0; i < conditions->length (); i++) {
    gimple* cond = (*conditions)[i];

    if (gimple_code (cond) != GIMPLE_COND) continue;

    tree lhs = gimple_cond_lhs (cond);
    tree rhs = gimple_cond_rhs (cond);

    // 收集边界字段
    vec<tree, va_gc>* cond_fields = NULL;
    vec_alloc (cond_fields, 4);

    if (index_is_variable) {
      // 索引是变量：尝试严格模式
      bool lhs_involves_index = expressionInvolvesVar (lhs, index_var, 0);
      bool rhs_involves_index = expressionInvolvesVar (rhs, index_var, 0);

      if (ctx.debug_file) {
        fprintf (ctx.debug_file, "    [COND %u] code=%s, lhs_involves=%d, rhs_involves=%d\n",
                 i, get_tree_code_name (gimple_cond_code (cond)),
                 lhs_involves_index, rhs_involves_index);
      }

      if (lhs_involves_index || rhs_involves_index) {
        // 严格模式成功：只从不涉及索引的一侧收集字段
        if (!lhs_involves_index) {
          collectBoundFieldsFromExpression (AD_ARGS, lhs, index_var, target_base_object, &cond_fields);
        }
        if (!rhs_involves_index) {
          collectBoundFieldsFromExpression (AD_ARGS, rhs, index_var, target_base_object, &cond_fields);
        }
      } else {
        // 严格模式失败（SSA 版本不匹配）：回退到宽松模式
        // 从两侧收集整数类型字段，依赖 isCapacityField 过滤
        collectBoundFieldsFromExpression (AD_ARGS, lhs, NULL_TREE, target_base_object, &cond_fields);
        collectBoundFieldsFromExpression (AD_ARGS, rhs, NULL_TREE, target_base_object, &cond_fields);
      }
    } else {
      // 索引是常量：宽松模式 - 从所有比较条件中收集整数类型字段
      // 仍然过滤非容量字段（vptr、布尔等）
      if (ctx.debug_file) {
        fprintf (ctx.debug_file, "    [COND %u] code=%s (relaxed mode)\n",
                 i, get_tree_code_name (gimple_cond_code (cond)));
      }

      // 从两侧都收集字段（因为我们不知道哪边是"索引侧"）
      collectBoundFieldsFromExpression (AD_ARGS, lhs, NULL_TREE, target_base_object, &cond_fields);
      collectBoundFieldsFromExpression (AD_ARGS, rhs, NULL_TREE, target_base_object, &cond_fields);
    }

    // 将所有字段添加到 related_fields（去重）
    for (unsigned int j = 0; j < vec_safe_length (cond_fields); j++) {
      tree field = (*cond_fields)[j];
      bool already_exists = false;
      for (unsigned int k = 0; k < vec_safe_length (analysis->related_fields); k++) {
        if ((*analysis->related_fields)[k] == field) {
          already_exists = true;
          break;
        }
      }
      if (!already_exists) {
        vec_safe_push (analysis->related_fields, field);
        analysis->field_bound_count++;
      }
    }

    // 也调用 analyzeBoundCondition 来获取更多信息（用于 debug）
    BoundConditionAssociation* assoc = NULL;
    AD_TRY (analyzeBoundCondition (AD_ARGS, cond, index_var, access, &assoc));

    if (assoc) {
      vec_safe_push (analysis->bounds, assoc);

      if (assoc->is_field_bound) {
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

  // 后备检查：如果从支配条件中没有收集到任何字段，尝试从索引表达式中收集
  // 例如析构函数中 for (i = fMaxCount - 1; i >= 0; i--) 的情况
  // 条件 i >= 0 没有字段引用，但索引 i 的初始值引用了 fMaxCount
  if (analysis->field_bound_count == 0 && index_var) {
    vec<tree, va_gc>* index_fields = NULL;
    vec_alloc (index_fields, 4);
    collectBoundFieldsFromExpression (AD_ARGS, index_var, NULL_TREE, target_base_object, &index_fields);

    for (unsigned int j = 0; j < vec_safe_length (index_fields); j++) {
      tree field = (*index_fields)[j];
      bool already_exists = false;
      for (unsigned int k = 0; k < vec_safe_length (analysis->related_fields); k++) {
        if ((*analysis->related_fields)[k] == field) {
          already_exists = true;
          break;
        }
      }
      if (!already_exists) {
        vec_safe_push (analysis->related_fields, field);
        analysis->field_bound_count++;
      }
    }

    if (analysis->field_bound_count > 0) {
      AD_DEBUG_PRINT ("[analyzeAccessBoundConditions] Fallback: found %u fields from index expression",
                      analysis->field_bound_count);
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

    // 跳过 vptr 字段（虚表指针不是真正的数组）
    if (entry->field_name && strstr (entry->field_name, "_vptr")) {
      continue;
    }

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
