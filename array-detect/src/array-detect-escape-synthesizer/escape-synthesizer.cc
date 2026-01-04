#include "escape-synthesizer.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// ============================================================================
// 辅助函数：安全调试函数识别
// ============================================================================

bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!function_name) {
    return false;
  }

  // 已知安全调试函数列表
  static char const * safe_debug_functions[] = {
    "printf",
    "fprintf",
    "sprintf",
    "snprintf",
    "vprintf",
    "vfprintf",
    "vsprintf",
    "vsnprintf",
    "puts",
    "fputs",
    "putchar",
    "fputc",
    "perror",
    "std::cout",
    "std::cerr",
    "std::clog",
    NULL
  };

  for (int i = 0; safe_debug_functions[i] != NULL; i++) {
    if (strcmp (function_name, safe_debug_functions[i]) == 0) {
      return true;
    }
    // 也检查包含该名称的情况（如 __printf_chk）
    if (strstr (function_name, safe_debug_functions[i]) != NULL) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// 辅助函数：算术运算识别
// ============================================================================

bool isArithmeticOperation (
  gimple * stmt,
  char const * &operation_name
) {
  if (!stmt || gimple_code (stmt) != GIMPLE_ASSIGN) {
    return false;
  }

  enum tree_code code = gimple_assign_rhs_code (stmt);

  switch (code) {
    // 算术运算
    case PLUS_EXPR:
      operation_name = "+";
      return true;
    case MINUS_EXPR:
      operation_name = "-";
      return true;
    case MULT_EXPR:
      operation_name = "*";
      return true;
    case TRUNC_DIV_EXPR:
    case EXACT_DIV_EXPR:
    case CEIL_DIV_EXPR:
    case FLOOR_DIV_EXPR:
    case ROUND_DIV_EXPR:
      operation_name = "/";
      return true;
    case TRUNC_MOD_EXPR:
    case CEIL_MOD_EXPR:
    case FLOOR_MOD_EXPR:
    case ROUND_MOD_EXPR:
      operation_name = "%";
      return true;

    // 位运算
    case BIT_AND_EXPR:
      operation_name = "&";
      return true;
    case BIT_IOR_EXPR:
      operation_name = "|";
      return true;
    case BIT_XOR_EXPR:
      operation_name = "^";
      return true;
    case LSHIFT_EXPR:
      operation_name = "<<";
      return true;
    case RSHIFT_EXPR:
      operation_name = ">>";
      return true;
    case BIT_NOT_EXPR:
      operation_name = "~";
      return true;

    default:
      return false;
  }
}

// ============================================================================
// 辅助函数：分类单个逃逸使用
// ============================================================================

ArrayDetectErrorCode classifyEscapeUse (
  AD_FUNC_ARGS,
  SourceUseInfo const &use_info,
  EscapeSynthesisResult * result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("[classifyEscapeUse] Classifying escape kind: %s",
                  getEscapeKindString (use_info.escape_kind));

  switch (use_info.escape_kind) {
    case SU_ESCAPE_NONE:
      // 这不应该出现在逃逸使用中
      AD_RETURNE (OK);

    case SU_ESCAPE_RETURN:
      {
        ESC_SYNTH_ADD (*result, ESC_SYNTH_RETURN_ESCAPE);
        if (!result->return_escape) {
          result->return_escape = ggc_alloc<DirectEscapeInfo> ();
          memset (result->return_escape, 0, sizeof (DirectEscapeInfo));
          result->return_escape->escape_stmts = NULL;
          result->return_escape->targets = NULL;
          result->return_escape->count = 0;
        }

        // 添加逃逸语句
        vec_safe_push (result->return_escape->escape_stmts, use_info.use_stmt);
        vec_safe_push (result->return_escape->targets,
                      use_info.escape_target ? use_info.escape_target : "<return>");
        result->return_escape->count++;
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_PARAMETER:
      {
        ESC_SYNTH_ADD (*result, ESC_SYNTH_PARAMETER_ESCAPE);
        if (!result->parameter_escape) {
          result->parameter_escape = ggc_alloc<DirectEscapeInfo> ();
          memset (result->parameter_escape, 0, sizeof (DirectEscapeInfo));
          result->parameter_escape->escape_stmts = NULL;
          result->parameter_escape->targets = NULL;
          result->parameter_escape->count = 0;
        }

        vec_safe_push (result->parameter_escape->escape_stmts, use_info.use_stmt);
        vec_safe_push (result->parameter_escape->targets,
                      use_info.escape_target ? use_info.escape_target : "<parameter>");
        result->parameter_escape->count++;
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_GLOBAL_STORE:
      {
        ESC_SYNTH_ADD (*result, ESC_SYNTH_GLOBAL_ESCAPE);
        if (!result->global_escape) {
          result->global_escape = ggc_alloc<DirectEscapeInfo> ();
          memset (result->global_escape, 0, sizeof (DirectEscapeInfo));
          result->global_escape->escape_stmts = NULL;
          result->global_escape->targets = NULL;
          result->global_escape->count = 0;
        }

        vec_safe_push (result->global_escape->escape_stmts, use_info.use_stmt);
        vec_safe_push (result->global_escape->targets,
                      use_info.escape_target ? use_info.escape_target : "<global>");
        result->global_escape->count++;
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_HEAP_STORE:
      {
        ESC_SYNTH_ADD (*result, ESC_SYNTH_HEAP_ESCAPE);
        if (!result->heap_escape) {
          result->heap_escape = ggc_alloc<HeapEscapeInfo> ();
          memset (result->heap_escape, 0, sizeof (HeapEscapeInfo));
          result->heap_escape->heap_stores = NULL;
          result->heap_escape->stored_values = NULL;
          result->heap_escape->count = 0;
        }

        vec_safe_push (result->heap_escape->heap_stores, use_info.use_stmt);
        vec_safe_push (result->heap_escape->stored_values, use_info.use_operand);
        result->heap_escape->count++;
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_FIELD_STORE:
      {
        ESC_SYNTH_ADD (*result, ESC_SYNTH_FIELD_ESCAPE);
        if (!result->field_escape) {
          result->field_escape = ggc_alloc<FieldEscapeInfo> ();
          memset (result->field_escape, 0, sizeof (FieldEscapeInfo));
          result->field_escape->field_stores = NULL;
          result->field_escape->field_decls = NULL;
          result->field_escape->field_names = NULL;
          result->field_escape->count = 0;
        }

        vec_safe_push (result->field_escape->field_stores, use_info.use_stmt);
        vec_safe_push (result->field_escape->field_decls, use_info.target_info.field_decl);

        // 提取字段名
        char const * field_name = use_info.escape_target;
        if (!field_name && use_info.target_info.field_decl &&
            DECL_NAME (use_info.target_info.field_decl)) {
          field_name = IDENTIFIER_POINTER (DECL_NAME (use_info.target_info.field_decl));
        }
        if (!field_name) {
          field_name = "<unknown-field>";
        }
        vec_safe_push (result->field_escape->field_names, ggc_strdup (field_name));
        result->field_escape->count++;
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_INDIRECT_CALL:
    case SU_ESCAPE_VIRTUAL_CALL:
    case SU_ESCAPE_EXTERNAL_CALL:
      {
        // 检查是否为安全调试函数
        char const * func_name = use_info.escape_target;

        if (func_name && isKnownSafeDebugFunction (AD_ARGS, func_name)) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_SAFE_DEBUG);
          if (!result->safe_debug) {
            result->safe_debug = ggc_alloc<SafeDebugEscape> ();
            memset (result->safe_debug, 0, sizeof (SafeDebugEscape));
            result->safe_debug->debug_calls = NULL;
            result->safe_debug->function_names = NULL;
            result->safe_debug->count = 0;
          }

          vec_safe_push (result->safe_debug->debug_calls, use_info.use_stmt);
          vec_safe_push (result->safe_debug->function_names,
                        ggc_strdup (func_name ? func_name : "<unknown>"));
          result->safe_debug->count++;
        } else {
          // 未知函数调用
          ESC_SYNTH_ADD (*result, ESC_SYNTH_UNKNOWN_CALL);
          if (!result->unknown_call) {
            result->unknown_call = ggc_alloc<UnknownCallEscape> ();
            memset (result->unknown_call, 0, sizeof (UnknownCallEscape));
            result->unknown_call->unknown_calls = NULL;
            result->unknown_call->call_descriptions = NULL;
            result->unknown_call->count = 0;
          }

          vec_safe_push (result->unknown_call->unknown_calls, use_info.use_stmt);
          vec_safe_push (result->unknown_call->call_descriptions,
                        ggc_strdup (func_name ? func_name : "<unknown-call>"));
          result->unknown_call->count++;
        }

        // 同时记录虚函数或间接调用
        if (use_info.escape_kind == SU_ESCAPE_VIRTUAL_CALL) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_VIRTUAL_CALL);
          if (!result->virtual_call) {
            result->virtual_call = ggc_alloc<CallEscapeInfo> ();
            memset (result->virtual_call, 0, sizeof (CallEscapeInfo));
            result->virtual_call->call_stmts = NULL;
            result->virtual_call->call_names = NULL;
            result->virtual_call->count = 0;
          }

          vec_safe_push (result->virtual_call->call_stmts, use_info.use_stmt);
          vec_safe_push (result->virtual_call->call_names,
                        ggc_strdup (func_name ? func_name : "<virtual-call>"));
          result->virtual_call->count++;
        } else if (use_info.escape_kind == SU_ESCAPE_INDIRECT_CALL) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_INDIRECT_CALL);
          if (!result->indirect_call) {
            result->indirect_call = ggc_alloc<CallEscapeInfo> ();
            memset (result->indirect_call, 0, sizeof (CallEscapeInfo));
            result->indirect_call->call_stmts = NULL;
            result->indirect_call->call_names = NULL;
            result->indirect_call->count = 0;
          }

          vec_safe_push (result->indirect_call->call_stmts, use_info.use_stmt);
          vec_safe_push (result->indirect_call->call_names,
                        ggc_strdup (func_name ? func_name : "<indirect-call>"));
          result->indirect_call->count++;
        }
      }
      AD_RETURNE (OK);

    case SU_ESCAPE_UNKNOWN:
    default:
      {
        // 检查是否为算术运算
        char const * operation_name = NULL;
        if (use_info.use_stmt && isArithmeticOperation (use_info.use_stmt, operation_name)) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_ARITHMETIC_POTENTIAL);
          if (!result->arithmetic_potential) {
            result->arithmetic_potential = ggc_alloc<ArithmeticPotentialEscape> ();
            memset (result->arithmetic_potential, 0, sizeof (ArithmeticPotentialEscape));
            result->arithmetic_potential->arithmetic_stmts = NULL;
            result->arithmetic_potential->operations = NULL;
            result->arithmetic_potential->count = 0;
          }

          vec_safe_push (result->arithmetic_potential->arithmetic_stmts, use_info.use_stmt);
          vec_safe_push (result->arithmetic_potential->operations, ggc_strdup (operation_name));
          result->arithmetic_potential->count++;
        } else {
          // 其他未知逃逸，归类为未知函数调用
          ESC_SYNTH_ADD (*result, ESC_SYNTH_UNKNOWN_CALL);
          if (!result->unknown_call) {
            result->unknown_call = ggc_alloc<UnknownCallEscape> ();
            memset (result->unknown_call, 0, sizeof (UnknownCallEscape));
            result->unknown_call->unknown_calls = NULL;
            result->unknown_call->call_descriptions = NULL;
            result->unknown_call->count = 0;
          }

          vec_safe_push (result->unknown_call->unknown_calls, use_info.use_stmt);
          vec_safe_push (result->unknown_call->call_descriptions,
                        ggc_strdup ("<unknown-escape>"));
          result->unknown_call->count++;
        }
      }
      AD_RETURNE (OK);
  }
} AD_FUNCTION_END

