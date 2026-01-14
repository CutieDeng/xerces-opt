// ============================================================================
// ad-malloc-capacity 模块实现
// ============================================================================
// 分析单个字段写入中的 malloc size 参数，寻找关联整数字段
// ============================================================================

#include "malloc-capacity.hh"
#include "gcc-ext-util.hh"
#include "string-utils.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：判断类型是否为整数类型
// ============================================================================

static bool isIntegerType (tree type) {
  if (!type) return false;
  tree main_type = TYPE_MAIN_VARIANT (type);
  return INTEGRAL_TYPE_P (main_type);
}

static bool isIntegralArg (tree arg) {
  if (!arg) return false;
  tree arg_type = TREE_TYPE (arg);
  if (!arg_type) return false;
  return INTEGRAL_TYPE_P (TYPE_MAIN_VARIANT (arg_type));
}

// ============================================================================
// 辅助函数：检查表达式是否引用了指定字段
// ============================================================================

static bool exprReferencesField (
  AD_FUNC_ARGS,
  tree expr,
  tree field_decl,
  int depth = 0
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || !field_decl) {
    return false;
  }

  // 防止无限递归
  if (depth > 20) {
    return false;
  }

  // 直接检查 COMPONENT_REF
  if (TREE_CODE (expr) == COMPONENT_REF) {
    tree accessed_field = TREE_OPERAND (expr, 1);
    if (accessed_field == field_decl) {
      AD_DEBUG_PRINT ("[exprReferencesField] FOUND: COMPONENT_REF directly references field (depth=%d)", depth);
      return true;
    }
    tree base = TREE_OPERAND (expr, 0);
    if (exprReferencesField (AD_ARGS, base, field_decl, depth + 1)) {
      return true;
    }
  }

  // 检查 MEM_REF
  if (TREE_CODE (expr) == MEM_REF) {
    tree base = TREE_OPERAND (expr, 0);
    if (exprReferencesField (AD_ARGS, base, field_decl, depth + 1)) {
      return true;
    }
  }

  // 检查 SSA_NAME（追溯定义）
  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def_stmt = SSA_NAME_DEF_STMT (expr);
    if (depth == 0) {
      AD_DEBUG_PRINT ("[exprReferencesField] SSA_NAME: def_stmt=%p, is_assign=%d",
                      (void*)def_stmt, def_stmt ? is_gimple_assign (def_stmt) : 0);
    }
    if (def_stmt && is_gimple_assign (def_stmt)) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (depth == 0) {
        AD_DEBUG_PRINT ("[exprReferencesField] SSA_NAME: rhs code=%d", rhs ? TREE_CODE (rhs) : -1);
      }
      if (exprReferencesField (AD_ARGS, rhs, field_decl, depth + 1)) {
        return true;
      }
      tree rhs2 = gimple_assign_rhs2 (def_stmt);
      if (rhs2) {
        if (depth == 0) {
          AD_DEBUG_PRINT ("[exprReferencesField] SSA_NAME: rhs2 code=%d", TREE_CODE (rhs2));
        }
        if (exprReferencesField (AD_ARGS, rhs2, field_decl, depth + 1)) {
          return true;
        }
      }
    } else if (def_stmt && is_gimple_call (def_stmt)) {
      // SSA_NAME 来自函数调用，检查调用参数
      if (depth == 0) {
        AD_DEBUG_PRINT ("[exprReferencesField] SSA_NAME: from GIMPLE_CALL");
      }
      unsigned nargs = gimple_call_num_args (def_stmt);
      for (unsigned i = 0; i < nargs; i++) {
        tree arg = gimple_call_arg (def_stmt, i);
        if (exprReferencesField (AD_ARGS, arg, field_decl, depth + 1)) {
          return true;
        }
      }
    } else if (def_stmt && gimple_code (def_stmt) == GIMPLE_PHI) {
      // PHI 节点：跳过，不追踪
      // PHI 节点表示控制流合并点，其中的字段引用是间接的
      // 例如 newMax = fCurCount + extraNeeded 被合并后用于 malloc
      // 这不应被视为 malloc-size 的直接关联
      if (depth == 0) {
        AD_DEBUG_PRINT ("[exprReferencesField] SSA_NAME: from GIMPLE_PHI, skipping");
      }
      // 不再追踪 PHI 节点的输入
    }
  }

  // 检查二元操作
  if (BINARY_CLASS_P (expr)) {
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl, depth + 1)) {
      return true;
    }
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 1), field_decl, depth + 1)) {
      return true;
    }
  }

  // 检查一元操作
  if (UNARY_CLASS_P (expr)) {
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl, depth + 1)) {
      return true;
    }
  }

  // 检查类型转换
  if (CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    if (exprReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl, depth + 1)) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// 检查 gimple 语句是否为 malloc/calloc/realloc 或类似分配调用
