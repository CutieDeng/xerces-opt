// ============================================================================
// ad-write-source 模块实现
// ============================================================================
// 追踪字段写入的来源
// 数据流：FieldWriteInfo -> WriteOriginalSource
// ============================================================================

#include "write-source.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-analysis.hh"
#include "info-print.hh"

namespace array_detector {

using namespace ::array_detect_ns;
using namespace ::field_analysis;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// traceWriteSource_reduceTrivialMoves
// ----------------------------------------------------------------------------
// 追踪 SSA 使用-定义链，跳过简单的赋值

ArrayDetectErrorCode traceWriteSource_reduceTrivialMoves (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree value,
  tree function,
  basic_block bb,
  tree& result_final_value,
  gimple*& result_final_stmt,
  bool& result_is_phi
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;

  result_is_phi = false;

  // 非 SSA_NAME 直接返回
  if (TREE_CODE (value) != SSA_NAME) {
    result_final_value = value;
    result_final_stmt = NULL;
    AD_RETURNE (OK);
  }

  gimple * def_stmt = SSA_NAME_DEF_STMT (value);
  if (!def_stmt) {
    result_final_value = value;
    result_final_stmt = NULL;
    AD_RETURNE (OK);
  }

  enum gimple_code code = gimple_code (def_stmt);

  // PHI 节点
  if (code == GIMPLE_PHI) {
    result_final_value = value;
    result_final_stmt = def_stmt;
    result_is_phi = true;
    AD_RETURNE (OK);
  }

  // 函数调用
  if (code == GIMPLE_CALL) {
    result_final_value = value;
    result_final_stmt = def_stmt;
    AD_RETURNE (OK);
  }

  // 赋值语句
  if (code == GIMPLE_ASSIGN) {
    tree lhs = gimple_assign_lhs (def_stmt);
    tree rhs = gimple_assign_rhs1 (def_stmt);
    enum tree_code rhs_code = gimple_assign_rhs_code (def_stmt);

    bool is_trivial = false;

    if (TREE_CODE (rhs) == SSA_NAME) {
      if (rhs_code == NOP_EXPR) {
        is_trivial = true;
      } else if (rhs_code == CONVERT_EXPR) {
        if (useless_type_conversion_p (TREE_TYPE (lhs), rhs)) {
          is_trivial = true;
        }
      } else if (rhs_code == VIEW_CONVERT_EXPR) {
        is_trivial = true;
      } else if (TREE_TYPE (lhs) == TREE_TYPE (rhs)) {
        if (gimple_assign_copy_p (def_stmt)) {
          is_trivial = true;
        }
      }
    }

    if (is_trivial) {
      basic_block def_bb = gimple_bb (def_stmt);
      AD_ASSERT_GCC_LOGIC (def_bb, "gimple_bb returned NULL");
      AD_TRY (traceWriteSource_reduceTrivialMoves (
        AD_ARGS, detector, rhs, function, def_bb,
        result_final_value, result_final_stmt, result_is_phi));
      AD_RETURNE (OK);
    }

    result_final_value = rhs;
    result_final_stmt = def_stmt;
    AD_RETURNE (OK);
  }

  result_final_value = value;
  result_final_stmt = def_stmt;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromCall
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromCall (
  AD_FUNC_ARGS,
  gimple* call_stmt,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_FUNCTION_CALL;
  out_data->function_call.stmt = call_stmt;
  out_data->function_call.location = gimple_location (call_stmt);

  tree fn = gimple_call_fn (call_stmt);
  if (fn && TREE_CODE (fn) == OBJ_TYPE_REF) {
    out_data->function_call.call_kind = FIELD_CALL_VIRTUAL;
    out_data->function_call.name = ggc_strdup ("<virtual>");
  } else if (fn && TREE_CODE (fn) == ADDR_EXPR) {
    tree fn_decl = TREE_OPERAND (fn, 0);
    if (fn_decl && DECL_NAME (fn_decl)) {
      out_data->function_call.name =
        ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fn_decl)));
      out_data->function_call.call_kind = FIELD_CALL_DIRECT;
    } else {
      out_data->function_call.name = ggc_strdup ("<unknown>");
      out_data->function_call.call_kind = FIELD_CALL_UNKNOWN;
    }
  } else {
    out_data->function_call.call_kind = FIELD_CALL_INDIRECT;
    out_data->function_call.name = ggc_strdup ("<indirect>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromConstant
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromConstant (
  AD_FUNC_ARGS,
  tree constant_value,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_CONSTANT;
  out_data->constant.value = constant_value;

  if (integer_zerop (constant_value)) {
    out_data->constant.str = ggc_strdup ("0");
  } else if (TREE_CODE (constant_value) == INTEGER_CST) {
    char buf[64];
    snprintf (buf, sizeof (buf), "%ld", (long)TREE_INT_CST_LOW (constant_value));
    out_data->constant.str = ggc_strdup (buf);
  } else {
    out_data->constant.str = ggc_strdup ("<constant>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromFieldAccess
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromFieldAccess (
  AD_FUNC_ARGS,
  tree field_ref,
  gimple* final_stmt,
  location_t location,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_FIELD_ACCESS;
  out_data->field_access.stmt = final_stmt;
  out_data->field_access.location = location;

  if (TREE_CODE (field_ref) == COMPONENT_REF) {
    tree field_decl = TREE_OPERAND (field_ref, 1);
    tree base = TREE_OPERAND (field_ref, 0);

    out_data->field_access.field = field_decl;
    out_data->field_access.object = base;

    if (field_decl && DECL_NAME (field_decl)) {
      out_data->field_access.field_name =
        ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (field_decl)));
    } else {
      out_data->field_access.field_name = ggc_strdup ("<anonymous>");
    }

    tree object_type = TREE_TYPE (base);
    if (object_type) {
      if (TREE_CODE (object_type) == POINTER_TYPE ||
          TREE_CODE (object_type) == REFERENCE_TYPE) {
        object_type = TREE_TYPE (object_type);
      }
      out_data->field_access.object_type = TYPE_MAIN_VARIANT (object_type);
      if (TYPE_NAME (object_type)) {
        tree type_name = TYPE_NAME (object_type);
        if (TREE_CODE (type_name) == IDENTIFIER_NODE) {
          out_data->field_access.type_name =
            ggc_strdup (IDENTIFIER_POINTER (type_name));
        } else if (TREE_CODE (type_name) == TYPE_DECL && DECL_NAME (type_name)) {
          out_data->field_access.type_name =
            ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (type_name)));
        }
      }
    }
  } else if (TREE_CODE (field_ref) == MEM_REF) {
    tree base = TREE_OPERAND (field_ref, 0);
    out_data->field_access.object = base;
    out_data->field_access.field_name = ggc_strdup ("<mem_ref>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromComputation
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromComputation (
  AD_FUNC_ARGS,
  tree expr,
  gimple* final_stmt,
  location_t location,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_COMPUTATION;
  out_data->computation.stmt = final_stmt;
  out_data->computation.expr = expr;
  out_data->computation.location = location;

  enum tree_code code = TREE_CODE (expr);
  char const * desc = get_tree_code_name (code);
  out_data->computation.desc = ggc_strdup (desc ? desc : "<computation>");

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromPhi
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromPhi (
  AD_FUNC_ARGS,
  gimple* phi_stmt,
  tree ssa_name,
  location_t location,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_PHI;
  out_data->phi.stmt = phi_stmt;
  out_data->phi.ssa_name = ssa_name;
  out_data->phi.location = location;

  tree var = SSA_NAME_VAR (ssa_name);
  if (var) {
    out_data->phi.var = var;
    if (DECL_NAME (var)) {
      out_data->phi.var_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (var)));
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromUnknown
// ----------------------------------------------------------------------------
// 输出：直接写入 out_kind

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromUnknown (
  AD_FUNC_ARGS,
  FieldSourceKind* out_kind
) AD_FUNCTION_BEGIN {
  if (!out_kind) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_kind = FIELD_SRC_UNKNOWN;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource
// ----------------------------------------------------------------------------
// 从最终值提取来源，写入 out_kind 和 out_data

ArrayDetectErrorCode traceWriteSource_extractSource (
  AD_FUNC_ARGS,
  tree final_value,
  gimple* final_stmt,
  location_t location,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  // SSA_NAME 且有函数调用定义
  if (TREE_CODE (final_value) == SSA_NAME) {
    if (final_stmt && gimple_code (final_stmt) == GIMPLE_CALL) {
      AD_TRY (traceWriteSource_extractSource_extractFromCall (
        AD_ARGS, final_stmt, out_kind, out_data));
      AD_RETURNE (OK);
    }
    // 无法追踪到明确来源
    AD_TRY (traceWriteSource_extractSource_extractFromUnknown (AD_ARGS, out_kind));
    AD_RETURNE (OK);
  }

  // 常量
  if (CONSTANT_CLASS_P (final_value)) {
    AD_TRY (traceWriteSource_extractSource_extractFromConstant (
      AD_ARGS, final_value, out_kind, out_data));
    AD_RETURNE (OK);
  }

  // 字段访问
  enum tree_code final_code = TREE_CODE (final_value);
  if (final_code == COMPONENT_REF || final_code == MEM_REF) {
    AD_TRY (traceWriteSource_extractSource_extractFromFieldAccess (
      AD_ARGS, final_value, final_stmt, location, out_kind, out_data));
    AD_RETURNE (OK);
  }

  // 其他计算表达式
  AD_TRY (traceWriteSource_extractSource_extractFromComputation (
    AD_ARGS, final_value, final_stmt, location, out_kind, out_data));
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// traceWriteSource
// ----------------------------------------------------------------------------
// 主入口：追踪写入来源
// 输出：直接写入 out_kind 和 out_data 指向的地址

ArrayDetectErrorCode traceWriteSource (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  FieldWriteInfo* write_info,
  FieldSourceKind* out_kind,
  FieldSourceDataUnion* out_data
) AD_FUNCTION_BEGIN {
  if (!write_info || !out_kind || !out_data) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 清零输出
  *out_kind = FIELD_SRC_UNKNOWN;
  memset (out_data, 0, sizeof (FieldSourceDataUnion));

  tree rhs = write_info->rhs;
  gimple* stmt = write_info->stmt;
  location_t location = write_info->location;
  tree function = write_info->function_decl;
  basic_block bb = write_info->bb;

  // 约减平凡赋值
  tree final_value;
  gimple* final_stmt;
  bool is_phi = false;

  AD_TRY (traceWriteSource_reduceTrivialMoves (
    AD_ARGS, detector, rhs, function, bb,
    final_value, final_stmt, is_phi));

  // PHI 节点特殊处理
  if (is_phi) {
    AD_TRY (traceWriteSource_extractSource_extractFromPhi (
      AD_ARGS, final_stmt, final_value, location, out_kind, out_data));
    AD_RETURNE (OK);
  }

  // 从最终值提取来源
  AD_TRY (traceWriteSource_extractSource (
    AD_ARGS, final_value, final_stmt, location, out_kind, out_data));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceFieldAssignments
// ----------------------------------------------------------------------------
// Pipeline 接口：追踪所有字段赋值

ArrayDetectErrorCode traceFieldAssignments (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Tracing field assignments (new module)");

  typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;
  unsigned int processed_count = 0;
  unsigned int source_extracted_count = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldAnalysisData *tfwo = (*iter).second;
    if (!tfwo || !tfwo->writes) {
      continue;
    }

    for (unsigned int j = 0; j < tfwo->writes->length (); ++j) {
      Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude *wrapper = (*tfwo->writes)[j];
      if (!wrapper) {
        continue;
      }

      processed_count++;

      // 从 wrapper 创建临时 FieldWriteInfo
      FieldWriteInfo temp_write_info;
      memset (&temp_write_info, 0, sizeof (FieldWriteInfo));
      temp_write_info.type = wrapper->type;
      temp_write_info.field_decl = wrapper->field;
      temp_write_info.function_decl = wrapper->func;
      temp_write_info.bb = wrapper->bb;
      temp_write_info.stmt = wrapper->stmt;
      temp_write_info.lhs = wrapper->lhs;
      temp_write_info.rhs = wrapper->rhs;
      temp_write_info.location = wrapper->write_location;

      // 直接写入 wrapper 成员地址
      AD_TRY (traceWriteSource (AD_ARGS, detector, &temp_write_info,
        &wrapper->source_kind, &wrapper->source_data));
      source_extracted_count++;

      // 调试输出（传 NULL 跳过，因为 WriteOriginalSource 已重构）
      TypeFieldKey key = (*iter).first;
      AD_TRY (printFieldWriteSourceInfoFromWrapper (AD_ARGS, key.type, key.field_decl, wrapper, NULL));
    }
  }

  AD_DEBUG_PRINT ("Tracing complete: %u write operations processed, %u sources extracted",
                  processed_count, source_extracted_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
