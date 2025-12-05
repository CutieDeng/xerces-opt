#include "gcc-ext-util.hh"

#include "info.hh"

namespace gcc_ext_util {

// 安全字符串复制函数，防止缓冲区溢出
void safe_string_copy(char* dest, size_t dest_size, const char* src) {
  if (dest_size == 0) return;
  if (!src) {
    dest[0] = '\0';
    return;
  }
  
  // 使用strncpy确保不会溢出，并确保字符串以null结尾
  strncpy(dest, src, dest_size - 1);
  dest[dest_size - 1] = '\0';
}

// 新增辅助函数：获取详细的源码位置信息
// 使用预分配的缓冲区，避免动态内存分配
CutieErrorCode get_source_location_string(CUTIE_FUNC_ARGS, location_t loc, char* buffer, size_t buffer_size) CUTIE_FUNCTION_BEGIN {
  if (loc == UNKNOWN_LOCATION) {
    snprintf(buffer, buffer_size, "<unknown location>");
    CUTIE_RETURNV (OK);
  }
  
  expanded_location xloc = expand_location(loc);
  
  if (xloc.file) {
    // 输出格式改为 a.cc +linenumber colnumber 的形式
    if (xloc.line > 0) {
      if (xloc.column > 0) {
        snprintf(buffer, buffer_size, "%s +%d %d", xloc.file, xloc.line, xloc.column);
      } else {
        snprintf(buffer, buffer_size, "%s +%d", xloc.file, xloc.line);
      }
    } else {
      snprintf(buffer, buffer_size, "%s", xloc.file);
    }
  } else {
    snprintf(buffer, buffer_size, "<unknown location>");
  }
  CUTIE_RETURNV (OK);
} CUTIE_FUNCTION_END

// 新增辅助函数：获取指定位置的源码行内容
// 使用预分配的缓冲区，避免动态内存分配
void get_source_line_content(location_t loc, char* buffer, size_t buffer_size) {
  if (loc == UNKNOWN_LOCATION) {
    snprintf(buffer, buffer_size, "&lt;source line content not available&gt;");
    return;
  }
  
  expanded_location xloc = expand_location(loc);
  
  if (xloc.file && xloc.line > 0) {
    // 尝试读取源代码文件的指定行
    FILE* file = fopen(xloc.file, "r");
    if (file) {
      char line_buffer[1024];
      size_t current_line = 0;
      while (fgets(line_buffer, sizeof(line_buffer), file) && current_line < ((size_t) xloc.line)) {
        current_line++;
        if (current_line == ((size_t) xloc.line)) {
          // 移除行尾的换行符
          size_t len = strlen(line_buffer);
          if (len > 0 && line_buffer[len-1] == '\n') {
            line_buffer[len-1] = '\0';
          }
          // 复制到输出缓冲区，注意不要溢出
          strncpy(buffer, line_buffer, buffer_size - 1);
          buffer[buffer_size - 1] = '\0';
          fclose(file);
          return;
        }
      }
      fclose(file);
    }
  }
  
  // 如果无法读取源代码行，则返回默认值
  snprintf(buffer, buffer_size, "<source line content not available>");
}

namespace {

CutieErrorCode get_call_expr_name(CUTIE_FUNC_ARGS, tree call_expr, char const *&result) CUTIE_FUNCTION_BEGIN {
  if (TREE_CODE(call_expr) != CALL_EXPR) {
    CUTIE_RETURNV(INVALID_ARGUMENT);
  }
  tree fn = TREE_OPERAND(call_expr, 0);
  if (!fn) {
    CUTIE_RETURNS("<call-expr>");
  }
  
  if (TREE_CODE(fn) == FUNCTION_DECL && DECL_NAME(fn)) {
    CUTIE_RETURNS(IDENTIFIER_POINTER(DECL_NAME(fn)));
  }
  if (TREE_CODE(fn) == ADDR_EXPR) {
    tree decl = TREE_OPERAND(fn, 0);
    if (decl && DECL_NAME(decl)) {
      CUTIE_RETURNS(IDENTIFIER_POINTER(DECL_NAME(decl)));
    }
  }
  if (TREE_CODE(fn) == INDIRECT_REF) {
    CUTIE_RETURNS("<indirect-call>");
  }
  CUTIE_RETURNS("<call-expr>");
} CUTIE_FUNCTION_END

CutieErrorCode gcc_field_desc(CUTIE_FUNC_ARGS, tree field, char const *&result) CUTIE_FUNCTION_BEGIN {
  if (!field) {
    CUTIE_RETURNS ("<null>");
  }
  if (DECL_NAME(field)) {
    CUTIE_RETURNS (IDENTIFIER_POINTER(DECL_NAME(field)));
  }
  CUTIE_RETURNS ("<unnamed>");
} CUTIE_FUNCTION_END

}
}

