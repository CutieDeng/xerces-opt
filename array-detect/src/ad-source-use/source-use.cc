// ============================================================================
// ad-source-use 模块实现
// ============================================================================
// 分析源操作数的 SSA 使用链，收集所有使用点和逃逸信息
// 数据流：source_operand -> SourceUseResult
// ============================================================================

#include "source-use.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "field-analysis.hh"

#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "ssa.h"
#include "tree-ssa.h"
#include "cgraph.h"
#include "print-tree.h"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// 前向声明：类型转换辅助函数
field_analysis::FieldEscapeKind collectAllFieldEscapes_convertEscapeKind (SourceUseEscapeKind kind);
field_analysis::FieldUseKind collectAllFieldEscapes_convertUseKind (SourceUseKind kind);

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain_classifyUseKind
// ----------------------------------------------------------------------------
// 分类 SSA 使用类型

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_classifyUseKind (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree ssa_name,
  SourceUseKind & result
) AD_FUNCTION_BEGIN {

  enum gimple_code code = gimple_code (use_stmt);

  switch (code) {
    case GIMPLE_ASSIGN: {
      // 检查SSA名称是作为右值使用
      tree rhs = gimple_assign_rhs1 (use_stmt);
      tree lhs = gimple_assign_lhs (use_stmt);

      if (rhs == ssa_name || (TREE_CODE (rhs) == SSA_NAME && rhs == ssa_name)) {
        // SSA名称作为右值使用 - 检查左值类型来确定使用方式
        enum tree_code lhs_code = TREE_CODE (lhs);

        if (lhs_code == SSA_NAME) {
          // 赋值给另一个SSA变量
          enum tree_code rhs_code = gimple_assign_rhs_code (use_stmt);
          if (rhs_code == NOP_EXPR || rhs_code == CONVERT_EXPR || rhs_code == SSA_NAME) {
            AD_RETURNO (SU_USE_ASSIGN);
          } else if (TREE_CODE_CLASS (rhs_code) == tcc_comparison) {
            AD_RETURNO (SU_USE_COMPARISON);
          } else if (TREE_CODE_CLASS (rhs_code) == tcc_binary ||
                     TREE_CODE_CLASS (rhs_code) == tcc_unary) {
            AD_RETURNO (SU_USE_ARITHMETIC);
          }
        }

        // 存储到内存位置（字段、指针解引用等）
        AD_RETURNO (SU_USE_STORE);
      }
      AD_RETURNO (SU_USE_OTHER);
    }

    case GIMPLE_CALL:
      // 检查是否作为参数
      for (unsigned i = 0; i < gimple_call_num_args (use_stmt); i++) {
        if (gimple_call_arg (use_stmt, i) == ssa_name) {
          AD_RETURNO (SU_USE_CALL_ARG);
        }
      }
      AD_RETURNO (SU_USE_OTHER);

    case GIMPLE_RETURN:
      if (gimple_return_retval (as_a<greturn*>(use_stmt)) == ssa_name) {
        AD_RETURNO (SU_USE_RETURN);
      }
      AD_RETURNO (SU_USE_OTHER);

    case GIMPLE_PHI:
      AD_RETURNO (SU_USE_PHI);

    case GIMPLE_COND:
      AD_RETURNO (SU_USE_CONDITIONAL);

    default:
      AD_RETURNO (SU_USE_OTHER);
  }

  AD_RETURNO (SU_USE_OTHER);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain_detectEscapeKind_isFunctionExternal
// ----------------------------------------------------------------------------
// 判断函数是否为外部函数

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_detectEscapeKind_isFunctionExternal (
  AD_FUNC_ARGS,
  tree function_decl,
  bool & result
) AD_FUNCTION_BEGIN {
  if (!function_decl || TREE_CODE (function_decl) != FUNCTION_DECL) {
    AD_RETURNO (true);
  }

  struct cgraph_node * node = cgraph_node::get (function_decl);
  if (!node || !node->definition) {
    AD_RETURNO (true);
  }

  AD_RETURNO (false);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain_detectEscapeKind
// ----------------------------------------------------------------------------
// 检测逃逸类型

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_detectEscapeKind (
  AD_FUNC_ARGS,
  SourceUseInfo const & use_info,
  SourceUseEscapeKind & result
) AD_FUNCTION_BEGIN {
  gimple * stmt = use_info.use_stmt;

  switch (use_info.kind) {
    case SU_USE_RETURN:
      AD_RETURNO (SU_ESCAPE_RETURN);

    case SU_USE_CALL_ARG: {
      if (!is_gimple_call (stmt)) break;

      tree fn = gimple_call_fn (stmt);
      if (!fn) {
        AD_RETURNO (SU_ESCAPE_UNKNOWN);
      }

      if (TREE_CODE (fn) == OBJ_TYPE_REF) {
        AD_RETURNO (SU_ESCAPE_VIRTUAL_CALL);
      }

      if (TREE_CODE (fn) != ADDR_EXPR) {
        AD_RETURNO (SU_ESCAPE_INDIRECT_CALL);
      }

      tree fn_decl = TREE_OPERAND (fn, 0);
      bool is_external = false;
      AD_TRY (analyzeSourceUse_traceSSAUseChain_detectEscapeKind_isFunctionExternal (AD_ARGS, fn_decl, is_external));

      if (is_external) {
        AD_RETURNO (SU_ESCAPE_EXTERNAL_CALL);
      } else {
        AD_RETURNO (SU_ESCAPE_PARAMETER);
      }
    }

    case SU_USE_STORE: {
      if (!is_gimple_assign (stmt)) break;

      tree lhs = gimple_assign_lhs (stmt);
      if (!lhs) break;

      if (TREE_CODE (lhs) == VAR_DECL && is_global_var (lhs)) {
        AD_RETURNO (SU_ESCAPE_GLOBAL_STORE);
      }

      if (TREE_CODE (lhs) == MEM_REF || TREE_CODE (lhs) == COMPONENT_REF) {
        if (TREE_CODE (lhs) == COMPONENT_REF) {
          AD_RETURNO (SU_ESCAPE_FIELD_STORE);
        } else {
          AD_RETURNO (SU_ESCAPE_HEAP_STORE);
        }
      }
      break;
    }

    default:
      break;
  }

  AD_RETURNO (SU_ESCAPE_NONE);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain_recordUsePoint
// ----------------------------------------------------------------------------
// 记录使用点 - 直接写入 FieldUsePoint

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_recordUsePoint (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree use_operand,
  SourceUseKind use_kind,
  SourceUseEscapeKind escape_kind,
  char const * escape_target,
  vec<field_analysis::FieldUsePoint>* all_uses,
  unsigned int* total_use_count
) AD_FUNCTION_BEGIN {
  if (!all_uses || !total_use_count) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  field_analysis::FieldUsePoint use_point;
  use_point.kind = collectAllFieldEscapes_convertUseKind (use_kind);
  use_point.stmt = use_stmt;
  use_point.operand = use_operand;
  use_point.location = gimple_location (use_stmt);
  use_point.bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;
  use_point.escape_kind = collectAllFieldEscapes_convertEscapeKind (escape_kind);
  use_point.escape_target = escape_target;

  all_uses->safe_push (use_point);
  (*total_use_count)++;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain
// ----------------------------------------------------------------------------
// 递归追踪 SSA 使用链 - 直接写入 wrapper 成员地址

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain (
  AD_FUNC_ARGS,
  tree ssa_name,
  vec<field_analysis::FieldUsePoint>* all_uses,
  unsigned int* total_use_count,
  bool* is_fully_analyzed,
  unsigned int depth,
  gimple * exclude_stmt
) AD_FUNCTION_BEGIN {
  if (depth >= MAX_ESCAPE_ANALYSIS_DEPTH) {
    *is_fully_analyzed = false;
    AD_RETURNE (OK);
  }

  if (!ssa_name || TREE_CODE (ssa_name) != SSA_NAME) {
    AD_RETURNE (OK);
  }

  ssa_use_operand_t * head = &(SSA_NAME_IMM_USE_NODE (ssa_name));
  ssa_use_operand_t * ptr = head->next;

  while (ptr != head) {
    gimple * use_stmt = USE_STMT (ptr);

    if (use_stmt) {
      // 排除当前写入操作本身
      if (exclude_stmt && use_stmt == exclude_stmt) {
        ptr = ptr->next;
        continue;
      }

      // 分类使用类型
      SourceUseKind use_kind = SU_USE_OTHER;
      AD_TRY (analyzeSourceUse_traceSSAUseChain_classifyUseKind (AD_ARGS, use_stmt, ssa_name, use_kind));

      // 创建临时 use_info 用于逃逸检测
      SourceUseInfo temp_info;
      temp_info.kind = use_kind;
      temp_info.use_stmt = use_stmt;
      temp_info.use_operand = ssa_name;
      temp_info.source_location = gimple_location (use_stmt);
      temp_info.bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;
      temp_info.escape_kind = SU_ESCAPE_NONE;
      temp_info.escape_target = NULL;
      temp_info.target_info.function_decl = NULL;

      // 检测逃逸类型
      SourceUseEscapeKind escape_kind = SU_ESCAPE_NONE;
      AD_TRY (analyzeSourceUse_traceSSAUseChain_detectEscapeKind (AD_ARGS, temp_info, escape_kind));

      // 确定逃逸目标
      char const * escape_target = NULL;

      if (escape_kind != SU_ESCAPE_NONE) {
        if (is_gimple_call (use_stmt)) {
          tree fn = gimple_call_fndecl (use_stmt);
          if (fn && DECL_NAME (fn)) {
            escape_target = IDENTIFIER_POINTER (DECL_NAME (fn));
          } else {
            escape_target = "<indirect call>";
          }
        } else if (is_gimple_assign (use_stmt)) {
          tree lhs = gimple_assign_lhs (use_stmt);
          if (TREE_CODE (lhs) == COMPONENT_REF) {
            tree field = TREE_OPERAND (lhs, 1);
            if (DECL_NAME (field)) {
              escape_target = IDENTIFIER_POINTER (DECL_NAME (field));
            } else {
              escape_target = "<anonymous field>";
            }
          } else {
            escape_target = "<memory>";
          }
        } else {
          escape_target = "<unknown>";
        }
      }

      // 记录使用点
      AD_TRY (analyzeSourceUse_traceSSAUseChain_recordUsePoint (
        AD_ARGS, use_stmt, ssa_name, use_kind, escape_kind,
        escape_target, all_uses, total_use_count
      ));

      // 如果是简单赋值，继续追踪结果 SSA
      if (use_kind == SU_USE_ASSIGN && is_gimple_assign (use_stmt)) {
        tree lhs = gimple_assign_lhs (use_stmt);
        if (lhs && TREE_CODE (lhs) == SSA_NAME) {
          AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, lhs, all_uses, total_use_count, is_fully_analyzed, depth + 1, exclude_stmt));
        }
      }
    }

    ptr = ptr->next;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldEscapes_convertEscapeKind
// ----------------------------------------------------------------------------
// 类型转换辅助函数

field_analysis::FieldEscapeKind collectAllFieldEscapes_convertEscapeKind (SourceUseEscapeKind kind) {
  switch (kind) {
    case SU_ESCAPE_NONE:          return field_analysis::FIELD_ESC_NONE;
    case SU_ESCAPE_RETURN:        return field_analysis::FIELD_ESC_RETURN;
    case SU_ESCAPE_PARAMETER:     return field_analysis::FIELD_ESC_PARAMETER;
    case SU_ESCAPE_GLOBAL_STORE:  return field_analysis::FIELD_ESC_GLOBAL;
    case SU_ESCAPE_HEAP_STORE:    return field_analysis::FIELD_ESC_HEAP;
    case SU_ESCAPE_FIELD_STORE:   return field_analysis::FIELD_ESC_FIELD;
    case SU_ESCAPE_INDIRECT_CALL: return field_analysis::FIELD_ESC_INDIRECT_CALL;
    case SU_ESCAPE_VIRTUAL_CALL:  return field_analysis::FIELD_ESC_VIRTUAL_CALL;
    case SU_ESCAPE_EXTERNAL_CALL: return field_analysis::FIELD_ESC_EXTERNAL_CALL;
    case SU_ESCAPE_UNKNOWN:       return field_analysis::FIELD_ESC_UNKNOWN;
    default:                      return field_analysis::FIELD_ESC_UNKNOWN;
  }
}

// ----------------------------------------------------------------------------
// collectAllFieldEscapes_convertUseKind
// ----------------------------------------------------------------------------

field_analysis::FieldUseKind collectAllFieldEscapes_convertUseKind (SourceUseKind kind) {
  switch (kind) {
    case SU_USE_LOAD:         return field_analysis::FIELD_USE_LOAD;
    case SU_USE_STORE:        return field_analysis::FIELD_USE_STORE;
    case SU_USE_CALL_ARG:     return field_analysis::FIELD_USE_CALL_ARG;
    case SU_USE_RETURN:       return field_analysis::FIELD_USE_RETURN;
    case SU_USE_PHI:          return field_analysis::FIELD_USE_PHI;
    case SU_USE_ASSIGN:       return field_analysis::FIELD_USE_ASSIGN;
    case SU_USE_ARITHMETIC:   return field_analysis::FIELD_USE_ARITHMETIC;
    case SU_USE_COMPARISON:   return field_analysis::FIELD_USE_COMPARISON;
    case SU_USE_ADDRESS_TAKEN: return field_analysis::FIELD_USE_ADDRESS;
    case SU_USE_CONDITIONAL:  return field_analysis::FIELD_USE_CONDITIONAL;
    case SU_USE_OTHER:        return field_analysis::FIELD_USE_OTHER;
    default:                  return field_analysis::FIELD_USE_OTHER;
  }
}

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// getEscapeKindString
// ----------------------------------------------------------------------------

char const * getEscapeKindString (SourceUseEscapeKind kind) {
  switch (kind) {
    case SU_ESCAPE_NONE:          return "NONE";
    case SU_ESCAPE_RETURN:        return "RETURN";
    case SU_ESCAPE_PARAMETER:     return "PARAMETER";
    case SU_ESCAPE_GLOBAL_STORE:  return "GLOBAL_STORE";
    case SU_ESCAPE_HEAP_STORE:    return "HEAP_STORE";
    case SU_ESCAPE_FIELD_STORE:   return "FIELD_STORE";
    case SU_ESCAPE_INDIRECT_CALL: return "INDIRECT_CALL";
    case SU_ESCAPE_VIRTUAL_CALL:  return "VIRTUAL_CALL";
    case SU_ESCAPE_EXTERNAL_CALL: return "EXTERNAL_CALL";
    case SU_ESCAPE_UNKNOWN:       return "UNKNOWN";
    default:                      return "<invalid>";
  }
}

// ----------------------------------------------------------------------------
// getUseKindString
// ----------------------------------------------------------------------------

char const * getUseKindString (SourceUseKind kind) {
  switch (kind) {
    case SU_USE_LOAD:         return "LOAD";
    case SU_USE_STORE:        return "STORE";
    case SU_USE_CALL_ARG:     return "CALL_ARG";
    case SU_USE_RETURN:       return "RETURN";
    case SU_USE_PHI:          return "PHI";
    case SU_USE_ASSIGN:       return "ASSIGN";
    case SU_USE_ARITHMETIC:   return "ARITHMETIC";
    case SU_USE_COMPARISON:   return "COMPARISON";
    case SU_USE_ADDRESS_TAKEN: return "ADDRESS_TAKEN";
    case SU_USE_CONDITIONAL:  return "CONDITIONAL";
    case SU_USE_OTHER:        return "OTHER";
    default:                  return "<invalid>";
  }
}

// ----------------------------------------------------------------------------
// analyzeSourceUse
// ----------------------------------------------------------------------------
// 主入口：分析源操作数的所有使用
// 输出：直接写入各个 out_ 参数指向的地址

ArrayDetectErrorCode analyzeSourceUse (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  gimple * exclude_stmt,
  // 直接写入 wrapper 成员
  tree* out_source_operand,
  gimple** out_source_stmt,
  vec<field_analysis::FieldUsePoint>** out_all_uses,
  unsigned int* out_total_use_count,
  unsigned int* out_max_use_depth,
  bool* out_is_fully_analyzed
) AD_FUNCTION_BEGIN {
  if (!out_source_operand || !out_source_stmt || !out_all_uses ||
      !out_total_use_count || !out_max_use_depth || !out_is_fully_analyzed) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化输出
  *out_source_operand = source_operand;
  *out_source_stmt = source_stmt;
  *out_total_use_count = 0;
  *out_max_use_depth = 0;
  *out_is_fully_analyzed = true;

  // 分配 all_uses 向量
  *out_all_uses = ggc_alloc<vec<field_analysis::FieldUsePoint>> ();
  if (!*out_all_uses) {
    AD_RETURNE (MEMORY_ERROR);
  }
  (*out_all_uses)->create (0);

  // 追踪 SSA 使用链
  if (source_operand && TREE_CODE (source_operand) == SSA_NAME) {
    AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, source_operand,
      *out_all_uses, out_total_use_count, out_is_fully_analyzed, 0, exclude_stmt));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldEscapes
// ----------------------------------------------------------------------------
// Pipeline 接口：收集所有字段的逃逸信息
// 直接写入 wrapper 成员地址

ArrayDetectErrorCode collectAllFieldEscapes (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_escaped
) AD_FUNCTION_BEGIN {
  total_analyzed = 0;
  total_escaped = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldAnalysisData * write_ops = (*iter).second;
    if (!write_ops || !write_ops->writes) continue;

    for (unsigned i = 0; i < write_ops->writes->length (); i++) {
      Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude * wrapper = (*write_ops->writes)[i];
      if (!wrapper) continue;

      // 从 wrapper 的 FieldWrite 部分提取数据进行分析
      tree source_operand = wrapper->rhs;
      gimple * source_stmt = wrapper->stmt;
      gimple * exclude_stmt = wrapper->stmt;

      // 直接写入 wrapper 成员地址
      AD_TRY (analyzeSourceUse (AD_ARGS, source_operand, source_stmt, exclude_stmt,
        &wrapper->source_operand,
        &wrapper->source_stmt,
        &wrapper->all_uses,
        &wrapper->total_use_count,
        &wrapper->max_use_depth,
        &wrapper->is_fully_analyzed
      ));

      total_analyzed++;

      // 统计逃逸
      if (wrapper->all_uses) {
        for (unsigned j = 0; j < wrapper->all_uses->length (); j++) {
          if ((*wrapper->all_uses)[j].is_escape ()) {
            wrapper->has_escape = true;
            wrapper->escape_count++;
          }
        }
      }

      if (wrapper->has_escape) {
        total_escaped++;
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// printSourceUseResult
// ----------------------------------------------------------------------------
// 调试输出

void printSourceUseResult (
  SourceUseResult const * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf (output, "=== Source Use Analysis Result ===\n");
  fprintf (output, "Source operand: ");
  if (result->source_operand) {
    print_generic_expr (output, result->source_operand, TDF_SLIM);
  } else {
    fprintf (output, "<null>");
  }
  fprintf (output, "\n");

  fprintf (output, "Total uses: %u\n", result->total_use_count);
  fprintf (output, "Escape count: %u\n", result->escape_count);
  fprintf (output, "Has escape: %s\n", result->has_escape ? "YES" : "NO");

  fprintf (output, "\n--- All Uses (Detailed) ---\n");
  if (result->all_uses) {
    for (unsigned i = 0; i < result->all_uses->length (); i++) {
      SourceUseInfo const & use = (*result->all_uses)[i];
      fprintf (output, "[%u] Kind: %s, Escape: %s",
              i, getUseKindString (use.kind),
              use.is_escape () ? getEscapeKindString (use.escape_kind) : "NONE");

      if (use.source_location != UNKNOWN_LOCATION) {
        expanded_location xloc = expand_location (use.source_location);
        fprintf (output, ", Location: %s:%d:%d",
                xloc.file, xloc.line, xloc.column);
      }
      fprintf (output, "\n");

      // 如果是逃逸，打印逃逸目标详情
      if (use.is_escape ()) {
        fprintf (output, "    Target: %s\n",
                use.escape_target ? use.escape_target : "<unknown>");
        fprintf (output, "    BB Index: %u\n", use.bb_index);
      }
    }
  }

  fprintf (output, "\nFully analyzed: %s\n",
          result->is_fully_analyzed ? "YES" : "NO (depth limit reached)");
  fprintf (output, "===================================\n");
}

} // namespace array_detect_ns
