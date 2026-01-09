// ============================================================================
// ad-write-source 模块实现
// ============================================================================
// 追踪字段写入的来源
// 数据流：FieldWriteInfo -> WriteOriginalSource
// ============================================================================

#include "../../include/ad-write-source/write-source.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-analysis.hh"
#include "info-print.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// traceWriteSource_reduceTrivialMoves
// ============================================================================
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

// ============================================================================
// traceWriteSource_extractFromCall
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromCall (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  gimple* call_stmt,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_FUNCTION_CALL;
  source->data.function_call.call_stmt = call_stmt;
  source->data.function_call.location = gimple_location (call_stmt);

  tree fn = gimple_call_fn (call_stmt);
  if (fn && TREE_CODE (fn) == OBJ_TYPE_REF) {
    source->data.function_call.call_type = CALL_VIRTUAL;
    source->data.function_call.function_name = ggc_strdup ("<virtual>");
  } else if (fn && TREE_CODE (fn) == ADDR_EXPR) {
    tree fn_decl = TREE_OPERAND (fn, 0);
    if (fn_decl && DECL_NAME (fn_decl)) {
      source->data.function_call.function_name =
        ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fn_decl)));
      source->data.function_call.call_type = CALL_DIRECT;
    } else {
      source->data.function_call.function_name = ggc_strdup ("<unknown>");
      source->data.function_call.call_type = CALL_UNKNOWN;
    }
  } else {
    source->data.function_call.call_type = CALL_INDIRECT;
    source->data.function_call.function_name = ggc_strdup ("<indirect>");
  }

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractFromConstant
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromConstant (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree constant_value,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_CONSTANT;
  source->data.constant.constant_value = constant_value;

  if (integer_zerop (constant_value)) {
    source->data.constant.constant_str = ggc_strdup ("0");
  } else if (TREE_CODE (constant_value) == INTEGER_CST) {
    char buf[64];
    snprintf (buf, sizeof (buf), "%ld", (long)TREE_INT_CST_LOW (constant_value));
    source->data.constant.constant_str = ggc_strdup (buf);
  } else {
    source->data.constant.constant_str = ggc_strdup ("<constant>");
  }

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractFromFieldAccess
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromFieldAccess (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree field_ref,
  gimple* final_stmt,
  gimple* original_stmt,
  location_t location,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)original_stmt;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_FIELD_ACCESS;
  source->data.field_access.access_stmt = final_stmt;
  source->data.field_access.access_expr = field_ref;
  source->data.field_access.location = location;

  if (TREE_CODE (field_ref) == COMPONENT_REF) {
    tree field_decl = TREE_OPERAND (field_ref, 1);
    tree base = TREE_OPERAND (field_ref, 0);

    source->data.field_access.field_decl = field_decl;
    source->data.field_access.base_object = base;

    if (field_decl && DECL_NAME (field_decl)) {
      source->data.field_access.field_name =
        ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (field_decl)));
    } else {
      source->data.field_access.field_name = ggc_strdup ("<anonymous>");
    }

    tree object_type = TREE_TYPE (base);
    if (object_type) {
      if (TREE_CODE (object_type) == POINTER_TYPE ||
          TREE_CODE (object_type) == REFERENCE_TYPE) {
        object_type = TREE_TYPE (object_type);
      }
      source->data.field_access.object_type = TYPE_MAIN_VARIANT (object_type);
      if (TYPE_NAME (object_type)) {
        tree type_name = TYPE_NAME (object_type);
        if (TREE_CODE (type_name) == IDENTIFIER_NODE) {
          source->data.field_access.type_name =
            ggc_strdup (IDENTIFIER_POINTER (type_name));
        } else if (TREE_CODE (type_name) == TYPE_DECL && DECL_NAME (type_name)) {
          source->data.field_access.type_name =
            ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (type_name)));
        }
      }
    }
  } else if (TREE_CODE (field_ref) == MEM_REF) {
    tree base = TREE_OPERAND (field_ref, 0);
    source->data.field_access.base_object = base;
    source->data.field_access.field_name = ggc_strdup ("<mem_ref>");
  }

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractFromComputation
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromComputation (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree expr,
  gimple* final_stmt,
  gimple* original_stmt,
  location_t location,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)original_stmt;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_COMPUTATION;
  source->data.computation.compute_stmt = final_stmt;
  source->data.computation.compute_expr = expr;
  source->data.computation.location = location;

  enum tree_code code = TREE_CODE (expr);
  char const * desc = get_tree_code_name (code);
  source->data.computation.description = ggc_strdup (desc ? desc : "<computation>");

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractFromPhi
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromPhi (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  gimple* phi_stmt,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_PHI;
  source->data.phi.phi_stmt = phi_stmt;
  source->data.phi.ssa_name = ssa_name;
  source->data.phi.location = location;

  tree var = SSA_NAME_VAR (ssa_name);
  if (var) {
    source->data.phi.var_decl = var;
    if (DECL_NAME (var)) {
      source->data.phi.var_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (var)));
    }
  }

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractFromUnknown
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractFromUnknown (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree value,
  location_t location,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)value;
  (void)location;
  (void)function;
  (void)bb;

  WriteOriginalSource * source = ggc_alloc<WriteOriginalSource>();
  if (!source) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (source, 0, sizeof (WriteOriginalSource));

  source->source_type = SOURCE_UNKNOWN;

  result = source;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource_extractSource
