#include "gcc-ext-util.hh"

#include "info.hh"

namespace gcc_ext_util {
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

// 获取类型名称
CutieErrorCode get_type_name (CUTIE_FUNC_ARGS, tree type, char const *&result) CUTIE_FUNCTION_BEGIN {
  if (!type) CUTIE_RETURNS("<unknown>");
  
  if (TYPE_NAME(type)) {
    if (TREE_CODE(TYPE_NAME(type)) == IDENTIFIER_NODE) {
      CUTIE_RETURNS(IDENTIFIER_POINTER(TYPE_NAME(type)));
    } else if (TREE_CODE(TYPE_NAME(type)) == TYPE_DECL) {
      tree name = DECL_NAME(TYPE_NAME(type));
      if (name) {
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

  tree lhs = gimple_assign_lhs(stmt);
  tree rhs = gimple_assign_rhs1(stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  CUTIE_TRY (is_field_access(CUTIE_ARGS, lhs, &field_decl, &object, is_field_access0));
  if (!is_field_access0) {
    CUTIE_RETURNV(OK);
  }

  FieldInfo* field_info = NULL;
  for (size_t i = 0; i < get_field_count(*detector); i++) {
    FieldInfo* fi = get_field(*detector, i);
    if (fi && fi->field_decl == field_decl) {
      field_info = fi;
      break;
    }
  }

  if (!field_info) {
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
            for (size_t i = 0; i < get_field_count(*detector); i++) {
               FieldInfo* fi = get_field(*detector, i);
               if (fi && fi->field_decl == field_decl) {
                 field_info = fi;
                 break;
               }
            }
        }
      }
    }
    if (!field_info) CUTIE_RETURNV(OK);
  }

  const char* source = "UNKNOWN";
  bool is_call = false;
  enum tree_code rhs_code = gimple_assign_rhs_code(stmt);
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

  char loc_buf[256];
  expanded_location xloc = expand_location(gimple_location(stmt));
  if (xloc.file) {
      snprintf(loc_buf, sizeof(loc_buf), "%s:%d", xloc.file, xloc.line);
  } else {
      strcpy(loc_buf, "<unknown location>");
  }
  const char* loc_str = ggc_strdup(loc_buf);
 
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

  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

CutieErrorCode process_type_fields(CUTIE_FUNC_ARGS, tree type, ArrayDetector* detector, hash_set<tree>* processed_types) CUTIE_FUNCTION_BEGIN {
  if (!type) {
    CUTIE_RETURNV(OK);
  }
  
  // 只处理结构体/类类型
  if (TREE_CODE(type) != RECORD_TYPE && TREE_CODE(type) != UNION_TYPE) {
    CUTIE_RETURNV(OK);
  }
  
  // 检查是否已处理过
  if (!processed_types->add(type)) {
    // 已处理过，跳过
    CUTIE_RETURNV(OK);
  }
  
  const char* type_name;
  CUTIE_TRY (gcc_ext_util::get_type_name(CUTIE_ARGS, type, type_name));
  CUTIE_DEBUG_PRINT("Processing type: %s", type_name);
  
  // 遍历字段
  tree field;
  for (field = TYPE_FIELDS(type); field; field = DECL_CHAIN(field)) {
    if (TREE_CODE(field) != FIELD_DECL) continue;
    
    const char* field_name;
    CUTIE_TRY (gcc_field_desc(CUTIE_ARGS, field, field_name));
    
    // 跳过虚函数表指针
    if (strstr(field_name, "_vptr") != NULL) {
      continue;
    }
    
    tree field_type = TREE_TYPE(field);
    bool is_ptr;
    CUTIE_TRY (is_pointer_type (CUTIE_ARGS, field_type, is_ptr));
    
    CUTIE_DEBUG_PRINT("  Field: %s, is_pointer: %d", field_name, is_ptr ? 1 : 0);
    
    // 创建字段信息（使用GCC的内存分配）
    FieldInfo* field_info = (FieldInfo*)ggc_alloc<FieldInfo>();
    if (!field_info) {
      CUTIE_RETURNV(MEMORY_ERROR);
    }
    // 初始化字段
    memset(field_info, 0, sizeof(FieldInfo));
    
    field_info->field_name = field_name;
    field_info->containing_type = type_name;
    field_info->field_decl = field;
    field_info->containing_type_tree = type;
    field_info->is_pointer = is_ptr;
        field_info->is_array_candidate = false;
        field_info->source_count = 0;
        field_info->sources = new vec<const char*>();
        field_info->sources->create(0);
        field_info->conflicting_assigns = new vec<const char*>();
        field_info->conflicting_assigns->create(0);
        field_info->function_assignments = new vec<FunctionAssignment*>();
        field_info->function_assignments->create(0);
    
    CUTIE_TRY_LABEL (add_field (*detector, CUTIE_ARGS, field_info), field_init_error);
    continue;

    field_init_error:
    if (field_info->sources) {
      field_info->sources->release();
      delete field_info->sources;
    }
    if (field_info->conflicting_assigns) {
      field_info->conflicting_assigns->release();
      delete field_info->conflicting_assigns;
    }
    if (field_info->function_assignments) {
      field_info->function_assignments->release();
      delete field_info->function_assignments;
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

} // namespace gcc_ext_util
