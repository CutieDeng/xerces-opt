// ============================================================================
// ad-source-use-info 模块实现
// ============================================================================
// 分析源操作数的 SSA 使用链，收集所有使用点和逃逸信息
// 数据流：source_operand -> (listof wrapper-source-use-info-source-escape-use-info)
// ============================================================================

#include "source-use-info.hh"
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
using namespace ::field_analysis;

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
// 记录使用点 - 创建 Wrapper_SourceUseInfo_SourceEscapeUseInfo

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_recordUsePoint (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree use_operand,
  SourceUseKind use_kind,
  SourceUseEscapeKind escape_kind,
  char const * escape_target,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* out_uses
) AD_FUNCTION_BEGIN {
  if (!out_uses) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 分配 Wrapper
  auto* wrapper = ggc_alloc<Wrapper_SourceUseInfo_SourceEscapeUseInfo> ();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (Wrapper_SourceUseInfo_SourceEscapeUseInfo));

  // 分配 SourceUseInfo
  auto* use_info = ggc_alloc<SourceUseInfo> ();
  if (!use_info) {
    AD_RETURNE (MEMORY_ERROR);
  }

  use_info->kind = use_kind;
  use_info->use_stmt = use_stmt;
  use_info->use_operand = use_operand;
  use_info->source_location = gimple_location (use_stmt);
  use_info->bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;
  use_info->escape_kind = escape_kind;
  use_info->escape_target = escape_target;
  use_info->target_info.function_decl = NULL;

  // 设置 Wrapper 字段
  // 注意：escape_use_info 由 source-escape-use-info 模块填充，此处只设置 use_info
  wrapper->use_info = use_info;
  wrapper->escape_use_info = NULL;

  vec_safe_push (out_uses, wrapper);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain
// ----------------------------------------------------------------------------
// 递归追踪 SSA 使用链

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain (
  AD_FUNC_ARGS,
  tree ssa_name,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* out_uses,
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
        escape_target, out_uses
      ));

      // 如果是简单赋值，继续追踪结果 SSA
      if (use_kind == SU_USE_ASSIGN && is_gimple_assign (use_stmt)) {
        tree lhs = gimple_assign_lhs (use_stmt);
        if (lhs && TREE_CODE (lhs) == SSA_NAME) {
          AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, lhs, out_uses, depth + 1, exclude_stmt));
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
// 输出：(listof wrapper-source-use-info-source-escape-use-info)

ArrayDetectErrorCode analyzeSourceUse (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * exclude_stmt,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>** out_uses
) AD_FUNCTION_BEGIN {
  if (!out_uses) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 使用 vec_alloc 分配 GGC 管理的向量
  vec_alloc (*out_uses, 4);
  if (!*out_uses) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // 追踪 SSA 使用链
  if (source_operand && TREE_CODE (source_operand) == SSA_NAME) {
    AD_TRY (analyzeSourceUse_traceSSAUseChain (AD_ARGS, source_operand,
      *out_uses, 0, exclude_stmt));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldUses
// ----------------------------------------------------------------------------
// Pipeline 接口：收集所有字段的使用信息
// 注意：此函数只填充 uses，escape_use_info 由 source-escape-use-info 模块后续填充

ArrayDetectErrorCode collectAllFieldUses (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  unsigned int &total_analyzed
) AD_FUNCTION_BEGIN {
  total_analyzed = 0;

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
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*tfad->writes)[i];
      if (!wrapper || !wrapper->write_info) continue;

      // 从 write_info 提取数据进行分析
      FieldWriteInfo* write_info = wrapper->write_info;
      tree source_operand = write_info->rhs;
      gimple * exclude_stmt = write_info->stmt;

      // 分析源使用 - 直接获取 wrapper 列表
      vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses = NULL;
      AD_TRY (analyzeSourceUse (AD_ARGS, source_operand, exclude_stmt, &uses));

      // 直接设置 wrapper->uses
      wrapper->uses = uses;
      total_analyzed++;
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
