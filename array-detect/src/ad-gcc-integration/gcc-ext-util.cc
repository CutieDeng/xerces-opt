#include "gcc-ext-util.hh"

#include "info.hh"
#include "array-detector.hh"

#include <cxxabi.h>
#include <cstring>

namespace gcc_ext_util {

// 安全字符串复制函数，防止缓冲区溢出
void safe_string_copy (char * dest, size_t dest_size, char const * src) {
  if (dest_size == 0) return;
  if (!src) {
    dest[0] = '\0';
    return;
  }
  
  // 使用strncpy确保不会溢出，并确保字符串以null结尾
  strncpy (dest, src, dest_size - 1);
  dest[dest_size - 1] = '\0';
}

// 新增辅助函数：获取详细的源码位置信息
// 使用预分配的缓冲区，避免动态内存分配
ArrayDetectErrorCode get_source_location_string (AD_FUNC_ARGS, location_t loc, char * buffer, size_t buffer_size) AD_FUNCTION_BEGIN {
  if (loc == UNKNOWN_LOCATION) {
    snprintf (buffer, buffer_size, "<unknown location>");
    AD_RETURNE (OK);
  }
  
  expanded_location xloc = expand_location (loc);
  
  if (xloc.file) {
    // 输出格式改为 a.cc +linenumber colnumber 的形式
    if (xloc.line > 0) {
      if (xloc.column > 0) {
        snprintf (buffer, buffer_size, "%s +%d %d", xloc.file, xloc.line, xloc.column);
      } else {
        snprintf (buffer, buffer_size, "%s +%d", xloc.file, xloc.line);
      }
    } else {
      snprintf (buffer, buffer_size, "%s", xloc.file);
    }
  } else {
    snprintf (buffer, buffer_size, "<unknown location>");
  }
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 新增辅助函数：获取指定位置的源码行内容
// 使用预分配的缓冲区，避免动态内存分配
void get_source_line_content (location_t loc, char * buffer, size_t buffer_size) {
  if (loc == UNKNOWN_LOCATION) {
    snprintf (buffer, buffer_size, "&lt;source line content not available&gt;");
    return;
  }
  
  expanded_location xloc = expand_location (loc);
  
  if (xloc.file && xloc.line > 0) {
    // 尝试读取源代码文件的指定行
    FILE * file = fopen (xloc.file, "r");
    if (file) {
      char line_buffer[1024];
      size_t current_line = 0;
      while (fgets (line_buffer, sizeof (line_buffer), file) && current_line < ((size_t) xloc.line)) {
        current_line++;
        if (current_line == ((size_t) xloc.line)) {
          // 移除行尾的换行符
          size_t len = strlen (line_buffer);
          if (len > 0 && line_buffer[len-1] == '\n') {
            line_buffer[len-1] = '\0';
          }
          // 复制到输出缓冲区，注意不要溢出
          strncpy (buffer, line_buffer, buffer_size - 1);
          buffer[buffer_size - 1] = '\0';
          fclose (file);
          return;
        }
      }
      fclose (file);
    }
  }
  
  // 如果无法读取源代码行，则返回默认值
  snprintf (buffer, buffer_size, "<source line content not available>");
}

namespace {

ArrayDetectErrorCode get_call_expr_name (AD_FUNC_ARGS, tree call_expr, char const *&result) AD_FUNCTION_BEGIN {
  if (TREE_CODE (call_expr) != CALL_EXPR) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  tree fn = TREE_OPERAND (call_expr, 0);
  if (!fn) {
    AD_RETURNO ("<call-expr>");
  }
  
  if (TREE_CODE (fn) == FUNCTION_DECL && DECL_NAME (fn)) {
    AD_RETURNO (IDENTIFIER_POINTER (DECL_NAME (fn)));
  }
  if (TREE_CODE (fn) == ADDR_EXPR) {
    tree decl = TREE_OPERAND (fn, 0);
    if (decl && DECL_NAME (decl)) {
      AD_RETURNO (IDENTIFIER_POINTER (DECL_NAME (decl)));
    }
  }
  if (TREE_CODE (fn) == INDIRECT_REF) {
    AD_RETURNO ("<indirect-call>");
  }
  AD_RETURNO ("<call-expr>");
} AD_FUNCTION_END

ArrayDetectErrorCode gcc_field_desc (AD_FUNC_ARGS, tree field, char const *&result) AD_FUNCTION_BEGIN {
  if (!field) {
    AD_RETURNO ("<null>");
  }
  if (DECL_NAME (field)) {
    AD_RETURNO (IDENTIFIER_POINTER (DECL_NAME (field)));
  }
  AD_RETURNO ("<unnamed>");
} AD_FUNCTION_END

}
}

namespace gcc_ext_util {

// 获取类型名称：从 GCC tree 节点提取类型名称字符串
// 语义：返回类型的可读名称，用于调试和报告
// 垃圾回收：返回的指针指向 GCC 内部管理的字符串，无需释放
ArrayDetectErrorCode get_type_name (AD_FUNC_ARGS, tree type, char const *&result) AD_FUNCTION_BEGIN {
  if (!type) AD_RETURNO ("<unknown>");
  
  if (TYPE_NAME (type)) {
    if (TREE_CODE (TYPE_NAME (type)) == IDENTIFIER_NODE) {
      // 返回 GCC 内部管理的标识符指针
      AD_RETURNO (IDENTIFIER_POINTER (TYPE_NAME (type)));
    } else if (TREE_CODE (TYPE_NAME (type)) == TYPE_DECL) {
      tree name = DECL_NAME (TYPE_NAME (type));
      if (name) {
        // 返回 GCC 内部管理的声明名称指针
        AD_RETURNO (IDENTIFIER_POINTER (name));
      }
    }
  }
  AD_RETURNO ("<unnamed>");
} AD_FUNCTION_END

// 辅助函数：获取模版参数列表并格式化
// 返回：格式化的模版参数字符串存入 ctx.escaped_string_buffer
// 注意：需要 C++ 前端头文件 cp-tree.h 才能获取完整的模版信息
// 在纯 GCC 插件环境中，这些宏可能不可用，此时回退到基础类型名
static ArrayDetectErrorCode formatTemplateArgs (AD_FUNC_ARGS, tree type, bool &has_template_args) AD_FUNCTION_BEGIN {
  has_template_args = false;

  // 使用 context 中的 escaped_string_buffer 作为模版参数缓冲区
  if (!ctx.escaped_string_buffer || ctx.escaped_string_buffer_size == 0) {
    AD_RETURNE (OK);
  }
  ctx.escaped_string_buffer[0] = '\0';

  if (!type) {
    AD_RETURNE (OK);
  }

  // 检查是否是模版实例化类型
  // 尝试从类型名称本身获取模版参数（类型名可能包含完整的模版签名）
  tree type_decl = TYPE_NAME (type);
  if (!type_decl || TREE_CODE (type_decl) != TYPE_DECL) {
    AD_RETURNE (OK);
  }

  // 尝试从 DECL_ORIGINAL_TYPE 获取原始模版类型
  tree original_type = DECL_ORIGINAL_TYPE (type_decl);
  if (original_type && original_type != type) {
    // 这是一个 typedef，不是模版实例化
    AD_RETURNE (OK);
  }

  // 直接从 GCC 的类型打印中提取模版参数
  // 如果 TYPE_NAME 包含完整的模版签名（某些 GCC 版本会这样做），则使用它
  if (type_decl && TREE_CODE (type_decl) == TYPE_DECL) {
    // 检查声明的汇编名称是否包含模版信息
    tree assembler_name = DECL_ASSEMBLER_NAME (type_decl);
    if (assembler_name && TREE_CODE (assembler_name) == IDENTIFIER_NODE) {
      (void)IDENTIFIER_POINTER (assembler_name);
      // C++ mangled name contains template parameter encoding
    }
  }

  // 检查 TYPE_LANG_SPECIFIC 是否存在
  // 某些 GCC 版本中，C++ 类型信息存储在这里
#if defined(TYPE_LANG_SPECIFIC) && defined(CLASSTYPE_TEMPLATE_INFO)
  // 这些宏仅在包含 cp-tree.h 时可用
  // 在标准插件环境中通常不可用
  if (TYPE_LANG_SPECIFIC (type)) {
    tree template_info = CLASSTYPE_TEMPLATE_INFO (type);
    if (template_info) {
      tree template_args = TI_ARGS (template_info);
      // 处理模版参数，结果存入 ctx.escaped_string_buffer
      has_template_args = true;
    }
  }
#else
  // C++ template info not available in plugin environment
#endif

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 模板参数提取函数
// ============================================================================

// 从 demangled name 解析模板参数
// 输入: "Vector<int, float>" 或 "std::vector<int, std::allocator<int>>"
// 输出: base_name, template_args 列表
static ArrayDetectErrorCode parseTemplateArgsFromDemangled (
  AD_FUNC_ARGS,
  char const* demangled_name,
  char const** out_base_name,
  vec<char const*, va_gc>** out_template_args
) AD_FUNCTION_BEGIN {
  *out_base_name = NULL;
  *out_template_args = NULL;

  if (!demangled_name || demangled_name[0] == '\0') {
    *out_base_name = "<unknown>";
    AD_RETURNE (OK);
  }

  // 查找第一个 '<' 作为模板参数开始
  char const* template_start = strchr (demangled_name, '<');
  if (!template_start) {
    // 非模板类型，直接返回原名
    *out_base_name = ggc_strdup (demangled_name);
    AD_RETURNE (OK);
  }

  // 提取基础类型名（'<' 之前的部分）
  size_t base_len = template_start - demangled_name;
  char* base_name = (char*)ggc_alloc_atomic (base_len + 1);
  memcpy (base_name, demangled_name, base_len);
  base_name[base_len] = '\0';
  *out_base_name = base_name;

  // 解析模板参数（处理嵌套的 '<' 和 '>'）
  vec<char const*, va_gc>* args = NULL;
  vec_alloc (args, 4);

  char const* p = template_start + 1;  // 跳过 '<'
  int depth = 1;
  char const* arg_start = p;

  while (*p && depth > 0) {
    if (*p == '<') {
      depth++;
    } else if (*p == '>') {
      depth--;
      if (depth == 0) {
        // 最后一个参数
        size_t arg_len = p - arg_start;
        if (arg_len > 0) {
          // 去除前后空格
          while (arg_len > 0 && arg_start[0] == ' ') { arg_start++; arg_len--; }
          while (arg_len > 0 && arg_start[arg_len - 1] == ' ') { arg_len--; }

          if (arg_len > 0) {
            char* arg = (char*)ggc_alloc_atomic (arg_len + 1);
            memcpy (arg, arg_start, arg_len);
            arg[arg_len] = '\0';
            vec_safe_push (args, (char const*)arg);
          }
        }
      }
    } else if (*p == ',' && depth == 1) {
      // 参数分隔符（只在最外层有效）
      size_t arg_len = p - arg_start;
      if (arg_len > 0) {
        // 去除前后空格
        while (arg_len > 0 && arg_start[0] == ' ') { arg_start++; arg_len--; }
        while (arg_len > 0 && arg_start[arg_len - 1] == ' ') { arg_len--; }

        if (arg_len > 0) {
          char* arg = (char*)ggc_alloc_atomic (arg_len + 1);
          memcpy (arg, arg_start, arg_len);
          arg[arg_len] = '\0';
          vec_safe_push (args, (char const*)arg);
        }
      }
      arg_start = p + 1;
    }
    p++;
  }

  *out_template_args = args;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 从 GCC type tree 提取模板参数信息
// 优先使用 cp-tree.h API，不可用时回退到 demangling
ArrayDetectErrorCode extractTemplateArgsFromType (
  AD_FUNC_ARGS,
  tree type,
  char const** out_base_name,
  vec<char const*, va_gc>** out_template_args
) AD_FUNCTION_BEGIN {
  *out_base_name = NULL;
  *out_template_args = NULL;

  if (!type) {
    *out_base_name = "<null-type>";
    AD_RETURNE (OK);
  }

  // 方案 1: 尝试使用 cp-tree.h API（条件编译）
#if defined(TYPE_LANG_SPECIFIC) && defined(CLASSTYPE_TEMPLATE_INFO) && defined(TI_ARGS) && defined(TREE_VEC_LENGTH) && defined(TREE_VEC_ELT)
  if (TYPE_LANG_SPECIFIC (type)) {
    tree template_info = CLASSTYPE_TEMPLATE_INFO (type);
    if (template_info) {
      tree template_args_tree = TI_ARGS (template_info);
      if (template_args_tree) {
        // 处理嵌套的 TREE_VEC (多级模板参数)
        while (template_args_tree && TREE_CODE (template_args_tree) == TREE_VEC) {
          int len = TREE_VEC_LENGTH (template_args_tree);
          if (len > 0) {
            tree first = TREE_VEC_ELT (template_args_tree, 0);
            // 如果第一个元素也是 TREE_VEC，说明有嵌套，取最内层
            if (first && TREE_CODE (first) == TREE_VEC) {
              template_args_tree = first;
              continue;
            }
          }
          break;
        }

        // 获取基础类型名
        char const* base_name = NULL;
        AD_TRY (get_type_name (AD_ARGS, type, base_name));
        *out_base_name = base_name ? ggc_strdup (base_name) : "<unknown>";

        // 提取模板参数
        int num_args = TREE_VEC_LENGTH (template_args_tree);
        AD_DEBUG_PRINT ("type=%s, num_args=%d", *out_base_name, num_args);

        vec<char const*, va_gc>* args = NULL;
        vec_alloc (args, num_args);

        for (int i = 0; i < num_args; i++) {
          tree arg = TREE_VEC_ELT (template_args_tree, i);
          char const* arg_str = "<unknown>";

          if (TYPE_P (arg)) {
            // 类型参数
            char const* arg_type_name = NULL;
            AD_TRY (get_type_name (AD_ARGS, arg, arg_type_name));
            arg_str = arg_type_name ? arg_type_name : "<unknown>";
            AD_DEBUG_PRINT ("  arg[%d] (type) = %s", i, arg_str);
          } else if (TREE_CODE (arg) == INTEGER_CST) {
            // 非类型模板参数（整数常量）
            char buf[64];
            snprintf (buf, sizeof(buf), "%lld", (long long)TREE_INT_CST_LOW (arg));
            arg_str = ggc_strdup (buf);
            AD_DEBUG_PRINT ("  arg[%d] (int) = %s", i, arg_str);
          } else {
            AD_DEBUG_PRINT ("  arg[%d] (unknown code=%d)", i, TREE_CODE (arg));
          }

          vec_safe_push (args, ggc_strdup (arg_str));
        }

        *out_template_args = args;
        AD_RETURNE (OK);
      }
    }
  }
  AD_DEBUG_PRINT ("cp-tree API not available or no template info");
#else
  AD_DEBUG_PRINT ("cp-tree macros not defined, trying demangling fallback");
#endif

  // 方案 2: 从 mangled name 使用 demangling 回退
  tree type_decl = TYPE_NAME (type);
  AD_DEBUG_PRINT ("TYPE_NAME -> %p (code=%d)",
                  (void*)type_decl, type_decl ? TREE_CODE (type_decl) : -1);
  if (type_decl && TREE_CODE (type_decl) == TYPE_DECL) {
    // 先尝试 DECL_ASSEMBLER_NAME（会触发 lazy 生成）
    tree assembler_name = DECL_ASSEMBLER_NAME (type_decl);
    AD_DEBUG_PRINT ("DECL_ASSEMBLER_NAME -> %p", (void*)assembler_name);
    if (assembler_name && TREE_CODE (assembler_name) == IDENTIFIER_NODE) {
      char const* mangled = IDENTIFIER_POINTER (assembler_name);
      AD_DEBUG_PRINT ("trying demangling: %s", mangled ? mangled : "(null)");
      if (mangled && mangled[0] != '\0') {
        // Demangle
        int status = 0;
        char* demangled = abi::__cxa_demangle (mangled, NULL, NULL, &status);
        AD_DEBUG_PRINT ("demangle status=%d, result=%s", status, demangled ? demangled : "(null)");
        if (status == 0 && demangled) {
          // 解析 demangled name
          AD_TRY (parseTemplateArgsFromDemangled (AD_ARGS, demangled, out_base_name, out_template_args));
          free (demangled);
          AD_RETURNE (OK);
        }
        if (demangled) free (demangled);
      }
    }
  }

  // 都失败了，使用简单类型名
  char const* simple_name = NULL;
  AD_TRY (get_type_name (AD_ARGS, type, simple_name));
  *out_base_name = simple_name ? simple_name : "<unknown>";
  AD_DEBUG_PRINT ("fallback to simple name: %s", *out_base_name);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 子函数：获取类型名（含命名空间和模版参数），返回格式化的字符串
// 返回：成功返回 OK，result 指向格式化的类型名字符串
ArrayDetectErrorCode formatTypeNameWithNamespace (AD_FUNC_ARGS, tree type, char const *&result) AD_FUNCTION_BEGIN {
  if (!type) {
    AD_RETURNO ("<unknown>");
  }

  char const *type_name = NULL;
  AD_TRY (get_type_name (AD_ARGS, type, type_name));
  if (!type_name) {
    AD_RETURNO ("<unknown>");
  }

  // 确保缓冲区可用
  if (!ctx.address_format_buffer || ctx.address_format_buffer_size == 0) {
    // 无缓冲区，回退到简单类型名
    AD_RETURNO (type_name);
  }

  // 获取模版参数（如果有），结果存入 ctx.escaped_string_buffer
  bool has_template_args = false;
  AD_TRY (formatTemplateArgs (AD_ARGS, type, has_template_args));

  // 尝试获取命名空间
  char const *ns_name = NULL;
  tree type_decl = TYPE_NAME (type);
  if (type_decl && TREE_CODE (type_decl) == TYPE_DECL) {
    tree context = DECL_CONTEXT (type_decl);
    if (context && TREE_CODE (context) == NAMESPACE_DECL && DECL_NAME (context)) {
      ns_name = IDENTIFIER_POINTER (DECL_NAME (context));
    }
  }

  // 组合命名空间、类型名和模版参数
  if (ns_name && has_template_args) {
    snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, "%s::%s%s",
              ns_name, type_name, ctx.escaped_string_buffer);
  } else if (ns_name) {
    snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, "%s::%s",
              ns_name, type_name);
  } else if (has_template_args) {
    snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, "%s%s",
              type_name, ctx.escaped_string_buffer);
  } else {
    // 无命名空间和模版参数，直接返回类型名
    AD_RETURNO (type_name);
  }

  AD_RETURNO (ctx.address_format_buffer);
} AD_FUNCTION_END

// 子函数：获取字段名
// 返回：成功返回 OK，result 指向字段名字符串
ArrayDetectErrorCode getFieldName (AD_FUNC_ARGS, tree field_decl, char const *&result) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!field_decl) {
    AD_RETURNO ("<unnamed>");
  }

