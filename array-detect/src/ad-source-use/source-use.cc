// ============================================================================
// ad-source-use 模块实现
// ============================================================================
// 分析源操作数的 SSA 使用链，收集所有使用点和逃逸信息
// 数据流：source_operand -> (listof SourceUseInfo)
// ============================================================================

#include "source-use.hh"
#include "array-detector.hh"
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
// 内部实现
// ============================================================================

namespace {

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
// 记录使用点 - 直接写入 SourceUseInfo

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_recordUsePoint (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree use_operand,
  SourceUseKind use_kind,
  SourceUseEscapeKind escape_kind,
  char const * escape_target,
  vec<SourceUseInfo>* all_uses
) AD_FUNCTION_BEGIN {
  if (!all_uses) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  SourceUseInfo use_point;
  use_point.kind = use_kind;
  use_point.use_stmt = use_stmt;
  use_point.use_operand = use_operand;
  use_point.source_location = gimple_location (use_stmt);
  use_point.bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;
  use_point.escape_kind = escape_kind;
  use_point.escape_target = escape_target;
  use_point.target_info.function_decl = NULL;

  all_uses->safe_push (use_point);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain
// ----------------------------------------------------------------------------
// 递归追踪 SSA 使用链

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain (
  AD_FUNC_ARGS,
  tree ssa_name,
  vec<SourceUseInfo>* all_uses,
  unsigned int depth,
  gimple * exclude_stmt
) AD_FUNCTION_BEGIN {
  if (depth >= MAX_ESCAPE_ANALYSIS_DEPTH) {
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
        escape_target, all_uses
      ));

      // 如果是简单赋值，继续追踪结果 SSA
      if (use_kind == SU_USE_ASSIGN && is_gimple_assign (use_stmt)) {
        tree lhs = gimple_assign_lhs (use_stmt);
        if (lhs && TREE_CODE (lhs) == SSA_NAME) {
          AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, lhs, all_uses, depth + 1, exclude_stmt));
        }
      }
    }

    ptr = ptr->next;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

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
// 输出：(listof SourceUseInfo)

ArrayDetectErrorCode analyzeSourceUse (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * exclude_stmt,
  vec<SourceUseInfo>** out_uses
) AD_FUNCTION_BEGIN {
  if (!out_uses) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配结果向量
  *out_uses = ggc_alloc<vec<SourceUseInfo>> ();
  if (!*out_uses) {
    AD_RETURNE (MEMORY_ERROR);
  }
  (*out_uses)->create (0);

  // 追踪 SSA 使用链
  if (source_operand && TREE_CODE (source_operand) == SSA_NAME) {
    AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, source_operand,
      *out_uses, 0, exclude_stmt));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldEscapes
// ----------------------------------------------------------------------------
// Pipeline 接口：收集所有字段的逃逸信息

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
    TypeFieldAnalysisData * tfad = (*iter).second;
    if (!tfad || !tfad->writes) continue;

    for (unsigned i = 0; i < tfad->writes->length (); i++) {
      field_analysis::Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*tfad->writes)[i];
      if (!wrapper || !wrapper->write_info) continue;

      // 从 write_info 提取数据进行分析
      FieldWriteInfo* write_info = wrapper->write_info;
      tree source_operand = write_info->rhs;
      gimple * exclude_stmt = write_info->stmt;

      // 分析源使用
      vec<SourceUseInfo>* all_uses = NULL;
      AD_TRY (analyzeSourceUse (AD_ARGS, source_operand, exclude_stmt, &all_uses));

      // 分配并填充 UseWrapper
      if (!wrapper->uses) {
        vec_alloc (wrapper->uses, 4);
      }

      auto* use_wrapper = ggc_alloc<field_analysis::Wrapper_SourceUse_EscapedUse> ();
      if (!use_wrapper) {
        AD_RETURNE (MEMORY_ERROR);
      }
      memset (use_wrapper, 0, sizeof (field_analysis::Wrapper_SourceUse_EscapedUse));
      use_wrapper->all_uses = all_uses;

      vec_safe_push (wrapper->uses, use_wrapper);
      total_analyzed++;

      // 检查是否有逃逸
      if (all_uses) {
        for (unsigned j = 0; j < all_uses->length (); j++) {
          if ((*all_uses)[j].is_escape ()) {
            total_escaped++;
            break;
          }
        }
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
