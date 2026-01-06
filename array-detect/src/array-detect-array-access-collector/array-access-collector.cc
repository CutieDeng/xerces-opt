#include "array-access-collector.hh"
#include "gcc-ext-util.hh"
#include "info-print.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：获取访问类型名称
// ============================================================================

static char const* getAccessTypeName (ArrayAccessType type) {
  switch (type) {
    case ACCESS_ARRAY_REF: return "ARRAY_REF";
    case ACCESS_MEM_REF: return "MEM_REF";
    case ACCESS_POINTER_PLUS: return "POINTER_PLUS";
    default: return "UNKNOWN";
  }
}

// ============================================================================
// 辅助函数：获取访问方向名称
// ============================================================================

static char const* getAccessDirectionName (AccessDirection dir) {
  switch (dir) {
    case ACCESS_READ: return "READ";
    case ACCESS_WRITE: return "WRITE";
    default: return "UNKNOWN";
  }
}

// ============================================================================
// 追溯基础指针到字段访问
// ============================================================================

ArrayDetectErrorCode traceBasePointerToField (
  AD_FUNC_ARGS,
  tree base_pointer,
  tree* out_type,
  tree* out_field_decl
) AD_FUNCTION_BEGIN {
  *out_type = NULL_TREE;
  *out_field_decl = NULL_TREE;

  // base_pointer 为 NULL 时，while 循环自然不执行，直接返回
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
        // 检查是否是从字段读取
        tree rhs = gimple_assign_rhs1 (def_stmt);
        if (rhs && TREE_CODE (rhs) == COMPONENT_REF) {
          current = rhs;
          continue;
        }

        // 检查是否是简单复制
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
// 分析表达式是否为数组访问
// 增强：添加详细的 MEM_REF read/write 调试机制
// ============================================================================

ArrayDetectErrorCode analyzeArrayAccess (
  AD_FUNC_ARGS,
  tree expr,
  gimple* stmt,
  function* fn,
  AccessDirection direction,
  ArrayAccessCapture*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  AD_ASSERT_GCC_LOGIC (expr, "expr must not be NULL");
  AD_ASSERT_GCC_LOGIC (stmt, "stmt must not be NULL");

  ArrayAccessCapture* capture = NULL;
  tree base_pointer = NULL_TREE;
  tree offset_expr = NULL_TREE;
  ArrayAccessType access_type;

  // 检查 ARRAY_REF: ptr[i]
  if (TREE_CODE (expr) == ARRAY_REF) {
    base_pointer = TREE_OPERAND (expr, 0);
    offset_expr = TREE_OPERAND (expr, 1);
    access_type = ACCESS_ARRAY_REF;
  }
  // 检查 MEM_REF: *(ptr + offset) 或 *ptr
  else if (TREE_CODE (expr) == MEM_REF) {
    tree mem_base = TREE_OPERAND (expr, 0);
    tree mem_offset = TREE_OPERAND (expr, 1);

    // 检查偏移量是否非零（表示数组访问）
    if (mem_offset && TREE_CODE (mem_offset) == INTEGER_CST) {
      HOST_WIDE_INT offset_val = TREE_INT_CST_LOW (mem_offset);
      if (offset_val != 0) {
        base_pointer = mem_base;
        offset_expr = mem_offset;
        access_type = ACCESS_MEM_REF;
      }
    }

    // 检查基址是否是 POINTER_PLUS_EXPR 结果
    if (!base_pointer && mem_base && TREE_CODE (mem_base) == SSA_NAME) {
      gimple* def_stmt = SSA_NAME_DEF_STMT (mem_base);
      if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
        enum tree_code def_code = gimple_assign_rhs_code (def_stmt);
        if (def_code == POINTER_PLUS_EXPR) {
          base_pointer = gimple_assign_rhs1 (def_stmt);
          offset_expr = gimple_assign_rhs2 (def_stmt);
          access_type = ACCESS_MEM_REF;
        }
      }
    }
  }

  // 未匹配到数组访问模式，正常返回（*out_capture 保持 NULL）
  // 注：这不是错误，调用者通过检查 *out_capture 判断是否找到匹配
  if (!base_pointer) {
    AD_RETURNE (OK);
  }

  // 创建捕获结构
  capture = ggc_alloc<ArrayAccessCapture>();
  memset (capture, 0, sizeof (ArrayAccessCapture));

  capture->access_type = access_type;
  capture->direction = direction;
  capture->base_pointer = base_pointer;
  capture->offset_expr = offset_expr;
  capture->access_expr = expr;
  capture->stmt = stmt;
  capture->fn = fn;
  capture->bb = gimple_bb (stmt);
  capture->location = gimple_location (stmt);

  // 检查偏移量是否为常量
  if (offset_expr && TREE_CODE (offset_expr) == INTEGER_CST) {
    capture->is_constant_offset = true;
    capture->constant_offset = TREE_INT_CST_LOW (offset_expr);
  } else {
    capture->is_constant_offset = false;
    capture->constant_offset = 0;
  }

  // 获取元素类型
  if (TREE_TYPE (expr)) {
    capture->element_type = TREE_TYPE (expr);
  }

  // 获取基础指针类型
  if (base_pointer && TREE_TYPE (base_pointer)) {
    capture->base_type = TREE_TYPE (base_pointer);
  }

  // 追溯基础指针到字段
  tree containing_type = NULL_TREE;
  tree field_decl = NULL_TREE;
  AD_TRY (traceBasePointerToField (AD_ARGS, base_pointer, &containing_type, &field_decl));

  if (containing_type && field_decl) {
    capture->is_field_based = true;
    capture->containing_type = containing_type;
    capture->pointer_field_decl = field_decl;
  } else {
    capture->is_field_based = false;
  }

  AD_RETURNO (capture);
} AD_FUNCTION_END

