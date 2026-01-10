// ============================================================================
// ad-source-escape-use-info 模块实现
// ============================================================================
// 提取逃逸使用信息
// 数据流：source-use-info -> source-escape-use-info
// 从纯 SourceUseInfo 计算逃逸信息
// ============================================================================

#include "source-escape-use-info.hh"
#include "source-use-info.hh"
#include "array-detector.hh"
#include "info-print.hh"

#include "tree.h"
#include "gimple.h"
#include "cgraph.h"

namespace array_detect_ns {

using namespace ::field_analysis;

// ============================================================================
// 内部实现 - 逃逸检测逻辑
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// detectEscape_isFunctionExternal
// ----------------------------------------------------------------------------
// 判断函数是否为外部函数

bool detectEscape_isFunctionExternal (tree function_decl) {
  if (!function_decl || TREE_CODE (function_decl) != FUNCTION_DECL) {
    return true;
  }

  struct cgraph_node * node = cgraph_node::get (function_decl);
  if (!node || !node->definition) {
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// detectEscapeKind
// ----------------------------------------------------------------------------
// 从纯 SourceUseInfo 检测逃逸类型

SourceUseEscapeKind detectEscapeKind (SourceUseInfo const* use_info) {
  if (!use_info || !use_info->use_stmt) {
    return SU_ESCAPE_NONE;
  }

  gimple* stmt = use_info->use_stmt;

  switch (use_info->kind) {
    case SU_USE_RETURN:
      return SU_ESCAPE_RETURN;

    case SU_USE_CALL_ARG: {
      if (!is_gimple_call (stmt)) break;

      tree fn = gimple_call_fn (stmt);
      if (!fn) {
        return SU_ESCAPE_UNKNOWN;
      }

      if (TREE_CODE (fn) == OBJ_TYPE_REF) {
        return SU_ESCAPE_VIRTUAL_CALL;
      }

      if (TREE_CODE (fn) != ADDR_EXPR) {
        return SU_ESCAPE_INDIRECT_CALL;
      }

      tree fn_decl = TREE_OPERAND (fn, 0);
      if (detectEscape_isFunctionExternal (fn_decl)) {
        return SU_ESCAPE_EXTERNAL_CALL;
      } else {
        return SU_ESCAPE_PARAMETER;
      }
    }

    case SU_USE_STORE: {
      if (!is_gimple_assign (stmt)) break;

      tree lhs = gimple_assign_lhs (stmt);
      if (!lhs) break;

      if (TREE_CODE (lhs) == VAR_DECL && is_global_var (lhs)) {
        return SU_ESCAPE_GLOBAL_STORE;
      }

      if (TREE_CODE (lhs) == MEM_REF || TREE_CODE (lhs) == COMPONENT_REF) {
        if (TREE_CODE (lhs) == COMPONENT_REF) {
          return SU_ESCAPE_FIELD_STORE;
        } else {
          return SU_ESCAPE_HEAP_STORE;
        }
      }
      break;
    }

    default:
      break;
  }

  return SU_ESCAPE_NONE;
}

// ----------------------------------------------------------------------------
// detectEscapeTarget
// ----------------------------------------------------------------------------
// 从 SourceUseInfo 提取逃逸目标描述

char const* detectEscapeTarget (SourceUseInfo const* use_info, SourceUseEscapeKind escape_kind) {
  if (!use_info || escape_kind == SU_ESCAPE_NONE) {
    return NULL;
  }

  gimple* stmt = use_info->use_stmt;
  if (!stmt) return "<unknown>";

  if (is_gimple_call (stmt)) {
    tree fn = gimple_call_fndecl (stmt);
    if (fn && DECL_NAME (fn)) {
      return IDENTIFIER_POINTER (DECL_NAME (fn));
    } else {
      return "<indirect call>";
    }
  } else if (is_gimple_assign (stmt)) {
    tree lhs = gimple_assign_lhs (stmt);
    if (lhs && TREE_CODE (lhs) == COMPONENT_REF) {
      tree field = TREE_OPERAND (lhs, 1);
      if (DECL_NAME (field)) {
        return IDENTIFIER_POINTER (DECL_NAME (field));
      } else {
        return "<anonymous field>";
      }
    } else {
      return "<memory>";
    }
  }

  return "<unknown>";
}

// ----------------------------------------------------------------------------
// detectEscapeTargetDecl
// ----------------------------------------------------------------------------
// 从 SourceUseInfo 提取逃逸目标声明

tree detectEscapeTargetDecl (SourceUseInfo const* use_info, SourceUseEscapeKind escape_kind) {
  if (!use_info || escape_kind == SU_ESCAPE_NONE) {
    return NULL;
  }

  gimple* stmt = use_info->use_stmt;
  if (!stmt) return NULL;

  if (is_gimple_call (stmt)) {
    return gimple_call_fndecl (stmt);
  } else if (is_gimple_assign (stmt)) {
    tree lhs = gimple_assign_lhs (stmt);
    if (lhs && TREE_CODE (lhs) == COMPONENT_REF) {
      return TREE_OPERAND (lhs, 1);  // field decl
    } else if (lhs && TREE_CODE (lhs) == VAR_DECL) {
      return lhs;  // global var
    }
  }

  return NULL;
}

} // anonymous namespace

// ============================================================================
// 辅助函数实现
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
// isEscapeSafeDebug
// ----------------------------------------------------------------------------
// 判断是否为安全调试逃逸（如 printf/fprintf 等）

bool isEscapeSafeDebug (
  AD_FUNC_ARGS,
  SourceUseInfo const* use_info
) {
  (void)ctx; (void)gcc_ctx;

  if (!use_info) return false;

  // 先检测逃逸类型
  SourceUseEscapeKind escape_kind = detectEscapeKind (use_info);
  if (escape_kind == SU_ESCAPE_NONE) return false;

  // 获取逃逸目标
  char const* target = detectEscapeTarget (use_info, escape_kind);
  if (!target) return false;

  // 常见的调试输出函数
  if (strcmp (target, "printf") == 0 ||
      strcmp (target, "fprintf") == 0 ||
      strcmp (target, "vprintf") == 0 ||
      strcmp (target, "vfprintf") == 0 ||
      strcmp (target, "puts") == 0 ||
      strcmp (target, "fputs") == 0 ||
      strcmp (target, "fwrite") == 0 ||
      strcmp (target, "write") == 0 ||
      strcmp (target, "__builtin_printf") == 0) {
    return true;
  }

  // 检查是否包含 "debug"、"log"、"trace" 等关键词
  if (strstr (target, "debug") != NULL ||
      strstr (target, "Debug") != NULL ||
      strstr (target, "DEBUG") != NULL ||
      strstr (target, "log") != NULL ||
      strstr (target, "Log") != NULL ||
      strstr (target, "LOG") != NULL ||
      strstr (target, "trace") != NULL ||
      strstr (target, "Trace") != NULL ||
      strstr (target, "TRACE") != NULL ||
      strstr (target, "dump") != NULL ||
      strstr (target, "Dump") != NULL ||
      strstr (target, "DUMP") != NULL ||
      strstr (target, "print") != NULL ||
      strstr (target, "Print") != NULL ||
      strstr (target, "PRINT") != NULL) {
    return true;
  }

  return false;
}

// ============================================================================
// 逃逸使用提取
// ============================================================================
// 输入：wrapper 列表（已填充 use_info）
// 输出：填充每个 wrapper 的 escape_use_info 字段
// 注意：现在从纯 SourceUseInfo 计算逃逸信息

ArrayDetectErrorCode extractSourceEscapeUseInfo (
  AD_FUNC_ARGS,
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses
) AD_FUNCTION_BEGIN {
  if (!uses) {
    AD_RETURNE (OK);  // 空列表，无需处理
  }

  for (unsigned int i = 0; i < uses->length (); i++) {
    Wrapper_SourceUseInfo_SourceEscapeUseInfo* wrapper = (*uses)[i];
    if (!wrapper || !wrapper->use_info) continue;

    SourceUseInfo const* use_info = wrapper->use_info;

    // 从纯 SourceUseInfo 计算逃逸类型
    SourceUseEscapeKind escape_kind = detectEscapeKind (use_info);

    // 检查是否为逃逸
    if (escape_kind == SU_ESCAPE_NONE) {
      wrapper->escape_use_info = NULL;
      continue;
    }

    // 创建 SourceEscapeUseInfo
    auto* escape_use_info = ggc_alloc<SourceEscapeUseInfo> ();
    if (!escape_use_info) {
      AD_RETURNE (MEMORY_ERROR);
    }

    // 填充逃逸信息 - 现在从纯 use_info 计算
    escape_use_info->escape_kind = escape_kind;
    escape_use_info->escape_target = detectEscapeTarget (use_info, escape_kind);
    escape_use_info->target_decl = detectEscapeTargetDecl (use_info, escape_kind);
    escape_use_info->is_safe_debug = isEscapeSafeDebug (AD_ARGS, use_info);

    wrapper->escape_use_info = escape_use_info;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// Pipeline 接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// extractAllSourceEscapeUseInfo
// ----------------------------------------------------------------------------
// 提取所有字段的逃逸使用信息

ArrayDetectErrorCode extractAllSourceEscapeUseInfo (
  AD_FUNC_ARGS,
  ::array_detector::ArrayDetector &detector,
  unsigned int &total_extracted
) AD_FUNCTION_BEGIN {
  total_extracted = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<::array_detector::TypeFieldKey, ::array_detector::TypeFieldAnalysisData*, ::array_detector::TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    ::array_detector::TypeFieldAnalysisData * tfad = (*iter).second;
    if (!tfad || !tfad->writes) continue;

    for (unsigned i = 0; i < tfad->writes->length (); i++) {
      Wrapper_WriteInfo_WriteSource_SourceEscapeConclude * wrapper = (*tfad->writes)[i];
      if (!wrapper || !wrapper->uses) continue;

      // 提取该写入的所有使用的逃逸信息
      AD_TRY (extractSourceEscapeUseInfo (AD_ARGS, wrapper->uses));
      total_extracted++;
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printSourceEscapeUseInfo (
  SourceEscapeUseInfo const* info,
  FILE* output
) {
  if (!info || !output) return;

  fprintf (output, "  SourceEscapeUseInfo:\n");
  fprintf (output, "    escape_kind: %s\n", getEscapeKindString (info->escape_kind));
  fprintf (output, "    escape_target: %s\n",
           info->escape_target ? info->escape_target : "<none>");
  fprintf (output, "    target_decl: %p\n", (void*)info->target_decl);
  fprintf (output, "    is_safe_debug: %s\n", info->is_safe_debug ? "true" : "false");
}

} // namespace array_detect_ns