// ============================================================================

ArrayDetectErrorCode traceWriteSource_extractSource (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  tree final_value,
  gimple* final_stmt,
  gimple* original_stmt,
  location_t location,
  tree function,
  basic_block bb,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  // SSA_NAME 且有函数调用定义
  if (TREE_CODE (final_value) == SSA_NAME) {
    if (final_stmt && gimple_code (final_stmt) == GIMPLE_CALL) {
      basic_block call_bb = gimple_bb (final_stmt);
      AD_ASSERT_GCC_LOGIC (call_bb, "gimple_bb returned NULL for call_stmt");
      AD_TRY (traceWriteSource_extractFromCall (
        AD_ARGS, detector, final_stmt, function, call_bb, result));
      AD_RETURNE (OK);
    }
    // 无法追踪到明确来源
    AD_TRY (traceWriteSource_extractFromUnknown (
      AD_ARGS, detector, final_value, location, function, bb, result));
    AD_RETURNE (OK);
  }

  // 常量
  if (CONSTANT_CLASS_P (final_value)) {
    AD_TRY (traceWriteSource_extractFromConstant (
      AD_ARGS, detector, final_value, function, bb, result));
    AD_RETURNE (OK);
  }

  // 字段访问
  enum tree_code final_code = TREE_CODE (final_value);
  if (final_code == COMPONENT_REF || final_code == MEM_REF) {
    AD_TRY (traceWriteSource_extractFromFieldAccess (
      AD_ARGS, detector, final_value, final_stmt, original_stmt,
      location, function, bb, result));
    AD_RETURNE (OK);
  }

  // 其他计算表达式
  AD_TRY (traceWriteSource_extractFromComputation (
    AD_ARGS, detector, final_value, final_stmt, original_stmt,
    location, function, bb, result));
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// traceWriteSource
// ============================================================================
// 主入口：追踪写入来源

ArrayDetectErrorCode traceWriteSource (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  FieldWriteInfo* write_info,
  WriteOriginalSource*& result
) AD_FUNCTION_BEGIN {
  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

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
    AD_TRY (traceWriteSource_extractFromPhi (
      AD_ARGS, detector, final_stmt, final_value, location, function, bb, result));
    AD_RETURNE (OK);
  }

  // 从最终值提取来源
  AD_TRY (traceWriteSource_extractSource (
    AD_ARGS, detector, final_value, final_stmt, stmt,
    location, function, bb, result));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 辅助函数：将 SourceType 转换为 FieldSourceKind
// ============================================================================

static field_analysis::FieldSourceKind convertSourceType (SourceType st) {
  switch (st) {
    case SOURCE_FUNCTION_CALL: return field_analysis::FIELD_SRC_FUNCTION_CALL;
    case SOURCE_CONSTANT:      return field_analysis::FIELD_SRC_CONSTANT;
    case SOURCE_FIELD_ACCESS:  return field_analysis::FIELD_SRC_FIELD_ACCESS;
    case SOURCE_COMPUTATION:   return field_analysis::FIELD_SRC_COMPUTATION;
    case SOURCE_PHI:           return field_analysis::FIELD_SRC_PHI;
    default:                   return field_analysis::FIELD_SRC_UNKNOWN;
  }
}

// ============================================================================
// 辅助函数：将 source_info 数据复制到 wrapper 的 source_data 中
// ============================================================================

static void copySourceDataToWrapper (
  FieldWriteAnalysisWrapper* wrapper,
  WriteOriginalSource const* source_info
) {
  if (!wrapper || !source_info) return;

  wrapper->source_kind = convertSourceType (source_info->source_type);

  switch (source_info->source_type) {
    case SOURCE_FUNCTION_CALL: {
      FunctionCallSource const &src = source_info->data.function_call;
      wrapper->source_data.function_call.stmt = src.call_stmt;
      wrapper->source_data.function_call.call_kind =
        (src.call_type == CALL_VIRTUAL) ? field_analysis::FIELD_CALL_VIRTUAL :
        (src.call_type == CALL_DIRECT)  ? field_analysis::FIELD_CALL_DIRECT :
        (src.call_type == CALL_INDIRECT) ? field_analysis::FIELD_CALL_INDIRECT :
        field_analysis::FIELD_CALL_UNKNOWN;
      wrapper->source_data.function_call.name = src.function_name;
      wrapper->source_data.function_call.location = src.location;
      break;
    }
    case SOURCE_CONSTANT: {
      ConstantSource const &src = source_info->data.constant;
      wrapper->source_data.constant.value = src.constant_value;
      wrapper->source_data.constant.str = src.constant_str;
      break;
    }
    case SOURCE_FIELD_ACCESS: {
      FieldAccessSource const &src = source_info->data.field_access;
      wrapper->source_data.field_access.stmt = src.access_stmt;
      wrapper->source_data.field_access.field = src.field_decl;
      wrapper->source_data.field_access.object = src.base_object;
      wrapper->source_data.field_access.object_type = src.object_type;
      wrapper->source_data.field_access.field_name = src.field_name;
      wrapper->source_data.field_access.type_name = src.type_name;
      wrapper->source_data.field_access.location = src.location;
      break;
    }
    case SOURCE_COMPUTATION: {
      ComputationSource const &src = source_info->data.computation;
      wrapper->source_data.computation.stmt = src.compute_stmt;
      wrapper->source_data.computation.expr = src.compute_expr;
      wrapper->source_data.computation.desc = src.description;
      wrapper->source_data.computation.location = src.location;
      break;
    }
    case SOURCE_PHI: {
      PhiSource const &src = source_info->data.phi;
      wrapper->source_data.phi.stmt = src.phi_stmt;
      wrapper->source_data.phi.ssa_name = src.ssa_name;
      wrapper->source_data.phi.var = src.var_decl;
      wrapper->source_data.phi.var_name = src.var_name;
      wrapper->source_data.phi.location = src.location;
      break;
    }
    default:
      wrapper->source_kind = field_analysis::FIELD_SRC_UNKNOWN;
      break;
  }
}

// ============================================================================
// traceFieldAssignments
// ============================================================================
// Pipeline 接口：追踪所有字段赋值

ArrayDetectErrorCode traceFieldAssignments (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Tracing field assignments (new module)");

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;
  unsigned int processed_count = 0;
  unsigned int source_extracted_count = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps *tfwo = (*iter).second;
    if (!tfwo || !tfwo->writes) {
      continue;
    }

    for (unsigned int j = 0; j < tfwo->writes->length (); ++j) {
      FieldWriteAnalysisWrapper *wrapper = (*tfwo->writes)[j];
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

      WriteOriginalSource* source_info = NULL;
      AD_TRY (traceWriteSource (AD_ARGS, detector, &temp_write_info, source_info));

      if (source_info) {
        copySourceDataToWrapper (wrapper, source_info);
        source_extracted_count++;

        // 调试输出
        TypeFieldKey key = (*iter).first;
        AD_TRY (printFieldWriteSourceInfoFromWrapper (AD_ARGS, key.type, key.field_decl, wrapper, source_info));
      }
    }
  }

  AD_DEBUG_PRINT ("Tracing complete: %u write operations processed, %u sources extracted",
                  processed_count, source_extracted_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
