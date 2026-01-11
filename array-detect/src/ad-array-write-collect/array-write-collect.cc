// ============================================================================
// ad-array-write-collect 模块实现
// ============================================================================
// 收集指定 (type, field) 对应的所有数组写入访问
// ============================================================================

#include "array-write-collect.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

// ============================================================================
// 检查表达式是否为数组写入
// ============================================================================

bool isArrayWriteExpr (tree expr) {
  if (!expr) return false;

  enum tree_code code = TREE_CODE (expr);
  return (code == ARRAY_REF || code == MEM_REF);
}

// ============================================================================
// 追溯基础指针到字段访问 (内部函数)
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
// 收集单个语句中的数组写入访问
// ============================================================================

ArrayDetectErrorCode collectArrayWriteAccess (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
  tree target_type,
  tree target_field,
  ArrayWriteAccess** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  if (!stmt || !is_gimple_assign (stmt)) {
    AD_RETURNE (OK);
  }

  // 检查 LHS 是否为数组访问 (写入)
  tree lhs = gimple_assign_lhs (stmt);
  if (!lhs) {
    AD_RETURNE (OK);
  }

  // 检查是否为数组访问
  if (!isArrayWriteExpr (lhs)) {
    AD_RETURNE (OK);
  }

  tree base_pointer = NULL_TREE;
  tree index_expr = NULL_TREE;

  // ARRAY_REF: ptr[i] = x
  if (TREE_CODE (lhs) == ARRAY_REF) {
    base_pointer = TREE_OPERAND (lhs, 0);
    index_expr = TREE_OPERAND (lhs, 1);
  }
  // MEM_REF: *(ptr + offset) = x
  else if (TREE_CODE (lhs) == MEM_REF) {
    base_pointer = TREE_OPERAND (lhs, 0);
    index_expr = TREE_OPERAND (lhs, 1);
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

  // 创建 ArrayWriteAccess
  ArrayWriteAccess* access = ggc_alloc<ArrayWriteAccess>();
  memset (access, 0, sizeof (ArrayWriteAccess));

  access->stmt = stmt;
  access->base_pointer = base_pointer;
  access->index_expr = index_expr;
  access->value_written = gimple_assign_rhs1 (stmt);
  access->element_type = TREE_TYPE (lhs);
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
// 收集指定 (type, field) 的所有数组写入访问
// ============================================================================

ArrayDetectErrorCode collectAllArrayWriteAccesses (
  AD_FUNC_ARGS,
  tree type,
  tree field,
  vec<ArrayWriteAccess*, va_gc>** results
) AD_FUNCTION_BEGIN {
  if (!results) {
    AD_RETURNE (OK);
  }

  // 遍历所有函数收集数组写入
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

        // 尝试收集数组写入
        ArrayWriteAccess* access = NULL;
        ArrayDetectErrorCode collect_err = collectArrayWriteAccess (
          AD_ARGS, stmt, fn, type, field, &access
        );

        if (collect_err != OK) {
          pop_cfun ();
          return collect_err;
        }

        if (access) {
          // 延迟初始化
          if (!*results) {
            vec_alloc (*results, 8);
          }
          vec_safe_push (*results, access);
        }
      }
    }

    pop_cfun ();
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 打印数组写入访问
// ============================================================================

void printArrayWriteAccess (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayWriteAccess* access
) {
  if (!out || !access) return;

  char const* field_name = safeGetFieldName (AD_ARGS, access->pointer_field_decl);

  fprintf (out, "  [array-write] field '%s' at %s:%d\n",
           field_name,
           LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "?",
           LOCATION_LINE (access->location));
}

// ============================================================================
// 打印所有数组写入访问
// ============================================================================

void printAllArrayWriteAccesses (
  AD_FUNC_ARGS,
  FILE* out,
  vec<ArrayWriteAccess*, va_gc>* accesses
) {
  if (!out || !accesses) return;

  fprintf (out, "=== Array Write Accesses (%u) ===\n",
           accesses->length ());

  for (unsigned i = 0; i < accesses->length (); i++) {
    printArrayWriteAccess (AD_ARGS, out, (*accesses)[i]);
  }
}

} // namespace array_detect_ns