  if (DECL_NAME (field_decl)) {
    AD_RETURNO (IDENTIFIER_POINTER (DECL_NAME (field_decl)));
  }
  AD_RETURNO ("<unnamed>");
} AD_FUNCTION_END

// 子函数：获取字段类型名（处理指针）
// 返回：成功返回 OK，result 指向格式化的字段类型名字符串
ArrayDetectErrorCode formatFieldTypeName (AD_FUNC_ARGS, tree field_type, char const *&result) AD_FUNCTION_BEGIN {
  if (!field_type) {
    AD_RETURNO ("<unknown>");
  }

  // 检查是否是单层指针
  bool is_pointer = (TREE_CODE (field_type) == POINTER_TYPE);
  tree base_type = is_pointer ? TREE_TYPE (field_type) : field_type;

  if (!base_type) {
    AD_RETURNO ("<unknown>");
  }

  char const *base_type_name = NULL;
  AD_TRY (get_type_name (AD_ARGS, base_type, base_type_name));
  if (!base_type_name) {
    AD_RETURNO ("<unknown>");
  }

  // 如果是指针，需要格式化输出（加上 *）
  if (is_pointer) {
    // 使用上下文缓冲区格式化指针类型名
    if (!ctx.address_format_buffer || ctx.address_format_buffer_size == 0) {
      AD_RETURNE (RESOURCE_ERROR);
    }
    snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, "%s*", base_type_name);
    AD_RETURNO (ctx.address_format_buffer);
  }
  AD_RETURNO (base_type_name);
} AD_FUNCTION_END