// ============================================================================
// 核心综合函数
// ============================================================================

ArrayDetectErrorCode synthesizeEscapeInfo (
  AD_FUNC_ARGS,
  SourceUseAnalysisResult * raw_result,
  EscapeSynthesisResult * &result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("[synthesizeEscapeInfo] Synthesizing escape info");

  if (!raw_result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配综合结果结构
  EscapeSynthesisResult * synth_result = ggc_alloc<EscapeSynthesisResult> ();
  if (!synth_result) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (synth_result, 0, sizeof (EscapeSynthesisResult));

  // 初始化字段
  synth_result->category_bitmap = ESC_SYNTH_NONE;
  synth_result->raw_result = raw_result;
  synth_result->total_uses = raw_result->total_use_count;
  synth_result->total_escapes = raw_result->escape_count;
  synth_result->category_count = 0;

  // 如果没有逃逸，标记为无逃逸
  if (!raw_result->has_escape || raw_result->escape_count == 0) {
    ESC_SYNTH_ADD (*synth_result, ESC_SYNTH_NO_ESCAPE);
    synth_result->no_escape = ggc_alloc<NoEscapeInfo> ();
    memset (synth_result->no_escape, 0, sizeof (NoEscapeInfo));
    synth_result->no_escape->field_access_expr = raw_result->source_operand;
    synth_result->no_escape->location = gimple_location (raw_result->source_stmt);
    synth_result->no_escape->description = ggc_strdup ("No escape detected");
    synth_result->category_count = 1;

    AD_RETURNO (synth_result);
  }

  // 遍历所有使用，分类逃逸的使用
  if (raw_result->all_uses) {
    unsigned int use_count = raw_result->all_uses->length ();
    AD_DEBUG_PRINT ("[synthesizeEscapeInfo] Processing %u uses for escapes", use_count);

    for (unsigned int i = 0; i < use_count; i++) {
      SourceUseInfo const &use_info = (*raw_result->all_uses)[i];
      if (use_info.is_escape()) {
        AD_TRY (classifyEscapeUse (AD_ARGS, use_info, synth_result));
      }
    }
  }

  // 计算类别数量
  synth_result->category_count = __builtin_popcount (synth_result->category_bitmap);

  AD_DEBUG_PRINT ("[synthesizeEscapeInfo] Synthesis complete: %u categories, bitmap=0x%x",
                  synth_result->category_count, synth_result->category_bitmap);

  AD_RETURNO (synth_result);
} AD_FUNCTION_END

// ============================================================================
// 综合所有字段的逃逸信息
// ============================================================================

ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<EscapeSynthesisResult*> * &synthesis_results,
  unsigned int &total_synthesized
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Synthesizing all field escapes");

  total_synthesized = 0;

  if (!detector.m_type_field_writes) {
    synthesis_results = NULL;
    AD_RETURNE (OK);
  }

  // 分配结果向量
  synthesis_results = ggc_alloc<vec<EscapeSynthesisResult*>> ();
  synthesis_results->create (0);

  // 遍历所有 (type, field) 的写入操作
  typedef hash_map<array_detector::TypeFieldKey, array_detector::TypeFieldWriteOps*, array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    array_detector::TypeFieldKey const &key = (*iter).first;
    array_detector::TypeFieldWriteOps * write_ops = (*iter).second;

    if (!write_ops || !write_ops->write_analysis_records) continue;

    // 获取类型名和字段名（用于调试输出）
    char const * type_name = NULL;
    AD_TRY (gcc_ext_util::formatTypeNameWithNamespace (AD_ARGS, key.type, type_name));
    char const * field_name = NULL;
    if (key.field_decl && DECL_NAME (key.field_decl)) {
      field_name = IDENTIFIER_POINTER (DECL_NAME (key.field_decl));
    }

    AD_DEBUG_PRINT ("  Processing (type=%s, field=%s): %u write operations",
                    type_name ? type_name : "<unknown>",
                    field_name ? field_name : "<unknown>",
                    write_ops->write_analysis_records->length ());

    // 为该 (type, field) 分配临时结果向量
    vec<EscapeSynthesisResult*> * field_synth_results = ggc_alloc<vec<EscapeSynthesisResult*>> ();
    field_synth_results->create (0);

    // 遍历该 (type, field) 的所有写入操作
    for (unsigned i = 0; i < write_ops->write_analysis_records->length (); i++) {
      array_detector::FieldWriteAnalysisRecord * record = (*write_ops->write_analysis_records)[i];
      if (!record || !record->write_capture) continue;

      // 从 record->escape_analysis 读取逃逸分析结果
      SourceUseAnalysisResult * raw_result = record->escape_analysis;
      if (!raw_result) {
        AD_DEBUG_PRINT ("    Write #%u: No escape analysis result", i);
        continue;
      }

      // 综合该写入操作的逃逸信息
      EscapeSynthesisResult * synth_result = NULL;
      AD_TRY (synthesizeEscapeInfo (AD_ARGS, raw_result, synth_result));

      if (synth_result) {
        // === 新设计：直接填充到分析记录中 ===
        // 不再使用 original_write_info，直接填充到 record->escape_synthesis
        record->escape_synthesis = synth_result;

        // 添加到该字段的临时结果列表
        field_synth_results->safe_push (synth_result);

        // 也添加到全局结果列表
        synthesis_results->safe_push (synth_result);
        total_synthesized++;

        AD_DEBUG_PRINT ("    Write #%u: Synthesized - categories=0x%x, uses=%u, escapes=%u",
                        i, synth_result->category_bitmap,
                        synth_result->total_uses, synth_result->total_escapes);
      }
    }

    // === 打印该字段的详细逃逸综合结果 ===
    AD_DEBUG_PRINT ("  Printing detailed escape synthesis results for (type=%s, field=%s)",
                    type_name ? type_name : "<unknown>",
                    field_name ? field_name : "<unknown>");
    printFieldEscapeSynthesisResults (AD_ARGS, key.type, key.field_decl, field_synth_results, ctx.debug_file);

    // === 进行二级综合：所有权分析 ===
    AD_DEBUG_PRINT ("  Performing ownership analysis for (type=%s, field=%s)",
                    type_name ? type_name : "<unknown>",
                    field_name ? field_name : "<unknown>");
    OwnershipAnalysisResult * ownership_result = NULL;
    AD_TRY (analyzeFieldOwnershipSupport (AD_ARGS, field_synth_results, key.type, key.field_decl, ownership_result));

    if (ownership_result) {
      AD_DEBUG_PRINT ("  Ownership analysis complete: verdict=%s",
                      getOwnershipVerdictString (ownership_result->verdict));
      printOwnershipAnalysisResult (ownership_result, ctx.debug_file);
    }
  }

  AD_DEBUG_PRINT ("Synthesis complete: %u results", total_synthesized);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 辅助函数：获取类别描述字符串