// ============================================================================

bool isMallocLikeCall (gimple* stmt, char const** out_func_name) {
  if (!stmt || !is_gimple_call (stmt)) {
    return false;
  }

  char const* name = nullptr;

  // 尝试获取直接调用的函数声明
  tree fndecl = gimple_call_fndecl (stmt);
  if (fndecl) {
    tree id = DECL_NAME (fndecl);
    if (id) {
      name = IDENTIFIER_POINTER (id);
    }
  }

  // 如果直接调用没有函数名，尝试从内部函数名获取
  if (!name) {
    tree fn = gimple_call_fn (stmt);
    if (fn) {
      // 虚函数调用 - 尝试从类型系统获取函数名
      tree fn_type = TREE_TYPE (fn);
      if (fn_type && TREE_CODE (fn_type) == POINTER_TYPE) {
        tree pointed = TREE_TYPE (fn_type);
        if (pointed && TREE_CODE (pointed) == FUNCTION_TYPE) {
          // 这是一个函数指针调用，无法静态确定函数名
          // 但可以尝试从调试信息获取
        }
      }
    }
  }

  if (!name) {
    return false;
  }

  // 检查是否为常见分配函数（精确匹配）
  if (strcmp (name, "malloc") == 0 ||
      strcmp (name, "calloc") == 0 ||
      strcmp (name, "realloc") == 0 ||
      strcmp (name, "xmalloc") == 0 ||
      strcmp (name, "xcalloc") == 0 ||
      strcmp (name, "xrealloc") == 0 ||
      strcmp (name, "g_malloc") == 0 ||
      strcmp (name, "g_malloc0") == 0 ||
      strcmp (name, "g_realloc") == 0 ||
      strcmp (name, "operator new") == 0 ||
      strcmp (name, "operator new[]") == 0) {
    if (out_func_name) {
      *out_func_name = name;
    }
    return true;
  }

  // 检查是否包含 "alloc" 子串（用于自定义分配器如 allocate, Allocate, etc.）
  if (strstr (name, "alloc") != nullptr ||
      strstr (name, "Alloc") != nullptr ||
      strstr (name, "ALLOC") != nullptr) {
    if (out_func_name) {
      *out_func_name = name;
    }
    return true;
  }

  return false;
}

// ============================================================================
// 检查表达式是否引用了指定类型的某个整数字段（收集所有匹配）
// ============================================================================