// 辅助函数：格式化包含模板参数的完整类型名
// 输出格式: "TypeName<Arg1, Arg2>" 或 "TypeName" (无模板参数时)
ArrayDetectErrorCode formatTypeNameWithTemplateArgs (AD_FUNC_ARGS, tree type, char const *&result) AD_FUNCTION_BEGIN {
  if (!type) {
    AD_RETURNO ("<null-type>");
  }

  // 提取模板参数
  char const* base_name = NULL;
  vec<char const*, va_gc>* template_args = NULL;
  AD_TRY (extractTemplateArgsFromType (AD_ARGS, type, &base_name, &template_args));

  if (!base_name) {
    // 回退到普通类型名
    char const* type_name = NULL;
    AD_TRY (formatTypeNameWithNamespace (AD_ARGS, type, type_name));
    AD_RETURNO (type_name ? type_name : "<unknown>");
  }

  // 如果没有模板参数，直接返回基础类型名
  if (!template_args || template_args->is_empty ()) {
    AD_RETURNO (base_name);
  }

  // 格式化带模板参数的类型名
  if (!ctx.address_format_buffer || ctx.address_format_buffer_size == 0) {
    AD_RETURNE (RESOURCE_ERROR);
  }

  // 构建格式化字符串: "TypeName<Arg1, Arg2, ...>"
  int offset = snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, "%s<", base_name);
  for (unsigned i = 0; i < template_args->length () && offset < (int)ctx.address_format_buffer_size - 2; i++) {
    if (i > 0) {
      offset += snprintf (ctx.address_format_buffer + offset, ctx.address_format_buffer_size - offset, ", ");
    }
    offset += snprintf (ctx.address_format_buffer + offset, ctx.address_format_buffer_size - offset, "%s", (*template_args)[i]);
  }
  snprintf (ctx.address_format_buffer + offset, ctx.address_format_buffer_size - offset, ">");

  // 使用 ggc_strdup 复制结果，避免 buffer 被后续调用覆盖
  AD_RETURNO (ggc_strdup (ctx.address_format_buffer));
} AD_FUNCTION_END