// ============================================================================
// 收集函数内的所有数组访问
// ============================================================================

ArrayDetectErrorCode collectFunctionArrayAccesses (
  AD_FUNC_ARGS,
  function* fn,
  vec<ArrayAccessCapture*, va_gc>*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  AD_ASSERT_GCC_LOGIC (fn, "fn must not be NULL");
  AD_ASSERT_GCC_LOGIC (fn->cfg, "fn->cfg must not be NULL");

  vec<ArrayAccessCapture*, va_gc>* accesses = NULL;
  vec_alloc (accesses, 16);

  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    gimple_stmt_iterator gsi;
    for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
      gimple* stmt = gsi_stmt (gsi);

      if (gimple_code (stmt) != GIMPLE_ASSIGN) {
        continue;
      }

      tree lhs = gimple_assign_lhs (stmt);
      tree rhs = gimple_assign_rhs1 (stmt);

      // 检查左值（写入）
      if (lhs) {
        ArrayAccessCapture* capture = NULL;
        AD_TRY (analyzeArrayAccess (AD_ARGS, lhs, stmt, fn, ACCESS_WRITE, capture));
        if (capture) {
          vec_safe_push (accesses, capture);
        }
      }

      // 检查右值（读取）
      if (rhs) {
        ArrayAccessCapture* capture = NULL;
        AD_TRY (analyzeArrayAccess (AD_ARGS, rhs, stmt, fn, ACCESS_READ, capture));
        if (capture) {
          vec_safe_push (accesses, capture);
        }
      }

      // 检查右值第二个操作数（如果存在）
      if (gimple_assign_rhs2 (stmt)) {
        tree rhs2 = gimple_assign_rhs2 (stmt);
        ArrayAccessCapture* capture = NULL;
        AD_TRY (analyzeArrayAccess (AD_ARGS, rhs2, stmt, fn, ACCESS_READ, capture));
        if (capture) {
          vec_safe_push (accesses, capture);
        }
      }
    }
  }

  AD_RETURNO (accesses);
} AD_FUNCTION_END

// ============================================================================
// 获取或创建 TypeFieldArrayAccesses 条目
// ============================================================================

ArrayDetectErrorCode getOrCreateTypeFieldAccesses (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* map,
  tree type,
  tree field_decl,
  TypeFieldArrayAccesses*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  AD_ASSERT_GCC_LOGIC (map, "map must not be NULL");
  AD_ASSERT_GCC_LOGIC (type, "type must not be NULL");
  AD_ASSERT_GCC_LOGIC (field_decl, "field_decl must not be NULL");

  TypeFieldKey key = { TYPE_MAIN_VARIANT (type), field_decl };

  TypeFieldArrayAccesses** existing = map->get (key);
  if (existing && *existing) {
    AD_RETURNO (*existing);
  }

  // 创建新条目
  TypeFieldArrayAccesses* entry = ggc_alloc<TypeFieldArrayAccesses>();
  memset (entry, 0, sizeof (TypeFieldArrayAccesses));

  entry->type = TYPE_MAIN_VARIANT (type);
  entry->pointer_field_decl = field_decl;
  entry->type_name = safeGetTypeName (AD_ARGS, type);
  entry->field_name = safeGetFieldName (AD_ARGS, field_decl);
  entry->read_count = 0;
  entry->write_count = 0;

  vec_alloc (entry->accesses, 8);

  map->put (key, entry);
  AD_RETURNO (entry);
} AD_FUNCTION_END

