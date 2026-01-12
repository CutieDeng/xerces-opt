// ============================================================================
// ad-array-read-collect 模块实现
// ============================================================================
// 全程序扫描，收集所有数组读取访问，直接填充到已有的 Wrapper 中
// ============================================================================

#include "array-read-collect.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using ::array_detector::TypeFieldKey;
using ::array_detector::TypeFieldHashMapTraits;
using ::array_detector::TypeFieldAnalysisData;
using ::field_analysis::Wrapper_ArrayReadAccess_ReadBoundConditions;

// ============================================================================
// 检查表达式是否为数组读取
// ============================================================================

bool isArrayReadExpr (tree expr) {
  if (!expr) return false;

  enum tree_code code = TREE_CODE (expr);
  return (code == ARRAY_REF || code == MEM_REF);
}

// ============================================================================
// 追溯基础指针到字段访问 (内部函数)
// ============================================================================

static ArrayDetectErrorCode traceBasePointerToField (
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

        // POINTER_PLUS_EXPR: arr[i] 变成 arr + i*sizeof(elem)
        // 追溯第一个操作数（基指针）
        if (rhs_code == POINTER_PLUS_EXPR) {
          current = rhs;  // rhs1 是基指针
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
// 收集单个语句中的数组读取访问
// ============================================================================

ArrayDetectErrorCode collectArrayReadAccessFromStmt (
  AD_FUNC_ARGS,
  gimple* stmt,
  function* fn,
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
  AD_TRY (traceBasePointerToField (AD_ARGS, base_pointer, &found_type, &found_field));

  // 只收集基于字段的数组访问
  if (!found_type || !found_field) {
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
// 扫描整个程序，收集所有数组读取访问，直接填充到 Wrapper
// ============================================================================

ArrayDetectErrorCode scanAllArrayReadAccesses (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* type_field_map
) AD_FUNCTION_BEGIN {
  if (!type_field_map) {
    AD_RETURNE (OK);
  }

  unsigned total_accesses = 0;

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
        AD_TRY (collectArrayReadAccessFromStmt (AD_ARGS, stmt, fn, &access));

        if (access) {
          // 查找对应的 Wrapper
          TypeFieldKey key = { access->containing_type, access->pointer_field_decl };
          TypeFieldAnalysisData** slot = type_field_map->get (key);

          if (slot && *slot) {
            TypeFieldAnalysisData* wrapper = *slot;

            // 创建 Wrapper_ArrayReadAccess_ReadBoundConditions
            Wrapper_ArrayReadAccess_ReadBoundConditions* read_wrapper =
              ggc_alloc<Wrapper_ArrayReadAccess_ReadBoundConditions> ();
            memset (read_wrapper, 0, sizeof (Wrapper_ArrayReadAccess_ReadBoundConditions));
            read_wrapper->read_access = access;
            read_wrapper->bound_conditions = NULL;  // 后续由 bound 模块填充

            // 添加到 Wrapper 的 array_reads 字段
            vec_safe_push (wrapper->array_reads, read_wrapper);
            total_accesses++;
          }
        }
      }
    }

    pop_cfun ();
  }

  AD_DEBUG_PRINT ("scanAllArrayReadAccesses: collected %u accesses", total_accesses);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 打印数组读取访问
// ============================================================================

void printArrayReadAccess (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayReadAccess* access
) {
  if (!out || !access) return;

  char const* type_name = safeGetTypeName (AD_ARGS, access->containing_type);
  char const* field_name = safeGetFieldName (AD_ARGS, access->pointer_field_decl);

  fprintf (out, "  [array-read] %s.%s at %s:%d\n",
           type_name ? type_name : "?",
           field_name ? field_name : "?",
           LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "?",
           LOCATION_LINE (access->location));
}

// ============================================================================
// 打印特定 (type, field) 的数组读取访问列表
// ============================================================================

void printArrayReadAccessesForField (
  AD_FUNC_ARGS,
  FILE* out,
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>* wrappers
) {
  if (!out || !wrappers) return;

  fprintf (out, "=== Array Read Accesses (%u) ===\n",
           wrappers->length ());

  for (unsigned i = 0; i < wrappers->length (); i++) {
    Wrapper_ArrayReadAccess_ReadBoundConditions* w = (*wrappers)[i];
    if (w && w->read_access) {
      printArrayReadAccess (AD_ARGS, out, w->read_access);
    }
  }
}

} // namespace array_detect_ns
