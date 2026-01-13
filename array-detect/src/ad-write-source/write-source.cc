// ============================================================================
// ad-write-source 模块实现
// ============================================================================
// 追踪字段写入的来源
// 数据流：FieldWriteInfo -> WriteOriginalSource
// ============================================================================

#include "write-source.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
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
// 输出：填充 WriteOriginalSource 的 function_call 部分

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromCall (
  AD_FUNC_ARGS,
  gimple* call_stmt,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_FUNCTION_CALL;
  FunctionCallSource& fc = out_source->data.function_call;

  fc.call_stmt = call_stmt;
  fc.location = gimple_location (call_stmt);

  tree fn = gimple_call_fn (call_stmt);
  if (fn && TREE_CODE (fn) == OBJ_TYPE_REF) {
    fc.call_type = CALL_VIRTUAL;
    // Try to extract actual virtual method name
    char const* extracted_name = nullptr;
    if (extractVirtualCallFunctionName (AD_ARGS, call_stmt, extracted_name) == OK && extracted_name) {
      fc.function_name = ggc_strdup (extracted_name);
    } else {
      fc.function_name = ggc_strdup ("<virtual>");
    }
  } else if (fn && TREE_CODE (fn) == ADDR_EXPR) {
    tree fn_decl = TREE_OPERAND (fn, 0);
    if (fn_decl && DECL_NAME (fn_decl)) {
      fc.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fn_decl)));
      fc.call_type = CALL_DIRECT;
    } else {
      fc.function_name = ggc_strdup ("<unknown>");
      fc.call_type = CALL_UNKNOWN;
    }
  } else if (fn && TREE_CODE (fn) == SSA_NAME) {
    // SSA_NAME may be from OBJ_TYPE_REF - check the definition
    gimple* def_stmt = SSA_NAME_DEF_STMT (fn);
    if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (rhs && TREE_CODE (rhs) == OBJ_TYPE_REF) {
        fc.call_type = CALL_VIRTUAL;
        char const* extracted_name = nullptr;
        if (extractVirtualCallFunctionName (AD_ARGS, call_stmt, extracted_name) == OK && extracted_name) {
          fc.function_name = ggc_strdup (extracted_name);
        } else {
          fc.function_name = ggc_strdup ("<virtual>");
        }
      } else {
        fc.call_type = CALL_INDIRECT;
        fc.function_name = ggc_strdup ("<indirect>");
      }
    } else {
      fc.call_type = CALL_INDIRECT;
      fc.function_name = ggc_strdup ("<indirect>");
    }
  } else {
    fc.call_type = CALL_INDIRECT;
    fc.function_name = ggc_strdup ("<indirect>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromConstant
// ----------------------------------------------------------------------------
// 输出：填充 WriteOriginalSource 的 constant 部分

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromConstant (
  AD_FUNC_ARGS,
  tree constant_value,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_CONSTANT;
  ConstantSource& cs = out_source->data.constant;

  cs.constant_value = constant_value;

  if (integer_zerop (constant_value)) {
    cs.constant_str = ggc_strdup ("0");
  } else if (TREE_CODE (constant_value) == INTEGER_CST) {
    char buf[64];
    snprintf (buf, sizeof (buf), "%ld", (long)TREE_INT_CST_LOW (constant_value));
    cs.constant_str = ggc_strdup (buf);
  } else {
    cs.constant_str = ggc_strdup ("<constant>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromFieldAccess
// ----------------------------------------------------------------------------
// 输出：填充 WriteOriginalSource 的 field_access 部分

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromFieldAccess (
  AD_FUNC_ARGS,
  tree field_ref,
  gimple* final_stmt,
  location_t location,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_FIELD_ACCESS;
  FieldAccessSource& fa = out_source->data.field_access;

  fa.access_stmt = final_stmt;
  fa.access_expr = field_ref;
  fa.location = location;

  if (TREE_CODE (field_ref) == COMPONENT_REF) {
    tree field_decl = TREE_OPERAND (field_ref, 1);
    tree base = TREE_OPERAND (field_ref, 0);

    fa.field_decl = field_decl;
    fa.base_object = base;

    if (field_decl && DECL_NAME (field_decl)) {
      fa.field_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (field_decl)));
    } else {
      fa.field_name = ggc_strdup ("<anonymous>");
    }

    tree object_type = TREE_TYPE (base);
    if (object_type) {
      if (TREE_CODE (object_type) == POINTER_TYPE ||
          TREE_CODE (object_type) == REFERENCE_TYPE) {
        object_type = TREE_TYPE (object_type);
      }
      fa.object_type = TYPE_MAIN_VARIANT (object_type);
      if (TYPE_NAME (object_type)) {
        tree type_name = TYPE_NAME (object_type);
        if (TREE_CODE (type_name) == IDENTIFIER_NODE) {
          fa.type_name = ggc_strdup (IDENTIFIER_POINTER (type_name));
        } else if (TREE_CODE (type_name) == TYPE_DECL && DECL_NAME (type_name)) {
          fa.type_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (type_name)));
        }
      }
    }
  } else if (TREE_CODE (field_ref) == MEM_REF) {
    tree base = TREE_OPERAND (field_ref, 0);
    fa.base_object = base;
    fa.field_name = ggc_strdup ("<mem_ref>");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromComputation
// ----------------------------------------------------------------------------
// 输出：填充 WriteOriginalSource 的 computation 部分

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromComputation (
  AD_FUNC_ARGS,
  tree expr,
  gimple* final_stmt,
  location_t location,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_COMPUTATION;
  ComputationSource& cp = out_source->data.computation;

  cp.compute_stmt = final_stmt;
  cp.compute_expr = expr;
  cp.location = location;

  enum tree_code code = TREE_CODE (expr);
  char const * desc = get_tree_code_name (code);
  cp.description = ggc_strdup (desc ? desc : "<computation>");

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromPhi
// ----------------------------------------------------------------------------
// 输出：填充 WriteOriginalSource 的 phi 部分

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromPhi (
  AD_FUNC_ARGS,
  gimple* phi_stmt,
  tree ssa_name,
  location_t location,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_PHI;
  PhiSource& ps = out_source->data.phi;

  ps.phi_stmt = phi_stmt;
  ps.ssa_name = ssa_name;
  ps.location = location;

  tree var = SSA_NAME_VAR (ssa_name);
  if (var) {
    ps.var_decl = var;
    if (DECL_NAME (var)) {
      ps.var_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (var)));
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource_extractFromUnknown
// ----------------------------------------------------------------------------
// 输出：设置 WriteOriginalSource 为未知

ArrayDetectErrorCode traceWriteSource_extractSource_extractFromUnknown (
  AD_FUNC_ARGS,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  if (!out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  out_source->source_type = SOURCE_UNKNOWN;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// traceWriteSource_extractSource
// ----------------------------------------------------------------------------
// 从最终值提取来源，写入 WriteOriginalSource

ArrayDetectErrorCode traceWriteSource_extractSource (
  AD_FUNC_ARGS,
  tree final_value,
  gimple* final_stmt,
  location_t location,
  WriteOriginalSource* out_source
) AD_FUNCTION_BEGIN {
  // SSA_NAME 且有函数调用定义
  if (TREE_CODE (final_value) == SSA_NAME) {
    if (final_stmt && gimple_code (final_stmt) == GIMPLE_CALL) {
      AD_TRY (traceWriteSource_extractSource_extractFromCall (
        AD_ARGS, final_stmt, out_source));
      AD_RETURNE (OK);
    }
    // 无法追踪到明确来源
    AD_TRY (traceWriteSource_extractSource_extractFromUnknown (AD_ARGS, out_source));
    AD_RETURNE (OK);
  }

  // 常量
  if (CONSTANT_CLASS_P (final_value)) {
    AD_TRY (traceWriteSource_extractSource_extractFromConstant (
      AD_ARGS, final_value, out_source));
    AD_RETURNE (OK);
  }

  // 字段访问
  enum tree_code final_code = TREE_CODE (final_value);
  if (final_code == COMPONENT_REF || final_code == MEM_REF) {
    AD_TRY (traceWriteSource_extractSource_extractFromFieldAccess (
      AD_ARGS, final_value, final_stmt, location, out_source));
    AD_RETURNE (OK);
  }

  // 其他计算表达式
  AD_TRY (traceWriteSource_extractSource_extractFromComputation (
    AD_ARGS, final_value, final_stmt, location, out_source));
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
// 输出：WriteOriginalSource 指针

ArrayDetectErrorCode traceWriteSource (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  FieldWriteInfo* write_info,
  WriteOriginalSource** out_source
) AD_FUNCTION_BEGIN {
  if (!write_info || !out_source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果
  *out_source = ggc_alloc<WriteOriginalSource> ();
  if (!*out_source) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // 清零
  memset (*out_source, 0, sizeof (WriteOriginalSource));
  (*out_source)->source_type = SOURCE_UNKNOWN;

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
      AD_ARGS, final_stmt, final_value, location, *out_source));
    AD_RETURNE (OK);
  }

  // 从最终值提取来源
  AD_TRY (traceWriteSource_extractSource (
    AD_ARGS, final_value, final_stmt, location, *out_source));

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
    TypeFieldAnalysisData *tfad = (*iter).second;
    if (!tfad || !tfad->writes) {
      continue;
    }

    for (unsigned int j = 0; j < tfad->writes->length (); ++j) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo *wrapper = (*tfad->writes)[j];
      if (!wrapper || !wrapper->write_info) {
        continue;
      }

      processed_count++;

      // 直接追踪写入来源
      AD_TRY (traceWriteSource (AD_ARGS, detector, wrapper->write_info,
        &wrapper->write_source));
      source_extracted_count++;
    }
  }

  AD_DEBUG_PRINT ("Tracing complete: %u write operations processed, %u sources extracted",
                  processed_count, source_extracted_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