// ============================================================================

char const * getEscapeCategoryString (unsigned int category) {
  switch (category) {
    case ESC_SYNTH_NO_ESCAPE: return "NO_ESCAPE";
    case ESC_SYNTH_ARITHMETIC_POTENTIAL: return "ARITHMETIC_POTENTIAL";
    case ESC_SYNTH_SAFE_DEBUG: return "SAFE_DEBUG";
    case ESC_SYNTH_UNKNOWN_CALL: return "UNKNOWN_CALL";
    case ESC_SYNTH_HEAP_ESCAPE: return "HEAP_ESCAPE";
    case ESC_SYNTH_RETURN_ESCAPE: return "RETURN_ESCAPE";
    case ESC_SYNTH_PARAMETER_ESCAPE: return "PARAMETER_ESCAPE";
    case ESC_SYNTH_GLOBAL_ESCAPE: return "GLOBAL_ESCAPE";
    case ESC_SYNTH_FIELD_ESCAPE: return "FIELD_ESCAPE";
    case ESC_SYNTH_VIRTUAL_CALL: return "VIRTUAL_CALL";
    case ESC_SYNTH_INDIRECT_CALL: return "INDIRECT_CALL";
    default: return "UNKNOWN";
  }
}

// ============================================================================
// 二级综合器实现：所有权分析
// ============================================================================

