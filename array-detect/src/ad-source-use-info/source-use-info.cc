// ============================================================================
// ad-source-use-info 模块实现
// ============================================================================
// 分析源操作数的 SSA 使用链，收集所有使用点
// 数据流：source_operand -> (listof wrapper-source-use-info-source-escape-use-info)
// 注意：只收集纯使用信息，逃逸检测由 source-escape-use-info 模块负责
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
// analyzeSourceUse_traceSSAUseChain_recordUsePoint
// ----------------------------------------------------------------------------
// 记录使用点 - 创建 Wrapper_SourceUseInfo_SourceEscapeUseInfo
// 注意：只记录纯使用信息，escape_use_info 由 source-escape-use-info 模块后续填充

ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain_recordUsePoint (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree use_operand,
  SourceUseKind use_kind,
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

  // 分配 SourceUseInfo - 只包含纯使用信息
  auto* use_info = ggc_alloc<SourceUseInfo> ();
  if (!use_info) {
    AD_RETURNE (MEMORY_ERROR);
  }

  use_info->kind = use_kind;
  use_info->use_stmt = use_stmt;
  use_info->use_operand = use_operand;
  use_info->source_location = gimple_location (use_stmt);
  use_info->bb_index = gimple_bb (use_stmt) ? gimple_bb (use_stmt)->index : 0;

  // 设置 Wrapper 字段
  // escape_use_info 由 source-escape-use-info 模块填充
  wrapper->use_info = use_info;
  wrapper->escape_use_info = NULL;

  vec_safe_push (out_uses, wrapper);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeSourceUse_traceSSAUseChain
// ----------------------------------------------------------------------------
// 递归追踪 SSA 使用链 - 只收集纯使用信息

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

      // 记录使用点（只记录纯使用信息）
      AD_TRY (analyzeSourceUse_traceSSAUseChain_recordUsePoint (
        AD_ARGS, use_stmt, ssa_name, use_kind, out_uses
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
// 注意：只填充 use_info，escape_use_info 由 source-escape-use-info 模块填充

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