// 调试信息增强：打印字段写入捕获信息
// 包括：类型名（含模板参数）、字段名、字段类型名
ArrayDetectErrorCode logFieldWriteCapture (AD_FUNC_ARGS, tree containing_type, tree field_decl) AD_FUNCTION_BEGIN {
  // 获取类型名（含模板参数）
  char const * type_name = NULL;
  AD_TRY (formatTypeNameWithTemplateArgs (AD_ARGS, containing_type, type_name));
  
  // 获取字段名
  char const * field_name = NULL;
  AD_TRY (getFieldName (AD_ARGS, field_decl, field_name));
  
  // 获取字段类型名
  char const * field_type_name = NULL;
  tree field_type = field_decl ? TREE_TYPE (field_decl) : NULL_TREE;
  AD_TRY (formatFieldTypeName (AD_ARGS, field_type, field_type_name));
  
  AD_DEBUG_PRINT ("fieldWrite: %s::%s (%s)", type_name, field_name, field_type_name);
  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode analyze_gimple_assignment (AD_FUNC_ARGS, gimple * stmt, ::array_detector::ArrayDetector &detector, char const * func_name, tree func_decl) AD_FUNCTION_BEGIN {
  if (gimple_code (stmt) != GIMPLE_ASSIGN) {
    AD_RETURNE (OK);
  }

  tree lhs = gimple_assign_lhs (stmt);
  tree rhs = gimple_assign_rhs1 (stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  AD_TRY (is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));
  if (!is_field_access0) {
    AD_RETURNE (OK);
  }

  // 获取字段信息
  char const * field_name;
  AD_TRY (gcc_field_desc (AD_ARGS, field_decl, field_name));
  tree field_type = TREE_TYPE (field_decl);
  char const * field_type_name;
  AD_TRY (get_type_name (AD_ARGS, field_type, field_type_name));
  
  // 获取对象类型信息
  tree object_type = TREE_TYPE (object);
  char const * object_type_name;
  AD_TRY (get_type_name (AD_ARGS, object_type, object_type_name));

  // 使用gimple_assign_rhs_code获取RHS的树节点类型，并手动转换为字符串
  enum tree_code rhs_code = gimple_assign_rhs_code (stmt);
  char const * rhs_code_str = "UNKNOWN";
  
  // 使用switch语句转换为字符串表示
  switch (rhs_code) {
    case INTEGER_CST: rhs_code_str = "INTEGER_CST"; break;
    case REAL_CST: rhs_code_str = "REAL_CST"; break;
    case STRING_CST: rhs_code_str = "STRING_CST"; break;
    case SSA_NAME: rhs_code_str = "SSA_NAME"; break;
    case VAR_DECL: rhs_code_str = "VAR_DECL"; break;
    case PARM_DECL: rhs_code_str = "PARM_DECL"; break;
    case CALL_EXPR: rhs_code_str = "CALL_EXPR"; break;
    case COMPONENT_REF: rhs_code_str = "COMPONENT_REF"; break;
    case POINTER_PLUS_EXPR: rhs_code_str = "POINTER_PLUS_EXPR"; break;
    case PLUS_EXPR: rhs_code_str = "PLUS_EXPR"; break;
    case MINUS_EXPR: rhs_code_str = "MINUS_EXPR"; break;
    case MULT_EXPR: rhs_code_str = "MULT_EXPR"; break;
    case RDIV_EXPR: rhs_code_str = "RDIV_EXPR"; break;
    default: rhs_code_str = "UNKNOWN"; break;
  }
  (void)rhs_code_str;

  FieldInfo * field_info = NULL;

  size_t field_count = 0;
  ArrayDetectErrorCode count_err = array_detector::getFieldCount (detector, AD_ARGS, &field_count);
  if (count_err != OK) {
    ecode = count_err;
    AD_RETURN ();
  }

  for (size_t i = 0; i < field_count; i++) {
    FieldInfo * fi = nullptr;
    ArrayDetectErrorCode field_err = array_detector::getField (detector, AD_ARGS, i, &fi);
    if (field_err != OK || !fi) {
      continue;
    }
    if (fi->field_decl == field_decl) {
      field_info = fi;
      break;
    }
  }

  if (!field_info) {
    tree object_type = TREE_TYPE (object);
    if (object_type) {
      if (TREE_CODE (object_type) == REFERENCE_TYPE) object_type = TREE_TYPE (object_type);
      if (TREE_CODE (object_type) == POINTER_TYPE) object_type = TREE_TYPE (object_type);
      tree containing_type = TYPE_MAIN_VARIANT (object_type);
      if (containing_type) {
        hash_set<tree> temp_processed;
        temp_processed.create_ggc (0);
        ArrayDetectErrorCode err = process_type_fields (AD_ARGS, containing_type, detector, &temp_processed);
        if (err == array_detect_ns::OK) {
            size_t new_field_count = 0;
            ArrayDetectErrorCode new_count_err = array_detector::getFieldCount (detector, AD_ARGS, &new_field_count);
            if (new_count_err == OK) {
                for (size_t i = 0; i < new_field_count; i++) {
                   FieldInfo * fi = nullptr;
                   ArrayDetectErrorCode new_field_err = array_detector::getField (detector, AD_ARGS, i, &fi);
                   if (new_field_err != OK || !fi) {
                       continue;
                   }
                   if (fi->field_decl == field_decl) {
                     field_info = fi;
                     break;
                   }
                }
            }
        }
      }
    }
    if (!field_info) {
      AD_RETURNE (OK);
    }
  }

  char const * source = "UNKNOWN";
  bool is_call = false;
  // rhs_code已经在前面定义过
  char const * rhs_desc = "";

  if (rhs_code == INTEGER_CST) {
    source = "CONST";
    rhs_desc = "constant";
  } else if (rhs_code == SSA_NAME || rhs_code == VAR_DECL || rhs_code == PARM_DECL) {
     if (rhs_code == SSA_NAME) {
        gimple * def_stmt = SSA_NAME_DEF_STMT (rhs);
        if (def_stmt && is_gimple_call (def_stmt)) {
            AD_TRY (get_call_expr_name (AD_ARGS, gimple_call_fn (def_stmt), source));
            if (!source) source = "<unknown-call>";
            is_call = true;
            rhs_desc = "call result";
        } else {
          source = "VAR";
          rhs_desc = "variable";
        }
     } else {
       source = "VAR";
       rhs_desc = "variable";
     }
  } else if (get_gimple_rhs_class (rhs_code) == GIMPLE_BINARY_RHS) {
     source = "EXPR";
     rhs_desc = "expression";
  } else {
     source = "OTHER";
     rhs_desc = "other";
  }

  // 获取详细的源码位置信息
  AD_TRY (get_source_location_string (AD_ARGS, gimple_location (stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
  char const * loc_str = ctx.source_location_buffer;
  
  // 获取源码行内容
  get_source_line_content (gimple_location (stmt), ctx.source_line_buffer, ctx.source_line_buffer_size);
  char const * source_line = ctx.source_line_buffer;
 
  AssignmentDetail * detail = ggc_alloc<AssignmentDetail>();
  detail->source = source;
  detail->is_call = is_call;
  detail->tree_code = (int)rhs_code;
  detail->location_info = loc_str;
  detail->rhs_description = rhs_desc;

  if (!field_info->function_assignments) {
      field_info->function_assignments = ggc_alloc<vec<FunctionAssignment*>>();
      field_info->function_assignments->create (0);
  }
  
  FunctionAssignment * target_fa = NULL;
  for (unsigned i = 0; i < field_info->function_assignments->length (); ++i) {
      FunctionAssignment * fa = (*field_info->function_assignments)[i];
      // 使用 function_decl 比较更准确，或者 func_name
      if (fa->function_decl == func_decl) {
         target_fa = fa;
         break;
      }
  }

  if (!target_fa) {
      target_fa = ggc_alloc<FunctionAssignment>();
      target_fa->function_name = func_name;
      target_fa->function_decl = func_decl;
      target_fa->function_id = func_name; // 简单起见
      target_fa->assignment_count = 0;
      target_fa->sources = ggc_alloc<vec<char const *>>();
      target_fa->sources->create (0);
      target_fa->assignment_details = ggc_alloc<vec<AssignmentDetail*>>();
      target_fa->assignment_details->create (0);
      
      field_info->function_assignments->safe_push (target_fa);
  }

  target_fa->assignment_count++;
  target_fa->sources->safe_push (source);
  target_fa->assignment_details->safe_push (detail);
  
  // 更新 FieldInfo 级别的统计
  field_info->source_count++;
  if (!field_info->sources) {
      field_info->sources = ggc_alloc<vec<char const *>>();
      field_info->sources->create (0);
  }
  field_info->sources->safe_push (source);
  (void)source_line;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// 处理类型字段：提取类型的所有字段定义
// 语义：遍历类型的字段，创建 FieldInfo 对象并添加到 detector
// 垃圾回收：所有分配使用 ggc_alloc，由 GCC 自动管理
ArrayDetectErrorCode process_type_fields (AD_FUNC_ARGS, tree type, ::array_detector::ArrayDetector &detector, hash_set<tree> *processed_types) AD_FUNCTION_BEGIN {
  if (!type) {
    AD_RETURNE (OK);
  }
  
  // 只处理结构体/类类型
  if (TREE_CODE (type) != RECORD_TYPE && TREE_CODE (type) != UNION_TYPE) {
    AD_RETURNE (OK);
  }
  
  // 检查是否已处理过（避免重复处理）
  if (!processed_types->add (type)) {
    // 已处理过，跳过
    AD_RETURNE (OK);
  }
  
  char const * type_name;
  AD_TRY (gcc_ext_util::get_type_name (AD_ARGS, type, type_name));

  // 遍历类型的所有字段
  tree field;
  for (field = TYPE_FIELDS (type); field; field = DECL_CHAIN (field)) {
    if (TREE_CODE (field) != FIELD_DECL) continue;
    
    // 跳过编译器生成的字段（如虚表指针）
    bool is_compiler_generated;
    AD_TRY (isCompilerGeneratedField (AD_ARGS, field, is_compiler_generated));
    if (is_compiler_generated) {
      continue;
    }

    char const *field_name;
    AD_TRY (gcc_field_desc (AD_ARGS, field, field_name));
    
    tree field_type = TREE_TYPE (field);
    bool is_ptr;
    AD_TRY (is_pointer_type (AD_ARGS, field_type, is_ptr));

    // 创建字段信息结构体（使用 GCC 垃圾回收分配）
    // 语义：分配 FieldInfo 结构体，由 GCC 自动管理生命周期
    FieldInfo * field_info = ggc_alloc<FieldInfo>();
    if (!field_info) {
      AD_RETURNE (MEMORY_ERROR);
    }
    // 初始化字段为零
    memset (field_info, 0, sizeof (FieldInfo));
    
    // 设置字段基本信息
    field_info->field_name = field_name;
    field_info->containing_type = type_name;
    field_info->field_decl = field;
    field_info->containing_type_tree = type;
    field_info->is_pointer = is_ptr;
    field_info->is_array_candidate = false;
    field_info->source_count = 0;
    
    // 分配 vec 容器（使用 GCC 垃圾回收）
    // 语义：为字段分配三个 vec 容器，用于存储赋值信息
    field_info->sources = ggc_alloc<vec<char const *>>();
    field_info->sources->create (0);
    
    field_info->conflicting_assigns = ggc_alloc<vec<char const *>>();
    field_info->conflicting_assigns->create (0);
    
    field_info->function_assignments = ggc_alloc<vec<FunctionAssignment*>>();
    field_info->function_assignments->create (0);
    
    // 添加字段到 detector
    // 语义：将字段信息添加到全局字段列表
    AD_TRY_LABEL (addField (detector, AD_ARGS, field_info), field_init_error);
    continue;

    field_init_error:
    // 错误处理：释放已分配的 vec 容器
    // 注意：FieldInfo 本身由 GCC 管理，不需要 delete
    if (field_info->sources) {
      field_info->sources->release ();
    }
    if (field_info->conflicting_assigns) {
      field_info->conflicting_assigns->release ();
    }
    if (field_info->function_assignments) {
      field_info->function_assignments->release ();
    }
    AD_RETURN ();
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 检查类型是否是指针类型
ArrayDetectErrorCode is_pointer_type (AD_FUNC_ARGS, tree type, bool &result) AD_FUNCTION_BEGIN {
  if (!type) AD_RETURNO (false);
  AD_RETURNO (TREE_CODE (type) == POINTER_TYPE);
} AD_FUNCTION_END

// 检查是否是字段访问（COMPONENT_REF）
ArrayDetectErrorCode is_field_access (AD_FUNC_ARGS, tree expr, tree * field_decl_out, tree * object_out, bool &result) AD_FUNCTION_BEGIN {
  if (!expr) AD_RETURNO (false);

  if (TREE_CODE (expr) == COMPONENT_REF) {
    *field_decl_out = TREE_OPERAND (expr, 1);
    *object_out = TREE_OPERAND (expr, 0);
    AD_RETURNO (true);
  }
  AD_RETURNO (false);
} AD_FUNCTION_END

// 检测是否是编译器生成的字段（如虚表指针 vptr）
// 使用 DECL_ARTIFICIAL 检测，不依赖字段名硬编码
ArrayDetectErrorCode isCompilerGeneratedField (AD_FUNC_ARGS, tree field_decl, bool &result) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;

  if (!field_decl) {
    AD_RETURNO (false);
  }

  // DECL_ARTIFICIAL 标记由编译器自动生成的声明
  // 包括虚表指针（vptr）、虚基类指针等
  AD_RETURNO (DECL_ARTIFICIAL (field_decl) != 0);
} AD_FUNCTION_END
}