static ArrayDetectErrorCode collectDirectFieldReferences (
  AD_FUNC_ARGS,
  tree expr,
  tree containing_type,
  vec<tree, va_gc>** out_fields
) AD_FUNCTION_BEGIN {
  if (!expr || !containing_type) {
    AD_RETURNE (OK);
  }

  if (TREE_CODE (containing_type) != RECORD_TYPE) {
    AD_DEBUG_PRINT ("[collectDirectFieldReferences] containing_type is not RECORD_TYPE (code=%d)",
                    TREE_CODE (containing_type));
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[collectDirectFieldReferences] scanning integer fields for references in expr (code=%d)",
                  TREE_CODE (expr));

  // 遍历类型的所有整数字段，检查表达式是否引用
  unsigned checked = 0;
  for (tree field = TYPE_FIELDS (containing_type); field; field = DECL_CHAIN (field)) {
    if (TREE_CODE (field) != FIELD_DECL) continue;

    tree field_type = TREE_TYPE (field);
    if (!isIntegerType (field_type)) continue;

    checked++;
    char const* fname = safeGetFieldName (AD_ARGS, field);
    bool found = exprReferencesField (AD_ARGS, expr, field, 0);
    AD_DEBUG_PRINT ("[collectDirectFieldReferences]   checking integer field '%s': %s",
                    fname, found ? "FOUND" : "not found");
    if (found) {
      vec_safe_push (*out_fields, field);
    }
  }

  AD_DEBUG_PRINT ("[collectDirectFieldReferences] checked %u integer fields", checked);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 检查两个 SSA_NAME 是否来自同一个定义链
// ============================================================================

static bool ssaNamesRelated (tree ssa1, tree ssa2, int depth = 0) {
  if (depth > 10) return false;
  if (!ssa1 || !ssa2) return false;
  if (ssa1 == ssa2) return true;

  // 如果两个都是 SSA_NAME，比较它们的定义
  if (TREE_CODE (ssa1) == SSA_NAME && TREE_CODE (ssa2) == SSA_NAME) {
    gimple* def1 = SSA_NAME_DEF_STMT (ssa1);
    gimple* def2 = SSA_NAME_DEF_STMT (ssa2);

    // 如果是同一个定义语句
    if (def1 && def1 == def2) return true;

    // 追溯 ssa1 的定义
    if (def1 && is_gimple_assign (def1)) {
      tree rhs1 = gimple_assign_rhs1 (def1);
      if (ssaNamesRelated (rhs1, ssa2, depth + 1)) return true;
    }

    // 追溯 ssa2 的定义
    if (def2 && is_gimple_assign (def2)) {
      tree rhs2 = gimple_assign_rhs1 (def2);
      if (ssaNamesRelated (ssa1, rhs2, depth + 1)) return true;
    }
  }

  return false;
}

// ============================================================================
// 查找 size 值被写入的所有整数字段
// ============================================================================

static ArrayDetectErrorCode findFieldsWrittenWithValue (
  AD_FUNC_ARGS,
  tree size_expr,
  tree containing_type,
  function* search_fn,
  vec<tree, va_gc>** out_fields
) AD_FUNCTION_BEGIN {
  if (!size_expr || !containing_type || !search_fn) {
    AD_RETURNE (OK);
  }

  // 遍历函数中的所有基本块和语句
  basic_block bb;
  FOR_EACH_BB_FN (bb, search_fn) {
    for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
         !gsi_end_p (gsi);
         gsi_next (&gsi)) {
      gimple* stmt = gsi_stmt (gsi);

      if (!is_gimple_assign (stmt)) continue;

      tree lhs = gimple_assign_lhs (stmt);
      tree rhs = gimple_assign_rhs1 (stmt);

      if (!lhs || !rhs) continue;

      // 检查 LHS 是否为目标类型的整数字段写入
      if (TREE_CODE (lhs) != COMPONENT_REF) continue;

      tree field = TREE_OPERAND (lhs, 1);
      tree base_obj = TREE_OPERAND (lhs, 0);

      if (!field || TREE_CODE (field) != FIELD_DECL) continue;

      tree base_type = TREE_TYPE (base_obj);
      if (!base_type || TREE_CODE (base_type) != RECORD_TYPE) continue;

      // 类型匹配检查
      if (TYPE_MAIN_VARIANT (base_type) != TYPE_MAIN_VARIANT (containing_type)) continue;

      // 检查是否为整数字段
      if (!isIntegerType (TREE_TYPE (field))) continue;

      // 检查 RHS 是否与 size_expr 相关
      bool related = false;

      // 直接相等
      if (rhs == size_expr) {
        related = true;
      }
      // SSA_NAME 关联检查
      else if (TREE_CODE (rhs) == SSA_NAME && TREE_CODE (size_expr) == SSA_NAME) {
        related = ssaNamesRelated (rhs, size_expr, 0);
      }
      // RHS 是 size_expr 的定义
      else if (TREE_CODE (size_expr) == SSA_NAME) {
        gimple* def = SSA_NAME_DEF_STMT (size_expr);
        if (def && is_gimple_assign (def)) {
          tree def_rhs = gimple_assign_rhs1 (def);
          if (def_rhs == rhs) related = true;
        }
      }

      if (related) {
        // 检查是否已添加
        bool already_added = false;
        if (*out_fields) {
          for (unsigned i = 0; i < (*out_fields)->length (); i++) {
            if ((**out_fields)[i] == field) {
              already_added = true;
              break;
            }
          }
        }
        if (!already_added) {
          vec_safe_push (*out_fields, field);
        }
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 查找 size 表达式关联的所有整数字段
// ============================================================================

ArrayDetectErrorCode traceSizeToIntegerFields (
  AD_FUNC_ARGS,
  tree size_expr,
  tree containing_type,
  tree function_decl,
  vec<tree, va_gc>** out_integer_fields
) AD_FUNCTION_BEGIN {
  if (!out_integer_fields) {
    AD_RETURNE (OK);
  }

  if (!*out_integer_fields) {
    vec_alloc (*out_integer_fields, 4);
  }

  if (!size_expr || !containing_type) {
    AD_DEBUG_PRINT ("[traceSizeToIntegerFields] SKIP: size_expr=%p, containing_type=%p",
                    (void*)size_expr, (void*)containing_type);
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[traceSizeToIntegerFields] size_expr tree_code=%d", TREE_CODE (size_expr));

  unsigned before_direct = *out_integer_fields ? (*out_integer_fields)->length () : 0;

  // 方式1：表达式直接引用整数字段
  AD_TRY (collectDirectFieldReferences (AD_ARGS, size_expr, containing_type, out_integer_fields));

  unsigned after_direct = *out_integer_fields ? (*out_integer_fields)->length () : 0;
  AD_DEBUG_PRINT ("[traceSizeToIntegerFields] collectDirectFieldReferences found %u field(s)",
                  after_direct - before_direct);

  // 方式2：查找 size 值被写入的整数字段
  // 从 function_decl 获取函数体进行搜索
  function* search_fn = function_decl ? DECL_STRUCT_FUNCTION (function_decl) : nullptr;
  if (search_fn) {
    AD_TRY (findFieldsWrittenWithValue (AD_ARGS, size_expr, containing_type, search_fn, out_integer_fields));
    unsigned after_written = *out_integer_fields ? (*out_integer_fields)->length () : 0;
    AD_DEBUG_PRINT ("[traceSizeToIntegerFields] findFieldsWrittenWithValue found %u additional field(s)",
                    after_written - after_direct);
  } else {
    AD_DEBUG_PRINT ("[traceSizeToIntegerFields] function_decl is NULL or has no body, skipping findFieldsWrittenWithValue");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析单个字段写入的 malloc 容量关联
// ============================================================================

ArrayDetectErrorCode analyzeMallocCapacity (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  WriteOriginalSource* write_source,
  tree containing_type,
  vec<MallocCapacityEvidence*, va_gc>** results
) AD_FUNCTION_BEGIN {
  if (!results || !write_info || !write_source) {
    AD_RETURNE (OK);
  }

  // 获取类型名用于调试
  char const* type_name_dbg = NULL;
  gcc_ext_util::formatTypeNameWithTemplateArgs (AD_ARGS, containing_type, type_name_dbg);
  char const* field_name_dbg = safeGetFieldName (AD_ARGS, write_info->field_decl);

  // 获取源码位置用于调试
  char loc_buf[256];
  gcc_ext_util::get_source_location_string (AD_ARGS, write_info->location, loc_buf, sizeof(loc_buf));

  AD_DEBUG_PRINT ("[malloc-capacity] === Analyzing write to %s.%s ===",
                  type_name_dbg ? type_name_dbg : "?", field_name_dbg);
  AD_DEBUG_PRINT ("[malloc-capacity]   location: %s", loc_buf);
  AD_DEBUG_PRINT ("[malloc-capacity]   source_type=%d (FUNCTION_CALL=%d)",
                  (int)write_source->source_type, (int)SOURCE_FUNCTION_CALL);

  // 只处理函数调用来源
  if (write_source->source_type != SOURCE_FUNCTION_CALL) {
    AD_DEBUG_PRINT ("[malloc-capacity]   SKIP: not a function call source");
    AD_RETURNE (OK);
  }

  gimple* call_stmt = write_source->data.function_call.call_stmt;
  if (!call_stmt) {
    AD_DEBUG_PRINT ("[malloc-capacity]   SKIP: call_stmt is NULL");
    AD_RETURNE (OK);
  }

  // 获取函数调用的源码位置
  char call_loc_buf[256];
  gcc_ext_util::get_source_location_string (AD_ARGS, gimple_location(call_stmt), call_loc_buf, sizeof(call_loc_buf));
  AD_DEBUG_PRINT ("[malloc-capacity]   call_stmt at: %s", call_loc_buf);

  // 不做函数名判定，仅记录已有函数名用于调试
  char const* pre_extracted_name = write_source->data.function_call.function_name;
  AD_DEBUG_PRINT ("[malloc-capacity]   func_name(observed)='%s'",
                  pre_extracted_name ? pre_extracted_name : "(null)");
  char const* func_name = pre_extracted_name ? pre_extracted_name : "<unknown>";

  vec<tree, va_gc>* integer_fields = NULL;
  tree size_expr = NULL_TREE;

  unsigned nargs = gimple_call_num_args (call_stmt);
  if (nargs > 0) {
    tree arg0 = gimple_call_arg (call_stmt, 0);
    if (isIntegralArg (arg0)) {
      vec<tree, va_gc>* candidate_fields = NULL;
      AD_TRY (traceSizeToIntegerFields (AD_ARGS, arg0, containing_type, write_info->function_decl, &candidate_fields));
      if (candidate_fields && candidate_fields->length () > 0) {
        size_expr = arg0;
        integer_fields = candidate_fields;
      }
    }
  }

  if (!size_expr) {
    for (unsigned i = 0; i < nargs; i++) {
      tree arg = gimple_call_arg (call_stmt, i);
      if (!isIntegralArg (arg)) continue;

      vec<tree, va_gc>* candidate_fields = NULL;
      AD_TRY (traceSizeToIntegerFields (AD_ARGS, arg, containing_type, write_info->function_decl, &candidate_fields));
      if (candidate_fields && candidate_fields->length () > 0) {
        size_expr = arg;
        integer_fields = candidate_fields;
        AD_DEBUG_PRINT ("[malloc-capacity]   fallback: arg[%u] treated as size expr", i);
        break;
      }
    }
  }

  if (!size_expr) {
    AD_DEBUG_PRINT ("[malloc-capacity]   SKIP: no size expression for allocation");
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[malloc-capacity]   size_expr found, tree_code=%d", TREE_CODE (size_expr));

  if (!integer_fields) {
    AD_TRY (traceSizeToIntegerFields (AD_ARGS, size_expr, containing_type, write_info->function_decl, &integer_fields));
  }

  if (!integer_fields || integer_fields->length () == 0) {
    AD_DEBUG_PRINT ("[malloc-capacity]   SKIP: no integer fields found in size expression");
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[malloc-capacity]   FOUND %u integer field(s) in size expression",
                  integer_fields->length ());

  // 确认有证据需要添加，此时才初始化 results
  if (!*results) {
    vec_alloc (*results, integer_fields->length ());
  }

  // 为每个整数字段创建证据
  for (unsigned i = 0; i < integer_fields->length (); i++) {
    tree integer_field = (*integer_fields)[i];

    MallocCapacityEvidence* evidence = ggc_alloc<MallocCapacityEvidence>();
    memset (evidence, 0, sizeof (MallocCapacityEvidence));

    evidence->pointer_field = write_info->field_decl;
    evidence->integer_field = integer_field;
    evidence->containing_type = containing_type;
    evidence->write_info = write_info;
    evidence->write_source = write_source;
    evidence->malloc_call = call_stmt;
    evidence->alloc_func_name = ggc_strdup (func_name);
    evidence->size_expr = size_expr;
    evidence->confidence = MALLOC_CONF_CERTAIN;
    evidence->location = gimple_location (call_stmt);
    evidence->description = "Allocation size references integer field";

    // 增强调试信息：包含类型模板
    char const* type_name = NULL;
    gcc_ext_util::formatTypeNameWithTemplateArgs (AD_ARGS, containing_type, type_name);
    AD_DEBUG_PRINT ("[malloc-capacity] %s.%s: %s() -> field '%s'",
                    type_name ? type_name : "?",
                    safeGetFieldName (AD_ARGS, write_info->field_decl),
                    func_name,
                    safeGetFieldName (AD_ARGS, integer_field));

    vec_safe_push (*results, evidence);
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 获取置信度名称
// ============================================================================

char const* mallocConfidenceToString (MallocEvidenceConfidence conf) {
  switch (conf) {
    case MALLOC_CONF_CERTAIN:  return "certain";
    case MALLOC_CONF_PROBABLE: return "probable";
    case MALLOC_CONF_WEAK:     return "weak";
    default:                   return "unknown";
  }
}

// ============================================================================
// 打印单个 malloc 容量证据
// ============================================================================

void printMallocCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  MallocCapacityEvidence* evidence
) {
  if (!out || !evidence) return;

  char const* ptr_name = safeGetFieldName (AD_ARGS, evidence->pointer_field);
  char const* int_name = safeGetFieldName (AD_ARGS, evidence->integer_field);

  fprintf (out, "  [malloc-evidence] %s -> %s (%s) confidence=%s\n",
           ptr_name,
           int_name,
           evidence->alloc_func_name ? evidence->alloc_func_name : "?",
           mallocConfidenceToString (evidence->confidence));
}

} // namespace array_detect_ns