// 判断单个逃逸综合结果是否反对 owned
bool isEscapeResultRejectingOwnership (
  EscapeSynthesisResult const * synth_result,
  unsigned int &rejection_reasons
) {
  if (!synth_result) return false;

  rejection_reasons = REJECT_NONE;
  bool is_rejecting = false;

  // 检查各种反对 owned 的逃逸类别
  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_HEAP_ESCAPE)) {
    rejection_reasons |= REJECT_HEAP_ESCAPE;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_RETURN_ESCAPE)) {
    rejection_reasons |= REJECT_RETURN_ESCAPE;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_PARAMETER_ESCAPE)) {
    rejection_reasons |= REJECT_PARAMETER_ESCAPE;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_GLOBAL_ESCAPE)) {
    rejection_reasons |= REJECT_GLOBAL_ESCAPE;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_VIRTUAL_CALL)) {
    rejection_reasons |= REJECT_VIRTUAL_CALL;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_INDIRECT_CALL)) {
    rejection_reasons |= REJECT_INDIRECT_CALL;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_UNKNOWN_CALL)) {
    rejection_reasons |= REJECT_UNKNOWN_CALL;
    is_rejecting = true;
  }

  if (ESC_SYNTH_HAS (*synth_result, ESC_SYNTH_FIELD_ESCAPE)) {
    rejection_reasons |= REJECT_FIELD_ESCAPE;
    is_rejecting = true;
  }

  // 不反对的类别：
  // - NO_ESCAPE: 无逃逸，支持 owned
  // - ARITHMETIC_POTENTIAL: 算术运算，不影响所有权
  // - SAFE_DEBUG: 安全调试函数，不影响所有权

  return is_rejecting;
}

