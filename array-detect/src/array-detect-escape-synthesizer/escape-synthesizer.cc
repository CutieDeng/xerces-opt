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
// 辅助函数：分类单个逃逸位置
// ============================================================================

ArrayDetectErrorCode classifyEscapeLocation (
  AD_FUNC_ARGS,
  SourceUseEscapeLocation const &escape_loc,
  EscapeSynthesisResult * result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("[classifyEscapeLocation] Classifying escape kind: %s",
                  getEscapeKindString (escape_loc.kind));

  switch (escape_loc.kind) {
    case SU_ESCAPE_NONE:
      // 这不应该出现在逃逸位置中
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
        vec_safe_push (result->return_escape->escape_stmts, escape_loc.stmt);
        vec_safe_push (result->return_escape->targets,
                      escape_loc.escape_target ? escape_loc.escape_target : "<return>");
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

        vec_safe_push (result->parameter_escape->escape_stmts, escape_loc.stmt);
        vec_safe_push (result->parameter_escape->targets,
                      escape_loc.escape_target ? escape_loc.escape_target : "<parameter>");
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

        vec_safe_push (result->global_escape->escape_stmts, escape_loc.stmt);
        vec_safe_push (result->global_escape->targets,
                      escape_loc.escape_target ? escape_loc.escape_target : "<global>");
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

        vec_safe_push (result->heap_escape->heap_stores, escape_loc.stmt);
        vec_safe_push (result->heap_escape->stored_values, escape_loc.use_operand);
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

        vec_safe_push (result->field_escape->field_stores, escape_loc.stmt);
        vec_safe_push (result->field_escape->field_decls, escape_loc.target_info.field_decl);

        // 提取字段名
        char const * field_name = escape_loc.escape_target;
        if (!field_name && escape_loc.target_info.field_decl &&
            DECL_NAME (escape_loc.target_info.field_decl)) {
          field_name = IDENTIFIER_POINTER (DECL_NAME (escape_loc.target_info.field_decl));
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
        char const * func_name = escape_loc.escape_target;

        if (func_name && isKnownSafeDebugFunction (AD_ARGS, func_name)) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_SAFE_DEBUG);
          if (!result->safe_debug) {
            result->safe_debug = ggc_alloc<SafeDebugEscape> ();
            memset (result->safe_debug, 0, sizeof (SafeDebugEscape));
            result->safe_debug->debug_calls = NULL;
            result->safe_debug->function_names = NULL;
            result->safe_debug->count = 0;
          }

          vec_safe_push (result->safe_debug->debug_calls, escape_loc.stmt);
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

          vec_safe_push (result->unknown_call->unknown_calls, escape_loc.stmt);
          vec_safe_push (result->unknown_call->call_descriptions,
                        ggc_strdup (func_name ? func_name : "<unknown-call>"));
          result->unknown_call->count++;
        }

        // 同时记录虚函数或间接调用
        if (escape_loc.kind == SU_ESCAPE_VIRTUAL_CALL) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_VIRTUAL_CALL);
          if (!result->virtual_call) {
            result->virtual_call = ggc_alloc<CallEscapeInfo> ();
            memset (result->virtual_call, 0, sizeof (CallEscapeInfo));
            result->virtual_call->call_stmts = NULL;
            result->virtual_call->call_names = NULL;
            result->virtual_call->count = 0;
          }

          vec_safe_push (result->virtual_call->call_stmts, escape_loc.stmt);
          vec_safe_push (result->virtual_call->call_names,
                        ggc_strdup (func_name ? func_name : "<virtual-call>"));
          result->virtual_call->count++;
        } else if (escape_loc.kind == SU_ESCAPE_INDIRECT_CALL) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_INDIRECT_CALL);
          if (!result->indirect_call) {
            result->indirect_call = ggc_alloc<CallEscapeInfo> ();
            memset (result->indirect_call, 0, sizeof (CallEscapeInfo));
            result->indirect_call->call_stmts = NULL;
            result->indirect_call->call_names = NULL;
            result->indirect_call->count = 0;
          }

          vec_safe_push (result->indirect_call->call_stmts, escape_loc.stmt);
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
        if (escape_loc.stmt && isArithmeticOperation (escape_loc.stmt, operation_name)) {
          ESC_SYNTH_ADD (*result, ESC_SYNTH_ARITHMETIC_POTENTIAL);
          if (!result->arithmetic_potential) {
            result->arithmetic_potential = ggc_alloc<ArithmeticPotentialEscape> ();
            memset (result->arithmetic_potential, 0, sizeof (ArithmeticPotentialEscape));
            result->arithmetic_potential->arithmetic_stmts = NULL;
            result->arithmetic_potential->operations = NULL;
            result->arithmetic_potential->count = 0;
          }

          vec_safe_push (result->arithmetic_potential->arithmetic_stmts, escape_loc.stmt);
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

          vec_safe_push (result->unknown_call->unknown_calls, escape_loc.stmt);
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

  // 遍历所有逃逸位置，进行分类
  if (raw_result->escape_locations) {
    unsigned int escape_count = raw_result->escape_locations->length ();
    AD_DEBUG_PRINT ("[synthesizeEscapeInfo] Processing %u escape locations", escape_count);

    for (unsigned int i = 0; i < escape_count; i++) {
      SourceUseEscapeLocation const &escape_loc = (*raw_result->escape_locations)[i];
      AD_TRY (classifyEscapeLocation (AD_ARGS, escape_loc, synth_result));
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

    if (!write_ops || !write_ops->write_ops) continue;

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
                    write_ops->write_ops->length ());

    // 遍历该 (type, field) 的所有写入操作
    for (unsigned i = 0; i < write_ops->write_ops->length (); i++) {
      FieldWriteCapture * capture = (*write_ops->write_ops)[i];
      if (!capture) continue;

      // 从 capture->aux 读取逃逸分析结果
      // 注意：aux 现在存储的是 SourceUseAnalysisResult*
      SourceUseAnalysisResult * raw_result = (SourceUseAnalysisResult *) capture->aux;
      if (!raw_result) {
        AD_DEBUG_PRINT ("    Write #%u: No escape analysis result", i);
        continue;
      }

      // 综合该写入操作的逃逸信息
      EscapeSynthesisResult * synth_result = NULL;
      AD_TRY (synthesizeEscapeInfo (AD_ARGS, raw_result, synth_result));

      if (synth_result) {
        // 记录原始写入信息（FieldWriteCapture）
        synth_result->original_write_info = capture;

        // 添加到结果列表
        synthesis_results->safe_push (synth_result);
        total_synthesized++;

        AD_DEBUG_PRINT ("    Write #%u: Synthesized - categories=0x%x, uses=%u, escapes=%u",
                        i, synth_result->category_bitmap,
                        synth_result->total_uses, synth_result->total_escapes);
      }
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

} // namespace array_detect_ns
