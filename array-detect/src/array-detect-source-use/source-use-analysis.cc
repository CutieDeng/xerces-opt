#include "source-use-analysis.hh"
#include "write-operation-trace.hh"
#include "info-print.hh"
#include "context.hh"
#include "analysis-data.hh"

#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "ssa.h"
#include "tree-ssa.h"
#include "cgraph.h"

namespace array_detect_ns {

// ============================================================================
// 默认配置
// ============================================================================

SourceUseEscapeRules getDefaultEscapeRules() {
  SourceUseEscapeRules rules;

  // 函数调用
  rules.external_call_is_escape = true;
  rules.virtual_call_is_escape = true;
  rules.indirect_call_is_escape = true;

  // 存储
  rules.global_store_is_escape = true;
  rules.heap_store_is_escape = true;
  rules.field_store_is_escape = false;  // 字段存储通常不算逃逸

  // 返回和参数
  rules.return_is_escape = true;
  rules.param_to_external_is_escape = true;
  rules.param_to_internal_is_escape = false;

  // 分析控制
  rules.max_analysis_depth = 5;
  rules.stop_at_first_escape = false;

  return rules;
}

// ============================================================================
// 辅助函数实现
// ============================================================================

const char* getEscapeKindString(SourceUseEscapeKind kind) {
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

const char* getUseKindString(SourceUseKind kind) {
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

// 分析单个 SSA 使用
static SourceUseKind classifyUseKind(gimple* use_stmt, tree ssa_name) {
  enum gimple_code code = gimple_code(use_stmt);

  switch (code) {
    case GIMPLE_ASSIGN: {
      // 检查是左值还是右值
      tree lhs = gimple_assign_lhs(use_stmt);
      tree rhs = gimple_assign_rhs1(use_stmt);

      if (rhs == ssa_name || (TREE_CODE(rhs) == SSA_NAME && rhs == ssa_name)) {
        // 作为右值使用
        enum tree_code rhs_code = gimple_assign_rhs_code(use_stmt);
        if (rhs_code == NOP_EXPR || rhs_code == CONVERT_EXPR || rhs_code == SSA_NAME) {
          return SU_USE_ASSIGN;
        } else if (TREE_CODE_CLASS(rhs_code) == tcc_comparison) {
          return SU_USE_COMPARISON;
        } else if (TREE_CODE_CLASS(rhs_code) == tcc_binary ||
                   TREE_CODE_CLASS(rhs_code) == tcc_unary) {
          return SU_USE_ARITHMETIC;
        }
        return SU_USE_STORE;
      }
      return SU_USE_OTHER;
    }

    case GIMPLE_CALL:
      // 检查是否作为参数
      for (unsigned i = 0; i < gimple_call_num_args(use_stmt); i++) {
        if (gimple_call_arg(use_stmt, i) == ssa_name) {
          return SU_USE_CALL_ARG;
        }
      }
      return SU_USE_OTHER;

    case GIMPLE_RETURN:
      if (gimple_return_retval(as_a<greturn*>(use_stmt)) == ssa_name) {
        return SU_USE_RETURN;
      }
      return SU_USE_OTHER;

    case GIMPLE_PHI:
      return SU_USE_PHI;

    case GIMPLE_COND:
      return SU_USE_CONDITIONAL;

    default:
      return SU_USE_OTHER;
  }

  return SU_USE_OTHER;
}

// 判断函数是否为外部函数
static bool isFunctionExternal(tree function_decl) {
  if (!function_decl || TREE_CODE(function_decl) != FUNCTION_DECL) {
    return true;
  }

  // 检查是否有函数体
  struct cgraph_node* node = cgraph_node::get(function_decl);
  if (!node || !node->definition) {
    return true;  // 外部函数或声明
  }

  return false;
}

// 分析使用是否逃逸
static SourceUseEscapeKind analyzeEscapeKind(
  const SourceUseInfo& use_info,
  const SourceUseEscapeRules& rules
) {
  gimple* stmt = use_info.use_stmt;

  switch (use_info.kind) {
    case SU_USE_RETURN:
      return rules.return_is_escape ? SU_ESCAPE_RETURN : SU_ESCAPE_NONE;

    case SU_USE_CALL_ARG: {
      // 检查被调用的函数
      if (!is_gimple_call(stmt)) break;

      tree fn = gimple_call_fn(stmt);
      if (!fn) return SU_ESCAPE_UNKNOWN;

      // 虚函数调用
      if (TREE_CODE(fn) == OBJ_TYPE_REF) {
        return rules.virtual_call_is_escape ? SU_ESCAPE_VIRTUAL_CALL : SU_ESCAPE_NONE;
      }

      // 间接调用
      if (TREE_CODE(fn) != ADDR_EXPR) {
        return rules.indirect_call_is_escape ? SU_ESCAPE_INDIRECT_CALL : SU_ESCAPE_NONE;
      }

      // 直接调用
      tree fn_decl = TREE_OPERAND(fn, 0);
      if (isFunctionExternal(fn_decl)) {
        return rules.param_to_external_is_escape ? SU_ESCAPE_EXTERNAL_CALL : SU_ESCAPE_NONE;
      } else {
        return rules.param_to_internal_is_escape ? SU_ESCAPE_PARAMETER : SU_ESCAPE_NONE;
      }
    }

    case SU_USE_STORE: {
      // 分析存储目标
      if (!is_gimple_assign(stmt)) break;

      tree lhs = gimple_assign_lhs(stmt);
      if (!lhs) break;

      // 全局变量
      if (TREE_CODE(lhs) == VAR_DECL && is_global_var(lhs)) {
        return rules.global_store_is_escape ? SU_ESCAPE_GLOBAL_STORE : SU_ESCAPE_NONE;
      }

      // 间接存储（可能是堆或字段）
      if (TREE_CODE(lhs) == MEM_REF || TREE_CODE(lhs) == COMPONENT_REF) {
        if (TREE_CODE(lhs) == COMPONENT_REF) {
          return rules.field_store_is_escape ? SU_ESCAPE_FIELD_STORE : SU_ESCAPE_NONE;
        } else {
          return rules.heap_store_is_escape ? SU_ESCAPE_HEAP_STORE : SU_ESCAPE_NONE;
        }
      }
      break;
    }

    default:
      break;
  }

  return SU_ESCAPE_NONE;
}

bool isEscapeUse(
  const SourceUseInfo& use_info,
  const SourceUseEscapeRules& rules
) {
  return analyzeEscapeKind(use_info, rules) != SU_ESCAPE_NONE;
}

// ============================================================================
// 使用分析主函数
// ============================================================================

// 递归分析 SSA 使用链
static void analyzeSSAUseChain(
  tree ssa_name,
  SourceUseAnalysisResult* result,
  const SourceUseEscapeRules& rules,
  unsigned int depth
) {
  if (depth >= rules.max_analysis_depth) {
    result->is_fully_analyzed = false;
    return;
  }

  if (!ssa_name || TREE_CODE(ssa_name) != SSA_NAME) {
    return;
  }

  // 遍历所有使用
  imm_use_iterator iter;
  gimple* use_stmt;

  FOR_EACH_IMM_USE_STMT(use_stmt, iter, ssa_name) {
    if (!use_stmt) continue;

    // 创建使用信息
    SourceUseInfo use_info;
    use_info.kind = classifyUseKind(use_stmt, ssa_name);
    use_info.use_stmt = use_stmt;
    use_info.use_operand = ssa_name;
    use_info.source_location = gimple_location(use_stmt);
    use_info.bb_index = gimple_bb(use_stmt) ? gimple_bb(use_stmt)->index : 0;

    // 分析逃逸
    use_info.escape_kind = analyzeEscapeKind(use_info, rules);
    use_info.is_escape = (use_info.escape_kind != SU_ESCAPE_NONE);

    // 添加到结果
    result->all_uses->safe_push(use_info);
    result->total_use_count++;

    // 如果是逃逸，记录逃逸位置
    if (use_info.is_escape) {
      result->has_escape = true;
      result->escape_count++;

      SourceUseEscapeLocation escape_loc;
      escape_loc.kind = use_info.escape_kind;
      escape_loc.stmt = use_stmt;
      escape_loc.use_operand = ssa_name;
      escape_loc.source_location = use_info.source_location;
      escape_loc.bb_index = use_info.bb_index;

      // 提取逃逸目标信息
      if (is_gimple_call(use_stmt)) {
        tree fn = gimple_call_fndecl(use_stmt);
        if (fn && DECL_NAME(fn)) {
          escape_loc.escape_target = IDENTIFIER_POINTER(DECL_NAME(fn));
          escape_loc.target_info.function_decl = fn;
        } else {
          escape_loc.escape_target = "<indirect call>";
          escape_loc.target_info.function_decl = NULL;
        }
      } else if (is_gimple_assign(use_stmt)) {
        tree lhs = gimple_assign_lhs(use_stmt);
        if (TREE_CODE(lhs) == COMPONENT_REF) {
          tree field = TREE_OPERAND(lhs, 1);
          if (DECL_NAME(field)) {
            escape_loc.escape_target = IDENTIFIER_POINTER(DECL_NAME(field));
            escape_loc.target_info.field_decl = field;
          } else {
            escape_loc.escape_target = "<anonymous field>";
            escape_loc.target_info.field_decl = field;
          }
        } else {
          escape_loc.escape_target = "<memory>";
          escape_loc.target_info.global_var = NULL;
        }
      } else {
        escape_loc.escape_target = "<unknown>";
        escape_loc.target_info.function_decl = NULL;
      }

      result->escape_locations->safe_push(escape_loc);

      if (rules.stop_at_first_escape) {
        return;
      }
    }

    // 如果是赋值，继续追踪新的 SSA
    if (use_info.kind == SU_USE_ASSIGN && is_gimple_assign(use_stmt)) {
      tree lhs = gimple_assign_lhs(use_stmt);
      if (lhs && TREE_CODE(lhs) == SSA_NAME) {
        analyzeSSAUseChain(lhs, result, rules, depth + 1);
      }
    }
  }
}

SourceUseAnalysisResult* analyzeSourceOperandUse(
  tree source_operand,
  gimple* source_stmt,
  const SourceUseEscapeRules& rules
) {
  // 分配结果结构
  SourceUseAnalysisResult* result = ggc_alloc<SourceUseAnalysisResult>();
  memset(result, 0, sizeof(SourceUseAnalysisResult));

  result->source_operand = source_operand;
  result->source_stmt = source_stmt;

  // 分配并初始化 vectors
  result->all_uses = ggc_alloc<vec<SourceUseInfo>>();
  result->all_uses->create(0);
  result->total_use_count = 0;

  result->escape_locations = ggc_alloc<vec<SourceUseEscapeLocation>>();
  result->escape_locations->create(0);
  result->escape_count = 0;

  result->has_escape = false;
  result->dominant_escape_kind = SU_ESCAPE_NONE;
  result->use_chain_root = NULL;
  result->max_use_depth = 0;
  result->is_fully_analyzed = true;
  result->aux = NULL;
  result->original_write_info = NULL;

  // 开始分析
  if (source_operand && TREE_CODE(source_operand) == SSA_NAME) {
    analyzeSSAUseChain(source_operand, result, rules, 0);
  }

  // 确定主要逃逸类型（选择出现次数最多的）
  if (result->has_escape && result->escape_locations) {
    int escape_counts[SU_ESCAPE_UNKNOWN + 1] = {0};
    for (unsigned i = 0; i < result->escape_locations->length(); i++) {
      SourceUseEscapeLocation& loc = (*result->escape_locations)[i];
      escape_counts[loc.kind]++;
    }

    int max_count = 0;
    for (int i = 0; i <= SU_ESCAPE_UNKNOWN; i++) {
      if (escape_counts[i] > max_count) {
        max_count = escape_counts[i];
        result->dominant_escape_kind = (SourceUseEscapeKind)i;
      }
    }
  }

  return result;
}

// ============================================================================
// 与 write-operation 集成
// ============================================================================

SourceUseAnalysisResult* analyzeFromWriteCapture(
  FieldWriteCapture* write_capture,
  const SourceUseEscapeRules& rules
) {
  if (!write_capture) {
    return NULL;
  }

  // 使用 FieldWriteCapture
  FieldWriteCapture* write_info = write_capture;

  // 提取源操作数
  tree source_operand = write_info->rhs;    // 右值表达式
  gimple* source_stmt = write_info->stmt;   // GIMPLE 语句

  // 分析
  SourceUseAnalysisResult* result = analyzeSourceOperandUse(
    source_operand,
    source_stmt,
    rules
  );

  if (result) {
    // 关联原始数据
    result->original_write_info = write_capture;

    // ========== 异构链表更新 ==========
    // 1. 从原 hash_map 的 aux 摘下旧数据
    void* old_aux = write_info->aux;

    // 2. 将新结果的 aux 指向旧数据
    result->aux = old_aux;

    // 3. 更新原数据的 aux 指向新结果
    write_info->aux = result;

    // 这样就实现了异构伪 list 的更新：
    // hash_map -> write_info -> (aux) -> source_use_result -> (aux) -> old_data
  }

  return result;
}

void freeSourceUseAnalysisResult(SourceUseAnalysisResult* result) {
  if (!result) return;

  // 释放 vec 结构（GC 管理，不需要手动释放）
  // result 本身也由 GC 管理

  // 注意：不要释放 aux 链中的数据，由调用者管理
}

// ============================================================================
// 调试输出
// ============================================================================

void printSourceUseAnalysisResult(
  const SourceUseAnalysisResult* result,
  FILE* output
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

  if (result->has_escape) {
    fprintf(output, "Dominant escape kind: %s\n",
            getEscapeKindString(result->dominant_escape_kind));
  }

  fprintf(output, "\n--- All Uses ---\n");
  if (result->all_uses) {
    for (unsigned i = 0; i < result->all_uses->length(); i++) {
      const SourceUseInfo& use = (*result->all_uses)[i];
      fprintf(output, "[%u] Kind: %s, Escape: %s",
              i, getUseKindString(use.kind),
              use.is_escape ? getEscapeKindString(use.escape_kind) : "NONE");

      if (use.source_location != UNKNOWN_LOCATION) {
        expanded_location xloc = expand_location(use.source_location);
        fprintf(output, ", Location: %s:%d:%d",
                xloc.file, xloc.line, xloc.column);
      }
      fprintf(output, "\n");
    }
  }

  fprintf(output, "\n--- Escape Locations ---\n");
  if (result->escape_locations) {
    for (unsigned i = 0; i < result->escape_locations->length(); i++) {
      const SourceUseEscapeLocation& loc = (*result->escape_locations)[i];
      fprintf(output, "[%u] %s -> %s\n",
              i, getEscapeKindString(loc.kind),
              loc.escape_target ? loc.escape_target : "<unknown>");
    }
  }

  fprintf(output, "\nFully analyzed: %s\n",
          result->is_fully_analyzed ? "YES" : "NO (depth limit reached)");
  fprintf(output, "===================================\n");
}

} // namespace array_detect_ns
