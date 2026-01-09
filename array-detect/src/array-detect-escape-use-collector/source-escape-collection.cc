#include "prelude.hh"
#include "state.hh"
#include "source-escape-collection.hh"
#include "array-detector.hh"
#include "write-operation-trace.hh"
#include "info-print.hh"

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
// 源逃逸收集模块 (Source Escape Collection)
// ============================================================================
// 收集源操作数的所有逃逸信息，包括：
// - SSA 使用链跟踪
// - 逃逸类型检测
// - 逃逸位置详细记录
// 所有详尽信息供后续逃逸综合器模块使用
// ============================================================================


// ============================================================================
// 辅助函数实现
// ============================================================================

char const * getEscapeKindString(SourceUseEscapeKind kind) {
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

char const * getUseKindString(SourceUseKind kind) {
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

// ============================================================================
// 核心分析逻辑
// ============================================================================

// 分类 SSA 使用类型
ArrayDetectErrorCode classifySSAUseKind (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree ssa_name,
  SourceUseKind &result
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

// 判断函数是否为外部函数
ArrayDetectErrorCode isFunctionExternal (
  AD_FUNC_ARGS,
  tree function_decl,
  bool &result
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

// 检测逃逸类型（全量逃逸检测）
ArrayDetectErrorCode detectEscapeKind (
  AD_FUNC_ARGS,
  const SourceUseInfo &use_info,
  SourceUseEscapeKind &result
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
      AD_TRY (isFunctionExternal (AD_ARGS, fn_decl, is_external));

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

// NOTE: isEscapeUse is not used and does not follow standard API
// Commented out - should be moved to submodule or removed
/*
bool isEscapeUse(
  const SourceUseInfo& use_info,
  const SourceUseEscapeRules& rules
) {
  return analyzeEscapeKind(use_info, rules) != SU_ESCAPE_NONE;
}
*/

// ============================================================================
// 使用分析主函数
// ============================================================================

// 递归跟踪 SSA 使用链逃逸（手动遍历immediate uses）
// exclude_stmt: 要排除的语句（即当前正在分析的写入操作本身，不应视为逃逸）
ArrayDetectErrorCode traceSSAUseChainEscapes (
  AD_FUNC_ARGS,
  tree ssa_name,
  SourceUseAnalysisResult * result,
  unsigned int depth,
  gimple * exclude_stmt
) AD_FUNCTION_BEGIN {
  if (depth >= MAX_ESCAPE_ANALYSIS_DEPTH) {
    result->is_fully_analyzed = false;
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
      if (exclude_stmt && use_stmt == exclude_stmt) {
        ptr = ptr->next;
        continue;
      }

      SourceUseInfo use_info;
      SourceUseKind use_kind = SU_USE_OTHER;
      AD_TRY (classifySSAUseKind (AD_ARGS, use_stmt, ssa_name, use_kind));

      use_info.kind = use_kind;
      use_info.use_stmt = use_stmt;
      use_info.use_operand = ssa_name;
      use_info.source_location = gimple_location (use_stmt);
      use_info.bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;

      SourceUseEscapeKind escape_kind = SU_ESCAPE_NONE;
      AD_TRY (detectEscapeKind (AD_ARGS, use_info, escape_kind));

      use_info.escape_kind = escape_kind;
      use_info.escape_target = NULL;
      use_info.target_info.function_decl = NULL;

      if (use_info.is_escape()) {
        result->has_escape = true;
        result->escape_count++;

        if (is_gimple_call (use_stmt)) {
          tree fn = gimple_call_fndecl (use_stmt);
          if (fn && DECL_NAME (fn)) {
            use_info.escape_target = IDENTIFIER_POINTER (DECL_NAME (fn));
            use_info.target_info.function_decl = fn;
          } else {
            use_info.escape_target = "<indirect call>";
            use_info.target_info.function_decl = NULL;
          }
        } else if (is_gimple_assign (use_stmt)) {
          tree lhs = gimple_assign_lhs (use_stmt);
          if (TREE_CODE (lhs) == COMPONENT_REF) {
            tree field = TREE_OPERAND (lhs, 1);
            if (DECL_NAME (field)) {
              use_info.escape_target = IDENTIFIER_POINTER (DECL_NAME (field));
              use_info.target_info.field_decl = field;
            } else {
              use_info.escape_target = "<anonymous field>";
              use_info.target_info.field_decl = field;
            }
          } else {
            use_info.escape_target = "<memory>";
            use_info.target_info.global_var = NULL;
          }
        } else {
          use_info.escape_target = "<unknown>";
          use_info.target_info.function_decl = NULL;
        }
      }

      result->all_uses->safe_push (use_info);
      result->total_use_count++;

      if (use_kind == SU_USE_ASSIGN && is_gimple_assign (use_stmt)) {
        tree lhs = gimple_assign_lhs (use_stmt);
        if (lhs && TREE_CODE (lhs) == SSA_NAME) {
          AD_TRY (traceSSAUseChainEscapes (AD_ARGS, lhs, result, depth + 1, exclude_stmt));
        }
      }
    }

    ptr = ptr->next;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode collectSourceOperandEscapes (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  gimple * exclude_stmt,
  SourceUseAnalysisResult * &result
) AD_FUNCTION_BEGIN {
  SourceUseAnalysisResult * tmp_result = ggc_alloc <SourceUseAnalysisResult> ();
  memset (tmp_result, 0, sizeof (SourceUseAnalysisResult));

  tmp_result->source_operand = source_operand;
  tmp_result->source_stmt = source_stmt;
  tmp_result->all_uses = ggc_alloc <vec<SourceUseInfo>> ();
  tmp_result->all_uses->create (0);
  tmp_result->total_use_count = 0;
  tmp_result->escape_count = 0;
  tmp_result->has_escape = false;
  tmp_result->dominant_escape_kind = SU_ESCAPE_NONE;
  tmp_result->max_use_depth = 0;
  tmp_result->is_fully_analyzed = true;
  tmp_result->aux = NULL;
  tmp_result->original_write_info = NULL;

  if (source_operand && TREE_CODE (source_operand) == SSA_NAME) {
    AD_TRY (traceSSAUseChainEscapes (AD_ARGS, source_operand, tmp_result, 0, exclude_stmt));
  }

  AD_RETURNO (tmp_result);
} AD_FUNCTION_END

// ============================================================================
// 与 write-operation 集成
// ============================================================================

ArrayDetectErrorCode collectFieldWriteEscapes (
  AD_FUNC_ARGS,
  FieldWriteCapture * write_capture,
  SourceUseAnalysisResult * &result
) AD_FUNCTION_BEGIN {
  if (!write_capture) {
    AD_RETURNO (NULL);
  }

  FieldWriteCapture * write_info = write_capture;
  tree source_operand = write_info->rhs;
  gimple * source_stmt = write_info->stmt;
  gimple * exclude_stmt = write_info->stmt;

  SourceUseAnalysisResult * tmp_result = NULL;
  AD_TRY (collectSourceOperandEscapes (AD_ARGS, source_operand, source_stmt, exclude_stmt, tmp_result));

  if (tmp_result) {
    tmp_result->original_write_info = write_capture;
    void * old_aux = write_info->aux;
    tmp_result->aux = old_aux;
    write_info->aux = tmp_result;
  }

  AD_RETURNO (tmp_result);
} AD_FUNCTION_END

void freeSourceUseAnalysisResult(SourceUseAnalysisResult * result) {
  if (!result) return;

  // 释放 vec 结构（GC 管理，不需要手动释放）
  // result 本身也由 GC 管理

  // 注意：不要释放 aux 链中的数据，由调用者管理
}

// ============================================================================
// 调试输出
// ============================================================================

void printSourceUseResult(
  const SourceUseResult * result,
  FILE * output
) {
  if (!result || !output) return;

  fprintf(output, "=== Source Use Analysis Result ===\n");
  fprintf(output, "Source operand: ");
  if (result->source_operand) {
    print_generic_expr(output, result->source_operand, TDF_SLIM);
  } else {
    fprintf(output, "<null>");
  }
  fprintf(output, "\n");

  fprintf(output, "Total uses: %u\n", result->total_use_count);
  fprintf(output, "Escape count: %u\n", result->escape_count);
  fprintf(output, "Has escape: %s\n", result->has_escape ? "YES" : "NO");

  fprintf(output, "\n--- All Uses (Detailed) ---\n");
  if (result->all_uses) {
    for (unsigned i = 0; i < result->all_uses->length(); i++) {
      const SourceUseInfo& use = (*result->all_uses)[i];
      fprintf(output, "[%u] Kind: %s, Escape: %s",
              i, getUseKindString(use.kind),
              use.is_escape() ? getEscapeKindString(use.escape_kind) : "NONE");

      if (use.source_location != UNKNOWN_LOCATION) {
        expanded_location xloc = expand_location(use.source_location);
        fprintf(output, ", Location: %s:%d:%d",
                xloc.file, xloc.line, xloc.column);
      }
      fprintf(output, "\n");

      // 如果是逃逸，打印逃逸目标详情
      if (use.is_escape()) {
        fprintf(output, "    Target: %s\n",
                use.escape_target ? use.escape_target : "<unknown>");
        fprintf(output, "    BB Index: %u\n", use.bb_index);
      }
    }
  }

  fprintf (output, "\nFully analyzed: %s\n",
          result->is_fully_analyzed ? "YES" : "NO (depth limit reached)");
  fprintf (output, "===================================\n");
}

// ============================================================================
// 辅助函数：将 SourceUseEscapeKind 转换为 FieldEscapeKind
// ============================================================================

static field_analysis::FieldEscapeKind convertEscapeKind (SourceUseEscapeKind kind) {
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

// 辅助函数：将 SourceUseKind 转换为 FieldUseKind
static field_analysis::FieldUseKind convertUseKind (SourceUseKind kind) {
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

// 辅助函数：将 SourceUseResult 数据复制到 Wrapper 的 UseAnalysis 部分
static void copyUseResultToWrapper (
  FieldWriteAnalysisWrapper *wrapper,
  SourceUseAnalysisResult const *use_result
) {
  if (!wrapper || !use_result) return;

  wrapper->source_operand = use_result->source_operand;
  wrapper->source_stmt = use_result->source_stmt;
  wrapper->total_use_count = use_result->total_use_count;
  wrapper->max_use_depth = use_result->max_use_depth;
  wrapper->is_fully_analyzed = use_result->is_fully_analyzed;
  wrapper->escape_count = use_result->escape_count;
  wrapper->has_escape = use_result->has_escape;

  // 转换并复制 all_uses
  if (use_result->all_uses && use_result->all_uses->length () > 0) {
    wrapper->all_uses = ggc_alloc<vec<field_analysis::FieldUsePoint>> ();
    wrapper->all_uses->create (use_result->all_uses->length ());

    // 同时创建 escape_uses 列表
    wrapper->escape_uses = ggc_alloc<vec<field_analysis::FieldUsePoint const*>> ();
    wrapper->escape_uses->create (0);

    for (unsigned i = 0; i < use_result->all_uses->length (); i++) {
      SourceUseInfo const &src = (*use_result->all_uses)[i];
      field_analysis::FieldUsePoint fp;
      fp.kind = convertUseKind (src.kind);
      fp.stmt = src.use_stmt;
      fp.operand = src.use_operand;
      fp.location = src.source_location;
      fp.bb_index = src.bb_index;
      fp.escape_kind = convertEscapeKind (src.escape_kind);
      fp.escape_target = src.escape_target;
      wrapper->all_uses->safe_push (fp);

      // 如果是逃逸使用，添加到 escape_uses
      if (src.is_escape ()) {
        wrapper->escape_uses->safe_push (&(*wrapper->all_uses)[wrapper->all_uses->length () - 1]);
      }
    }
  }
}

// ============================================================================
// Pipeline 接口实现
// ============================================================================

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

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps * write_ops = (*iter).second;
    if (!write_ops || !write_ops->writes) continue;

    for (unsigned i = 0; i < write_ops->writes->length (); i++) {
      FieldWriteAnalysisWrapper * wrapper = (*write_ops->writes)[i];
      if (!wrapper) continue;

      // 从 wrapper 的 FieldWrite 部分提取数据进行分析
      tree source_operand = wrapper->rhs;
      gimple * source_stmt = wrapper->stmt;
      gimple * exclude_stmt = wrapper->stmt;

      SourceUseAnalysisResult * use_result = NULL;
      AD_TRY (collectSourceOperandEscapes (AD_ARGS, source_operand, source_stmt, exclude_stmt, use_result));

      if (use_result) {
        total_analyzed++;
        if (use_result->has_escape) {
          total_escaped++;
        }

        // 将 use_result 数据复制到 wrapper 的 FieldUseAnalysis 部分
        copyUseResultToWrapper (wrapper, use_result);

        // 输出所有收集到的 escape 信息
        printSourceUseAnalysisResult (use_result, ctx.debug_file);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