namespace gcc_ext_util {

// 获取类型名称：从 GCC tree 节点提取类型名称字符串
// 语义：返回类型的可读名称，用于调试和报告
// 垃圾回收：返回的指针指向 GCC 内部管理的字符串，无需释放
CutieErrorCode get_type_name (CUTIE_FUNC_ARGS, tree type, char const *&result) CUTIE_FUNCTION_BEGIN {
  if (!type) CUTIE_RETURNS("<unknown>");
  
  if (TYPE_NAME(type)) {
    if (TREE_CODE(TYPE_NAME(type)) == IDENTIFIER_NODE) {
      // 返回 GCC 内部管理的标识符指针
      CUTIE_RETURNS(IDENTIFIER_POINTER(TYPE_NAME(type)));
    } else if (TREE_CODE(TYPE_NAME(type)) == TYPE_DECL) {
      tree name = DECL_NAME(TYPE_NAME(type));
      if (name) {
        // 返回 GCC 内部管理的声明名称指针
        CUTIE_RETURNS(IDENTIFIER_POINTER(name));
      }
    }
  }
  CUTIE_RETURNS("<unnamed>");
} CUTIE_FUNCTION_END

CutieErrorCode analyze_gimple_assignment (CUTIE_FUNC_ARGS, gimple* stmt, ArrayDetector* detector, char const* func_name, tree func_decl) CUTIE_FUNCTION_BEGIN {
  if (gimple_code(stmt) != GIMPLE_ASSIGN) {
    CUTIE_RETURNV(OK);
  }

  CUTIE_DEBUG_PRINT("  Analyzing assignment statement:");
  tree lhs = gimple_assign_lhs(stmt);
  tree rhs = gimple_assign_rhs1(stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  CUTIE_TRY (is_field_access(CUTIE_ARGS, lhs, &field_decl, &object, is_field_access0));
  if (!is_field_access0) {
    CUTIE_DEBUG_PRINT("    Not a field access, skipping");
    CUTIE_RETURNV(OK);
  }

  // 获取字段信息
  const char* field_name;
  CUTIE_TRY (gcc_field_desc(CUTIE_ARGS, field_decl, field_name));
  tree field_type = TREE_TYPE(field_decl);
  const char* field_type_name;
  CUTIE_TRY (get_type_name(CUTIE_ARGS, field_type, field_type_name));
  
  // 获取对象类型信息
  tree object_type = TREE_TYPE(object);
  const char* object_type_name;
  CUTIE_TRY (get_type_name(CUTIE_ARGS, object_type, object_type_name));
  
  CUTIE_DEBUG_PRINT("    Field: %s", field_name);
  CUTIE_DEBUG_PRINT("    Field type: %s", field_type_name);
  CUTIE_DEBUG_PRINT("    Object type: %s", object_type_name);
  // 使用gimple_assign_rhs_code获取RHS的树节点类型，并手动转换为字符串
  enum tree_code rhs_code = gimple_assign_rhs_code(stmt);
  const char* rhs_code_str = "UNKNOWN";
  
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
  CUTIE_DEBUG_PRINT("    RHS code: %s", rhs_code_str);

  FieldInfo* field_info = NULL;
  CUTIE_DEBUG_PRINT("    Looking for existing field info...");

  size_t field_count = 0;
  CutieErrorCode count_err = get_field_count(*detector, CUTIE_ARGS, &field_count);
  if (count_err != OK) {
    CUTIE_DEBUG_PRINT("    Failed to get field count");
    ecode = count_err;
    CUTIE_RETURNR;
  }

  for (size_t i = 0; i < field_count; i++) {
    FieldInfo* fi = nullptr;
    CutieErrorCode field_err = get_field(*detector, CUTIE_ARGS, i, &fi);
    if (field_err != OK || !fi) {
      CUTIE_DEBUG_PRINT("    Failed to get field, continuing...");
      continue;
    }
    if (fi->field_decl == field_decl) {
      field_info = fi;
      CUTIE_DEBUG_PRINT("    Found existing field info");
      break;
    }
  }

  if (!field_info) {
    CUTIE_DEBUG_PRINT("    Field info not found, trying to create...");
    tree object_type = TREE_TYPE(object);
    if (object_type) {
      if (TREE_CODE(object_type) == REFERENCE_TYPE) object_type = TREE_TYPE(object_type);
      if (TREE_CODE(object_type) == POINTER_TYPE) object_type = TREE_TYPE(object_type);
      tree containing_type = TYPE_MAIN_VARIANT(object_type);
      if (containing_type) {
        hash_set<tree> temp_processed;
        temp_processed.create_ggc(0);
        CutieErrorCode err = process_type_fields(CUTIE_ARGS, containing_type, detector, &temp_processed);
        if (err == cutie_ns::OK) {
            size_t new_field_count = 0;
            CutieErrorCode new_count_err = get_field_count(*detector, CUTIE_ARGS, &new_field_count);
            if (new_count_err != OK) {
                CUTIE_DEBUG_PRINT("    Failed to get new field count after processing type fields");
            } else {
                for (size_t i = 0; i < new_field_count; i++) {
                   FieldInfo* fi = nullptr;
                   CutieErrorCode new_field_err = get_field(*detector, CUTIE_ARGS, i, &fi);
                   if (new_field_err != OK || !fi) {
                       CUTIE_DEBUG_PRINT("    Failed to get new field, continuing...");
                       continue;
                   }
                   if (fi->field_decl == field_decl) {
                     field_info = fi;
                     CUTIE_DEBUG_PRINT("    Created new field info");
                     break;
                   }
                }
            }
        }
      }
    }
    if (!field_info) {
      CUTIE_DEBUG_PRINT("    Failed to find or create field info, skipping");
      CUTIE_RETURNV(OK);
    }
  }

  const char* source = "UNKNOWN";
  bool is_call = false;
  // rhs_code已经在前面定义过
  const char* rhs_desc = "";

  if (rhs_code == INTEGER_CST) {
    source = "CONST";
    rhs_desc = "constant";
  } else if (rhs_code == SSA_NAME || rhs_code == VAR_DECL || rhs_code == PARM_DECL) {
     if (rhs_code == SSA_NAME) {
        gimple* def_stmt = SSA_NAME_DEF_STMT(rhs);
        if (def_stmt && is_gimple_call(def_stmt)) {
            CUTIE_TRY (get_call_expr_name (CUTIE_ARGS, gimple_call_fn (def_stmt), source));
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
  } else if (get_gimple_rhs_class(rhs_code) == GIMPLE_BINARY_RHS) {
     source = "EXPR";
     rhs_desc = "expression";
  } else {
     source = "OTHER";
     rhs_desc = "other";
  }

  // 获取详细的源码位置信息
  CUTIE_TRY (get_source_location_string (CUTIE_ARGS, gimple_location(stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
  const char* loc_str = ctx.source_location_buffer;
  
  // 获取源码行内容
  get_source_line_content(gimple_location(stmt), ctx.source_line_buffer, ctx.source_line_buffer_size);
  const char* source_line = ctx.source_line_buffer;
 
  AssignmentDetail* detail = ggc_alloc<AssignmentDetail>();
  detail->source = source;
  detail->is_call = is_call;
  detail->tree_code = (int)rhs_code;
  detail->location_info = loc_str;
  detail->rhs_description = rhs_desc;

  if (!field_info->function_assignments) {
      field_info->function_assignments = ggc_alloc<vec<FunctionAssignment*>>();
      field_info->function_assignments->create(0);
  }
  
  FunctionAssignment* target_fa = NULL;
  for (unsigned i = 0; i < field_info->function_assignments->length(); ++i) {
      FunctionAssignment* fa = (*field_info->function_assignments)[i];
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
      target_fa->sources = ggc_alloc<vec<const char*>>();
      target_fa->sources->create(0);
      target_fa->assignment_details = ggc_alloc<vec<AssignmentDetail*>>();
      target_fa->assignment_details->create(0);
      
      field_info->function_assignments->safe_push(target_fa);
  }

  target_fa->assignment_count++;
  target_fa->sources->safe_push(source);
  target_fa->assignment_details->safe_push(detail);
  
  // 更新 FieldInfo 级别的统计
  field_info->source_count++;
  if (!field_info->sources) {
      field_info->sources = ggc_alloc<vec<const char*>>();
      field_info->sources->create(0);
  }
  field_info->sources->safe_push(source);
  
  CUTIE_DEBUG_PRINT("    Assignment processed successfully:");
  CUTIE_DEBUG_PRINT("      Field: %s::%s", field_info->containing_type, field_info->field_name);
  CUTIE_DEBUG_PRINT("      Assignment source: %s (%s)", source, rhs_desc);
  CUTIE_DEBUG_PRINT("      Location: %s", loc_str);
  CUTIE_DEBUG_PRINT("      Source line: %s", source_line);
  CUTIE_DEBUG_PRINT("      Total assignments for this field: %d", field_info->source_count);

  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 处理类型字段：提取类型的所有字段定义
// 语义：遍历类型的字段，创建 FieldInfo 对象并添加到 detector
// 垃圾回收：所有分配使用 ggc_alloc，由 GCC 自动管理
CutieErrorCode process_type_fields(CUTIE_FUNC_ARGS, tree type, ArrayDetector* detector, hash_set<tree>* processed_types) CUTIE_FUNCTION_BEGIN {
  if (!type) {
    CUTIE_RETURNV(OK);
  }
  
  // 只处理结构体/类类型
  if (TREE_CODE(type) != RECORD_TYPE && TREE_CODE(type) != UNION_TYPE) {
    CUTIE_RETURNV(OK);
  }
  
  // 检查是否已处理过（避免重复处理）
  if (!processed_types->add(type)) {
    // 已处理过，跳过
    CUTIE_RETURNV(OK);
  }
  
  const char* type_name;
  CUTIE_TRY (gcc_ext_util::get_type_name(CUTIE_ARGS, type, type_name));
  CUTIE_DEBUG_PRINT("Processing type: %s", type_name);
  
  // 遍历类型的所有字段
  tree field;
  for (field = TYPE_FIELDS(type); field; field = DECL_CHAIN(field)) {
    if (TREE_CODE(field) != FIELD_DECL) continue;
    
    const char* field_name;
    CUTIE_TRY (gcc_field_desc(CUTIE_ARGS, field, field_name));
    
    // 跳过虚函数表指针（编译器生成的内部字段）
    if (strstr(field_name, "_vptr") != NULL) {
      continue;
    }
    
    tree field_type = TREE_TYPE(field);
    bool is_ptr;
    CUTIE_TRY (is_pointer_type (CUTIE_ARGS, field_type, is_ptr));
    
    CUTIE_DEBUG_PRINT("  Field: %s, is_pointer: %d", field_name, is_ptr ? 1 : 0);
    
    // 创建字段信息结构体（使用 GCC 垃圾回收分配）
    // 语义：分配 FieldInfo 结构体，由 GCC 自动管理生命周期
    FieldInfo* field_info = ggc_alloc<FieldInfo>();
    if (!field_info) {
      CUTIE_RETURNV(MEMORY_ERROR);
    }
    // 初始化字段为零
    memset(field_info, 0, sizeof(FieldInfo));
    
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
    field_info->sources = ggc_alloc<vec<const char*>>();
    field_info->sources->create(0);
    
    field_info->conflicting_assigns = ggc_alloc<vec<const char*>>();
    field_info->conflicting_assigns->create(0);
    
    field_info->function_assignments = ggc_alloc<vec<FunctionAssignment*>>();
    field_info->function_assignments->create(0);
    
    // 添加字段到 detector
    // 语义：将字段信息添加到全局字段列表
    CUTIE_TRY_LABEL (add_field (*detector, CUTIE_ARGS, field_info), field_init_error);
    continue;

    field_init_error:
    // 错误处理：释放已分配的 vec 容器
    // 注意：FieldInfo 本身由 GCC 管理，不需要 delete
    if (field_info->sources) {
      field_info->sources->release();
    }
    if (field_info->conflicting_assigns) {
      field_info->conflicting_assigns->release();
    }
    if (field_info->function_assignments) {
      field_info->function_assignments->release();
    }
    CUTIE_RETURNR;
  }
  
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 检查类型是否是指针类型
CutieErrorCode is_pointer_type(CUTIE_FUNC_ARGS, tree type, bool &result) CUTIE_FUNCTION_BEGIN {
  if (!type) CUTIE_RETURNS (false);
  CUTIE_RETURNS (TREE_CODE(type) == POINTER_TYPE);
} CUTIE_FUNCTION_END

// 检查是否是字段访问（COMPONENT_REF）
CutieErrorCode is_field_access(CUTIE_FUNC_ARGS, tree expr, tree* field_decl_out, tree* object_out, bool &result) CUTIE_FUNCTION_BEGIN {
  if (!expr) CUTIE_RETURNS (false);
  
  if (TREE_CODE(expr) == COMPONENT_REF) {
    *field_decl_out = TREE_OPERAND(expr, 1);
    *object_out = TREE_OPERAND(expr, 0);
    CUTIE_RETURNS (true);
  }
  CUTIE_RETURNS (false);
} CUTIE_FUNCTION_END
}