// 分析单个 (type, field) 的所有权支持情况
ArrayDetectErrorCode analyzeFieldOwnershipSupport (
  AD_FUNC_ARGS,
  vec<EscapeSynthesisResult*> * write_results,
  tree type,
  tree field_decl,
  OwnershipAnalysisResult * &result
) AD_FUNCTION_BEGIN {
  if (!write_results) {
    result = NULL;
    AD_RETURNE (OK);
  }

  // 分配结果结构
  result = ggc_alloc<OwnershipAnalysisResult> ();
  memset (result, 0, sizeof (OwnershipAnalysisResult));

  // 设置标识信息
  result->type = type;
  result->field_decl = field_decl;
  result->all_write_results = write_results;

  // 获取类型名和字段名
  AD_TRY (gcc_ext_util::formatTypeNameWithNamespace (AD_ARGS, type, result->type_name));
  if (field_decl && DECL_NAME (field_decl)) {
    result->field_name = IDENTIFIER_POINTER (DECL_NAME (field_decl));
  } else {
    result->field_name = "<anonymous>";
  }

  // 初始化统计信息
  result->total_writes = write_results->length ();
  result->supporting_writes = 0;
  result->rejecting_writes = 0;
  result->uncertain_writes = 0;
  result->rejection_reasons = REJECT_NONE;

  // 默认假设：支持 owned
  result->verdict = OWNERSHIP_VERDICT_SUPPORTED;

  // 遍历所有写入操作的综合结果
  for (unsigned i = 0; i < write_results->length (); i++) {
    EscapeSynthesisResult * synth = (*write_results)[i];
    if (!synth) continue;

    unsigned int write_rejection_reasons = REJECT_NONE;
    bool is_rejecting = isEscapeResultRejectingOwnership (synth, write_rejection_reasons);

    if (is_rejecting) {
      result->rejecting_writes++;
      result->rejection_reasons |= write_rejection_reasons;
      result->verdict = OWNERSHIP_VERDICT_REJECTED;
    } else {
      result->supporting_writes++;
    }
  }

  // 生成结论描述
  if (result->verdict == OWNERSHIP_VERDICT_SUPPORTED) {
    result->verdict_description = "No evidence against owned pointer (supported)";
  } else if (result->verdict == OWNERSHIP_VERDICT_REJECTED) {
    result->verdict_description = "Evidence against owned pointer found (rejected)";
  } else {
    result->verdict_description = "Uncertain ownership (needs more analysis)";
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// 获取结论描述字符串
char const * getOwnershipVerdictString (OwnershipSupportVerdict verdict) {
  switch (verdict) {
    case OWNERSHIP_VERDICT_SUPPORTED: return "SUPPORTED (no objection)";
    case OWNERSHIP_VERDICT_REJECTED: return "REJECTED (evidence against)";
    case OWNERSHIP_VERDICT_UNCERTAIN: return "UNCERTAIN";
    default: return "UNKNOWN";
  }
}

// 获取反对原因描述字符串
char const * getOwnershipRejectionReasonString (unsigned int reason) {
  switch (reason) {
    case REJECT_HEAP_ESCAPE: return "HEAP_ESCAPE";
    case REJECT_RETURN_ESCAPE: return "RETURN_ESCAPE";
    case REJECT_PARAMETER_ESCAPE: return "PARAMETER_ESCAPE";
    case REJECT_GLOBAL_ESCAPE: return "GLOBAL_ESCAPE";
    case REJECT_VIRTUAL_CALL: return "VIRTUAL_CALL";
    case REJECT_INDIRECT_CALL: return "INDIRECT_CALL";
    case REJECT_UNKNOWN_CALL: return "UNKNOWN_CALL";
    case REJECT_FIELD_ESCAPE: return "FIELD_ESCAPE";
    default: return "UNKNOWN";
  }
}

// 打印所有权分析结果
void printOwnershipAnalysisResult (
  OwnershipAnalysisResult const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "\n");
  fprintf (output, "=== Ownership Analysis Result ===\n");
  fprintf (output, "Type: %s\n", result->type_name ? result->type_name : "<unknown>");
  fprintf (output, "Field: %s\n", result->field_name ? result->field_name : "<unknown>");
  fprintf (output, "\n");
  fprintf (output, "Verdict: %s\n", getOwnershipVerdictString (result->verdict));
  fprintf (output, "Description: %s\n", result->verdict_description ? result->verdict_description : "");
  fprintf (output, "\n");
  fprintf (output, "Statistics:\n");
  fprintf (output, "  Total writes: %u\n", result->total_writes);
  fprintf (output, "  Supporting writes (no objection): %u\n", result->supporting_writes);
  fprintf (output, "  Rejecting writes (with objection): %u\n", result->rejecting_writes);
  fprintf (output, "  Uncertain writes: %u\n", result->uncertain_writes);
  fprintf (output, "\n");

  if (result->rejection_reasons != REJECT_NONE) {
    fprintf (output, "Rejection Reasons:\n");
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_HEAP_ESCAPE)) {
      fprintf (output, "  - HEAP_ESCAPE: Escapes to heap (may be shared)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_RETURN_ESCAPE)) {
      fprintf (output, "  - RETURN_ESCAPE: Escapes via return (ownership transfer)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_PARAMETER_ESCAPE)) {
      fprintf (output, "  - PARAMETER_ESCAPE: Escapes via parameter (may be shared)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_GLOBAL_ESCAPE)) {
      fprintf (output, "  - GLOBAL_ESCAPE: Escapes to global variable (may be shared)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_VIRTUAL_CALL)) {
      fprintf (output, "  - VIRTUAL_CALL: Escapes via virtual call (uncertain behavior)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_INDIRECT_CALL)) {
      fprintf (output, "  - INDIRECT_CALL: Escapes via indirect call (uncertain behavior)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_UNKNOWN_CALL)) {
      fprintf (output, "  - UNKNOWN_CALL: Escapes via unknown call (uncertain behavior)\n");
    }
    if (OWNERSHIP_HAS_REJECTION (*result, REJECT_FIELD_ESCAPE)) {
      fprintf (output, "  - FIELD_ESCAPE: Escapes to other fields (may be shared)\n");
    }
  } else {
    fprintf (output, "No rejection reasons (all writes support owned pointer assumption)\n");
  }

  fprintf (output, "=================================\n");
}

// 打印字段的所有写入操作的逃逸综合结果
void printFieldEscapeSynthesisResults (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<EscapeSynthesisResult*> * write_results,
  FILE * output
) {
  if (!write_results || !output) return;

  // 获取类型名和字段名
  char const * type_name = NULL;
  gcc_ext_util::formatTypeNameWithNamespace (AD_ARGS, type, type_name);
  char const * field_name = NULL;
  if (field_decl && DECL_NAME (field_decl)) {
    field_name = IDENTIFIER_POINTER (DECL_NAME (field_decl));
  }

  fprintf (output, "\n");
  fprintf (output, "=== Field Escape Synthesis Results ===\n");
  fprintf (output, "Type: %s\n", type_name ? type_name : "<unknown>");
  fprintf (output, "Field: %s\n", field_name ? field_name : "<unknown>");
  fprintf (output, "Total write operations: %u\n", write_results->length ());
  fprintf (output, "\n");

  for (unsigned i = 0; i < write_results->length (); i++) {
    EscapeSynthesisResult * synth = (*write_results)[i];
    if (!synth) continue;

    fprintf (output, "--- Write Operation #%u ---\n", i);
    fprintf (output, "  Categories (bitmap: 0x%x):\n", synth->category_bitmap);

    // 打印所有激活的类别
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_NO_ESCAPE)) {
      fprintf (output, "    - NO_ESCAPE (single field access, no escape)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_ARITHMETIC_POTENTIAL)) {
      fprintf (output, "    - ARITHMETIC_POTENTIAL (arithmetic/bit operations)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_SAFE_DEBUG)) {
      fprintf (output, "    - SAFE_DEBUG (safe debug functions like printf)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_UNKNOWN_CALL)) {
      fprintf (output, "    - UNKNOWN_CALL (unknown function call)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_HEAP_ESCAPE)) {
      fprintf (output, "    - HEAP_ESCAPE (escapes to heap)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_RETURN_ESCAPE)) {
      fprintf (output, "    - RETURN_ESCAPE (escapes via return)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_PARAMETER_ESCAPE)) {
      fprintf (output, "    - PARAMETER_ESCAPE (escapes via parameter)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_GLOBAL_ESCAPE)) {
      fprintf (output, "    - GLOBAL_ESCAPE (escapes to global variable)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_FIELD_ESCAPE)) {
      fprintf (output, "    - FIELD_ESCAPE (multiple field accesses)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_VIRTUAL_CALL)) {
      fprintf (output, "    - VIRTUAL_CALL (virtual function call)\n");
    }
    if (ESC_SYNTH_HAS (*synth, ESC_SYNTH_INDIRECT_CALL)) {
      fprintf (output, "    - INDIRECT_CALL (indirect call)\n");
    }

    fprintf (output, "  Statistics:\n");
    fprintf (output, "    Total uses: %u\n", synth->total_uses);
    fprintf (output, "    Total escapes: %u\n", synth->total_escapes);
    fprintf (output, "    Category count: %u\n", synth->category_count);
    fprintf (output, "\n");
  }

  fprintf (output, "======================================\n");
}

} // namespace array_detect_ns
