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

const char * getEscapeKindString(SourceUseEscapeKind kind) {
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

const char * getUseKindString(SourceUseKind kind) {
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
  AD_DEBUG_PRINT ("Classifying SSA use kind");

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
  AD_DEBUG_PRINT ("Checking if function is external");

  if (!function_decl || TREE_CODE (function_decl) != FUNCTION_DECL) {
    AD_RETURNO (true);
  }

  // 检查是否有函数体
  struct cgraph_node * node = cgraph_node::get (function_decl);
  if (!node || !node->definition) {
    AD_DEBUG_PRINT ("Function is external or declaration only");
    AD_RETURNO (true);  // 外部函数或声明
  }

  AD_DEBUG_PRINT ("Function is internal");
  AD_RETURNO (false);
} AD_FUNCTION_END

// 检测逃逸类型（全量逃逸检测，所有可能的逃逸均被捕获）
ArrayDetectErrorCode detectEscapeKind (
  AD_FUNC_ARGS,
  const SourceUseInfo &use_info,
  SourceUseEscapeKind &result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Detecting escape kind");

  gimple * stmt = use_info.use_stmt;

  switch (use_info.kind) {
    case SU_USE_RETURN:
      AD_DEBUG_PRINT ("Use is RETURN -> ESCAPE");
      AD_RETURNO (SU_ESCAPE_RETURN);

    case SU_USE_CALL_ARG: {
      // 检查被调用的函数
      if (!is_gimple_call (stmt)) break;

      tree fn = gimple_call_fn (stmt);
      if (!fn) {
        AD_DEBUG_PRINT ("Unknown function call -> ESCAPE_UNKNOWN");
        AD_RETURNO (SU_ESCAPE_UNKNOWN);
      }

      // 虚函数调用
      if (TREE_CODE (fn) == OBJ_TYPE_REF) {
        AD_DEBUG_PRINT ("Virtual call detected -> ESCAPE");
        AD_RETURNO (SU_ESCAPE_VIRTUAL_CALL);
      }

      // 间接调用
      if (TREE_CODE (fn) != ADDR_EXPR) {
        AD_DEBUG_PRINT ("Indirect call detected -> ESCAPE");
        AD_RETURNO (SU_ESCAPE_INDIRECT_CALL);
      }

      // 直接调用 - 全量分析：无论内部还是外部函数均视为逃逸
      tree fn_decl = TREE_OPERAND (fn, 0);
      bool is_external = false;
      AD_TRY (isFunctionExternal (AD_ARGS, fn_decl, is_external));

      if (is_external) {
        AD_DEBUG_PRINT ("External function call -> ESCAPE");
        AD_RETURNO (SU_ESCAPE_EXTERNAL_CALL);
      } else {
        AD_DEBUG_PRINT ("Internal function call -> ESCAPE (full analysis mode)");
        AD_RETURNO (SU_ESCAPE_PARAMETER);
      }
    }

    case SU_USE_STORE: {
      // 分析存储目标
      if (!is_gimple_assign (stmt)) break;

      tree lhs = gimple_assign_lhs (stmt);
      if (!lhs) break;

      // 全局变量
      if (TREE_CODE (lhs) == VAR_DECL && is_global_var (lhs)) {
        AD_DEBUG_PRINT ("Global store detected -> ESCAPE");
        AD_RETURNO (SU_ESCAPE_GLOBAL_STORE);
      }

      // 间接存储（可能是堆或字段）
      if (TREE_CODE (lhs) == MEM_REF || TREE_CODE (lhs) == COMPONENT_REF) {
        if (TREE_CODE (lhs) == COMPONENT_REF) {
          AD_DEBUG_PRINT ("Field store detected -> ESCAPE");
          AD_RETURNO (SU_ESCAPE_FIELD_STORE);
        } else {
          AD_DEBUG_PRINT ("Heap store detected -> ESCAPE");
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
    AD_DEBUG_PRINT ("Max trace depth reached: %u", depth);
    result->is_fully_analyzed = false;
    AD_RETURNE (OK);
  }

  if (!ssa_name || TREE_CODE (ssa_name) != SSA_NAME) {
    AD_DEBUG_PRINT ("Not an SSA_NAME, skipping");
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("Tracing SSA use chain escapes at depth %u", depth);

  // 手动遍历 immediate use list（避免使用有问题的迭代器宏）
  // SSA names 的 immediate uses 存储在一个循环链表中
  ssa_use_operand_t * head = &(SSA_NAME_IMM_USE_NODE (ssa_name));
  ssa_use_operand_t * ptr = head->next;

  unsigned int use_count = 0;

  // 遍历循环链表，直到回到 head
  while (ptr != head) {
    gimple * use_stmt = USE_STMT (ptr);

    if (use_stmt) {
      // === 关键修改：排除当前正在分析的写入语句 ===
      // 如果该使用就是我们正在分析的写入操作本身，跳过它
      if (exclude_stmt && use_stmt == exclude_stmt) {
        AD_DEBUG_PRINT ("  Skipping use (excluded write stmt itself)");
        ptr = ptr->next;
        continue;
      }

      use_count++;
      AD_DEBUG_PRINT ("  Processing use #%u", use_count);

      // 创建使用信息
      SourceUseInfo use_info;
      SourceUseKind use_kind = SU_USE_OTHER;
      AD_TRY (classifySSAUseKind (AD_ARGS, use_stmt, ssa_name, use_kind));

      use_info.kind = use_kind;
      use_info.use_stmt = use_stmt;
      use_info.use_operand = ssa_name;
      use_info.source_location = gimple_location (use_stmt);
      use_info.bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;

      // 检测逃逸
      SourceUseEscapeKind escape_kind = SU_ESCAPE_NONE;
      AD_TRY (detectEscapeKind (AD_ARGS, use_info, escape_kind));

      use_info.escape_kind = escape_kind;
      use_info.is_escape = (escape_kind != SU_ESCAPE_NONE);

      AD_DEBUG_PRINT ("    Use kind: %s, Escape kind: %s",
                      getUseKindString (use_kind),
                      getEscapeKindString (escape_kind));

      // 添加到结果
      result->all_uses->safe_push (use_info);
      result->total_use_count++;

      // 如果是逃逸，记录逃逸位置
      if (use_info.is_escape) {
        AD_DEBUG_PRINT ("    ESCAPE detected: %s", getEscapeKindString (escape_kind));
        result->has_escape = true;
        result->escape_count++;

        SourceUseEscapeLocation escape_loc;
        escape_loc.kind = escape_kind;
        escape_loc.stmt = use_stmt;
        escape_loc.use_operand = ssa_name;
        escape_loc.source_location = use_info.source_location;
        escape_loc.bb_index = use_info.bb_index;

        // 提取逃逸目标信息
        if (is_gimple_call (use_stmt)) {
          tree fn = gimple_call_fndecl (use_stmt);
          if (fn && DECL_NAME (fn)) {
            escape_loc.escape_target = IDENTIFIER_POINTER (DECL_NAME (fn));
            escape_loc.target_info.function_decl = fn;
          } else {
            escape_loc.escape_target = "<indirect call>";
            escape_loc.target_info.function_decl = NULL;
          }
        } else if (is_gimple_assign (use_stmt)) {
          tree lhs = gimple_assign_lhs (use_stmt);
          if (TREE_CODE (lhs) == COMPONENT_REF) {
            tree field = TREE_OPERAND (lhs, 1);
            if (DECL_NAME (field)) {
              escape_loc.escape_target = IDENTIFIER_POINTER (DECL_NAME (field));
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

        result->escape_locations->safe_push (escape_loc);
        // 全量分析模式：继续收集所有逃逸位置，不提前停止
      }

      // 如果是赋值，继续追踪新的 SSA
      if (use_kind == SU_USE_ASSIGN && is_gimple_assign (use_stmt)) {
        tree lhs = gimple_assign_lhs (use_stmt);
        if (lhs && TREE_CODE (lhs) == SSA_NAME) {
          AD_DEBUG_PRINT ("  Following SSA assignment chain");
          AD_TRY (traceSSAUseChainEscapes (AD_ARGS, lhs, result, depth + 1, exclude_stmt));
        }
      }
    }

    // 移动到下一个 use
    ptr = ptr->next;
  }

  AD_DEBUG_PRINT ("SSA use chain analysis complete: %u uses found", use_count);
  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode collectSourceOperandEscapes (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  gimple * exclude_stmt,
  SourceUseAnalysisResult * &result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Collecting source operand escapes");

  // 使用临时变量构建结果
  SourceUseAnalysisResult * tmp_result = ggc_alloc <SourceUseAnalysisResult> ();
  memset (tmp_result, 0, sizeof (SourceUseAnalysisResult));

  tmp_result->source_operand = source_operand;
  tmp_result->source_stmt = source_stmt;

  // 分配并初始化 vectors
  tmp_result->all_uses = ggc_alloc <vec<SourceUseInfo>> ();
  tmp_result->all_uses->create (0);
  tmp_result->total_use_count = 0;

  tmp_result->escape_locations = ggc_alloc <vec<SourceUseEscapeLocation>> ();
  tmp_result->escape_locations->create (0);
  tmp_result->escape_count = 0;

  tmp_result->has_escape = false;
  tmp_result->dominant_escape_kind = SU_ESCAPE_NONE;
  tmp_result->use_chain_root = NULL;
  tmp_result->max_use_depth = 0;
  tmp_result->is_fully_analyzed = true;
  tmp_result->aux = NULL;
  tmp_result->original_write_info = NULL;

  // 开始跟踪收集（排除指定的语句）
  if (source_operand && TREE_CODE (source_operand) == SSA_NAME) {
    AD_DEBUG_PRINT ("Source operand is SSA_NAME, starting escape trace");
    AD_TRY (traceSSAUseChainEscapes (AD_ARGS, source_operand, tmp_result, 0, exclude_stmt));
  } else {
    AD_DEBUG_PRINT ("Source operand is not SSA_NAME, skipping escape trace");
  }

  // 注意：不在这里计算dominant_escape_kind
  // 逃逸综合分析将在下一个模块完成，这里只收集所有详尽的逃逸信息
  if (tmp_result->has_escape) {
    AD_DEBUG_PRINT ("Escape analysis complete: %u total uses, %u escapes, %u escape locations",
                    tmp_result->total_use_count,
                    tmp_result->escape_count,
                    tmp_result->escape_locations ? tmp_result->escape_locations->length () : 0);
  }

  // 通过返回值宏统一返回
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
  AD_DEBUG_PRINT ("Collecting field write escapes");

  if (!write_capture) {
    AD_DEBUG_PRINT ("Write capture is NULL");
    AD_RETURNO (NULL);
  }

  // 使用 FieldWriteCapture
  FieldWriteCapture * write_info = write_capture;

  // 提取源操作数
  tree source_operand = write_info->rhs;    // 右值表达式
  gimple * source_stmt = write_info->stmt;   // GIMPLE 语句

  // === 关键修改：排除当前写入语句本身 ===
  // 当前写入语句是 write_info->stmt（例如 a.ptr = b.ptr）
  // 我们分析的是源操作数 b.ptr 的逃逸
  // 但 b.ptr 被用在 a.ptr = b.ptr 这个语句中，这不应该被视为逃逸
  gimple * exclude_stmt = write_info->stmt;

  // 使用临时变量接收收集结果
  SourceUseAnalysisResult * tmp_result = NULL;
  AD_TRY (collectSourceOperandEscapes (AD_ARGS, source_operand, source_stmt, exclude_stmt, tmp_result));

  if (tmp_result) {
    // 关联原始数据
    tmp_result->original_write_info = write_capture;

    // ========== 异构链表更新 ==========
    // 1. 从原 hash_map 的 aux 摘下旧数据
    void * old_aux = write_info->aux;

    // 2. 将新结果的 aux 指向旧数据
    tmp_result->aux = old_aux;

    // 3. 更新原数据的 aux 指向新结果
    write_info->aux = tmp_result;

    // 这样就实现了异构伪 list 的更新：
    // hash_map -> write_info -> (aux) -> source_use_result -> (aux) -> old_data

    AD_DEBUG_PRINT ("Write capture analysis complete: %u uses, %u escapes, %u escape locations",
                    tmp_result->total_use_count,
                    tmp_result->escape_count,
                    tmp_result->escape_locations ? tmp_result->escape_locations->length () : 0);
  }

  // 通过返回值宏统一返回
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

void printSourceUseAnalysisResult(
  const SourceUseAnalysisResult * result,
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
  if (result->escape_locations) {
    fprintf(output, "Escape locations count: %u\n", result->escape_locations->length());
  }

  fprintf(output, "\n--- All Uses (Detailed) ---\n");
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

  fprintf(output, "\n--- Escape Locations (Detailed) ---\n");
  if (result->escape_locations) {
    for (unsigned i = 0; i < result->escape_locations->length(); i++) {
      const SourceUseEscapeLocation& loc = (*result->escape_locations)[i];
      fprintf(output, "[%u] Escape Kind: %s\n",
              i, getEscapeKindString(loc.kind));
      fprintf(output, "    Target: %s\n",
              loc.escape_target ? loc.escape_target : "<unknown>");
      fprintf(output, "    BB Index: %u\n", loc.bb_index);
      if (loc.source_location != UNKNOWN_LOCATION) {
        expanded_location xloc = expand_location(loc.source_location);
        fprintf(output, "    Location: %s:%d:%d\n",
                xloc.file, xloc.line, xloc.column);
      }
    }
  } else {
    fprintf(output, "  (No escape locations recorded)\n");
  }

  fprintf (output, "\nFully analyzed: %s\n",
          result->is_fully_analyzed ? "YES" : "NO (depth limit reached)");
  fprintf (output, "===================================\n");
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
  AD_DEBUG_PRINT ("Collecting all field escapes (full analysis mode)");

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
    if (!write_ops || !write_ops->write_ops) continue;

    for (unsigned i = 0; i < write_ops->write_ops->length (); i++) {
      FieldWriteCapture * capture = (*write_ops->write_ops)[i];
      if (!capture) continue;

      SourceUseAnalysisResult * use_result = NULL;
      AD_TRY (collectFieldWriteEscapes (AD_ARGS, capture, use_result));

      if (use_result) {
        total_analyzed++;
        if (use_result->has_escape) {
          total_escaped++;
        }

        // 将逃逸分析结果存储到 capture->aux，供后续综合器使用
        // 注意：aux 字段在 write-operation-trace 阶段存储 FieldSourceInfo*
        // 我们需要将其转移到 SourceUseAnalysisResult 中，然后存储 SourceUseAnalysisResult
        use_result->original_write_info = capture->aux;  // 保存原来的 FieldSourceInfo*
        capture->aux = use_result;  // 用 SourceUseAnalysisResult 替换 aux

        // 输出所有收集到的 escape 信息到 stderr
        printSourceUseAnalysisResult (use_result, stderr);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