// ============================================================================
// 收集所有函数的数组访问并按 (type, field) 聚合
// ============================================================================

ArrayDetectErrorCode collectAllArrayAccessesByTypeField (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  // 创建 hash_map
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* map =
    new hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>();

  unsigned int total_accesses = 0;
  unsigned int field_based_accesses = 0;

  // 遍历所有函数
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    // 防止访问已被内联释放的函数体
    if (node->inlined_to)
      continue;
    if (!node->has_gimple_body_p ())
      continue;

    function* fn = DECL_STRUCT_FUNCTION (node->decl);
    if (!fn || !fn->cfg) continue;

    push_cfun (fn);

    vec<ArrayAccessCapture*, va_gc>* fn_accesses = NULL;
    ArrayDetectErrorCode err = collectFunctionArrayAccesses (AD_ARGS, fn, fn_accesses);

    if (err == OK && fn_accesses) {
      for (unsigned int i = 0; i < fn_accesses->length (); i++) {
        ArrayAccessCapture* capture = (*fn_accesses)[i];
        if (!capture) continue;

        total_accesses++;

        if (capture->is_field_based) {
          field_based_accesses++;

          TypeFieldArrayAccesses* entry = NULL;
          AD_TRY (getOrCreateTypeFieldAccesses (AD_ARGS, map,
                    capture->containing_type, capture->pointer_field_decl, entry));

          if (entry) {
            vec_safe_push (entry->accesses, capture);
            if (capture->direction == ACCESS_READ) {
              entry->read_count++;
            } else {
              entry->write_count++;
            }
          }
        }
      }
    }

    pop_cfun ();
  }

  AD_DEBUG_PRINT ("arrayAccess: %u total, %u field-based", total_accesses, field_based_accesses);

  AD_RETURNO (map);
} AD_FUNCTION_END

// ============================================================================
// 调试输出：打印单个数组访问捕获
// ============================================================================

void printArrayAccessCapture (
  AD_FUNC_ARGS,
  FILE* out,
  ArrayAccessCapture* capture
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out || !capture) return;

  fprintf (out, "  [%s %s] at %s:%d\n",
           getAccessDirectionName (capture->direction),
           getAccessTypeName (capture->access_type),
           LOCATION_FILE (capture->location) ? LOCATION_FILE (capture->location) : "<unknown>",
           LOCATION_LINE (capture->location));

  if (capture->is_field_based) {
    fprintf (out, "    Base: %s::%s\n",
             safeGetTypeName (AD_ARGS, capture->containing_type),
             safeGetFieldName (AD_ARGS, capture->pointer_field_decl));
  }

  if (capture->is_constant_offset) {
    fprintf (out, "    Offset: %ld (constant)\n", (long)capture->constant_offset);
  } else {
    fprintf (out, "    Offset: <variable>\n");
  }
}

// ============================================================================
// 调试输出：打印所有数组访问
// ============================================================================

void printAllArrayAccesses (
  AD_FUNC_ARGS,
  FILE* out,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* map
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out) return;

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "                    ARRAY ACCESS COLLECTION RESULTS\n");
  fprintf (out, "================================================================================\n");

  if (!map) {
    fprintf (out, "No array accesses collected.\n");
    return;
  }

  typedef hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits> MapType;

  unsigned int total_fields = 0;
  unsigned int total_accesses = 0;

  for (MapType::iterator iter = map->begin (); iter != map->end (); ++iter) {
    TypeFieldArrayAccesses* entry = (*iter).second;
    if (!entry) continue;

    total_fields++;

    fprintf (out, "\n=== %s::%s ===\n", entry->type_name, entry->field_name);
    fprintf (out, "Total accesses: %u (read: %u, write: %u)\n\n",
             entry->accesses ? entry->accesses->length () : 0,
             entry->read_count, entry->write_count);

    if (entry->accesses) {
      total_accesses += entry->accesses->length ();
      for (unsigned int i = 0; i < entry->accesses->length (); i++) {
        printArrayAccessCapture (AD_ARGS, out, (*entry->accesses)[i]);
      }
    }
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "Summary:\n");
  fprintf (out, "  Total pointer fields with array accesses: %u\n", total_fields);
  fprintf (out, "  Total array accesses collected: %u\n", total_accesses);
  fprintf (out, "================================================================================\n");
  fprintf (out, "\n");
}

} // namespace array_detect_ns
