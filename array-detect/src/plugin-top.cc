#include "gcc-common.hh"
#include "plugin-version.h"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "array-detector.hh"
#include "array-detector-op0.hh"

// ----------------------------------------------------------------------------
// 字段信息结构
// ----------------------------------------------------------------------------
#include "info.hh"

// ----------------------------------------------------------------------------
// print_results 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------
#include "info-print.hh"

// ----------------------------------------------------------------------------
// 辅助函数
// ----------------------------------------------------------------------------

// 获取类型名称
static const char* get_type_name(tree type) {
  if (!type) return "<unknown>";
  
  if (TYPE_NAME(type)) {
    if (TREE_CODE(TYPE_NAME(type)) == IDENTIFIER_NODE) {
      return IDENTIFIER_POINTER(TYPE_NAME(type));
    } else if (TREE_CODE(TYPE_NAME(type)) == TYPE_DECL) {
      tree name = DECL_NAME(TYPE_NAME(type));
      if (name) {
        return IDENTIFIER_POINTER(name);
      }
    }
  }
  
  return "<unnamed>";
}

// 获取字段名称
namespace {
char const *gcc_field_desc(tree field) {
  if (!field) {
    return "<null>";
  }
  if (DECL_NAME(field)) {
    return IDENTIFIER_POINTER(DECL_NAME(field));
  }
  return "<unnamed>";
}
}

// 检查类型是否是指针类型
static bool is_pointer_type(tree type) {
  if (!type) return false;
  return TREE_CODE(type) == POINTER_TYPE;
}

// 检查是否是字段访问（COMPONENT_REF）
static bool is_field_access(tree expr, tree* field_decl_out, tree* object_out) {
  if (!expr) return false;
  
  if (TREE_CODE(expr) == COMPONENT_REF) {
    *field_decl_out = TREE_OPERAND(expr, 1);
    *object_out = TREE_OPERAND(expr, 0);
    return true;
  }
  return false;
}

// 检查是否是函数调用表达式（未使用，但保留以备将来使用）
// static bool is_function_call(tree expr) {
//   if (!expr) return false;
//   return TREE_CODE(expr) == CALL_EXPR;
// }

// ----------------------------------------------------------------------------
// 业务逻辑函数
// ----------------------------------------------------------------------------

// 辅助函数：处理一个类型，提取其字段
namespace cutie_ns {
namespace {
CutieErrorCode process_type_fields(CUTIE_FUNC_ARGS, tree type, ArrayDetector* detector, hash_set<tree>* processed_types) CUTIE_FUNCTION_BEGIN {
  if (!type) {
    ecode = cutie_ns::OK;
    CUTIE_RETURN;
  }
  
  // 只处理结构体/类类型
  if (TREE_CODE(type) != RECORD_TYPE && TREE_CODE(type) != UNION_TYPE) {
    ecode = cutie_ns::OK;
    CUTIE_RETURN;
  }
  
  // 检查是否已处理过
  if (!processed_types->add(type)) {
    // 已处理过，跳过
    ecode = cutie_ns::OK;
    CUTIE_RETURN;
  }
  
  const char* type_name = get_type_name(type);
  CUTIE_DEBUG_PRINT("Processing type: %s", type_name);
  
  // 遍历字段
  tree field;
  for (field = TYPE_FIELDS(type); field; field = DECL_CHAIN(field)) {
    if (TREE_CODE(field) != FIELD_DECL) continue;
    
    const char* field_name = gcc_field_desc(field);
    
    // 跳过虚函数表指针
    if (strstr(field_name, "_vptr") != NULL) {
      continue;
    }
    
    tree field_type = TREE_TYPE(field);
    bool is_ptr = is_pointer_type(field_type);
    
    CUTIE_DEBUG_PRINT("  Field: %s, is_pointer: %d", field_name, is_ptr ? 1 : 0);
    
    // 创建字段信息（使用GCC的内存分配）
    FieldInfo* field_info = (FieldInfo*)ggc_alloc<FieldInfo>();
    if (!field_info) {
      ecode = cutie_ns::MEMORY_ERROR;
      CUTIE_RETURN;
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
    
    CUTIE_TRY_LABEL(add_field(*detector, CUTIE_ARGS, field_info), field_init_error);
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
    CUTIE_RETURN;
  }
  
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END
}
} // namespace cutie_ns

// 收集所有类型和字段
namespace cutie_ns {
CutieErrorCode collect_all_types_and_fields(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Collecting all types and fields");
  
  // 使用hash_set来避免重复处理同一类型
  hash_set<tree> processed_types;
  processed_types.create_ggc(0);
  
  // 方法：遍历所有函数，从函数体中的字段访问提取类型
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun();
    if (!fn) continue;
    
    // 遍历函数中的语句，查找字段访问
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        
        // 检查赋值语句中的类型
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          tree lhs = gimple_assign_lhs(stmt);
          
          // 检查是否是字段访问
          tree field_decl = NULL_TREE;
          tree object = NULL_TREE;
          if (is_field_access(lhs, &field_decl, &object)) {
            // 找到字段访问，获取包含类型
            tree object_type = TREE_TYPE(object);
            if (!object_type) continue;
            
            // 如果是引用类型，获取其基础类型
            if (TREE_CODE(object_type) == REFERENCE_TYPE) {
              object_type = TREE_TYPE(object_type);
            }
            
            // 如果是指针类型，获取其指向的类型
            if (TREE_CODE(object_type) == POINTER_TYPE) {
              object_type = TREE_TYPE(object_type);
            }
            
            tree containing_type = TYPE_MAIN_VARIANT(object_type);
            if (containing_type) {
              CUTIE_TRY (process_type_fields(CUTIE_ARGS, containing_type, detector, &processed_types));
            }
          }
        }
      }
    }
  }
  
  // hash_set使用GCC的垃圾回收，不需要显式释放
  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END
} // namespace cutie_ns

// 分析字段赋值
namespace cutie_ns {

namespace {
const char* get_call_expr_name(tree call_expr) {
  if (TREE_CODE(call_expr) != CALL_EXPR) return NULL;
  tree fn = TREE_OPERAND(call_expr, 0);
  if (!fn) return "<call-expr>";
  
  if (TREE_CODE(fn) == FUNCTION_DECL && DECL_NAME(fn)) {
    return IDENTIFIER_POINTER(DECL_NAME(fn));
  }
  if (TREE_CODE(fn) == ADDR_EXPR) {
    tree decl = TREE_OPERAND(fn, 0);
    if (decl && DECL_NAME(decl)) {
      return IDENTIFIER_POINTER(DECL_NAME(decl));
    }
  }
  if (TREE_CODE(fn) == INDIRECT_REF) {
    return "<indirect-call>";
  }
  return "<call-expr>";
}
}

namespace {
CutieErrorCode analyze_gimple_assignment(CUTIE_FUNC_ARGS, gimple* stmt, ArrayDetector* detector, const char* func_name, tree func_decl) CUTIE_FUNCTION_BEGIN {
  if (gimple_code(stmt) != GIMPLE_ASSIGN) {
    CUTIE_RETURN;
  }

  tree lhs = gimple_assign_lhs(stmt);
  tree rhs = gimple_assign_rhs1(stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  if (!is_field_access(lhs, &field_decl, &object)) {
    CUTIE_RETURN;
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
    if (!field_info) CUTIE_RETURN;
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
            source = get_call_expr_name(gimple_call_fn(def_stmt));
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

  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END
}

} // namespace cutie_ns

// 已经在文件开头定义了这些类型别名

// ----------------------------------------------------------------------------
// Pass 注册结构
// ----------------------------------------------------------------------------

namespace {

const pass_data array_detect_pass_data = {
  .type = IPA_PASS,
  .name = "array-detect-wpa-pass",
  .optinfo_flags = OPTGROUP_NONE,
  .tv_id = TV_NONE,
  .properties_required = 0,
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0,
};

class pass_array_detect : public ipa_opt_pass_d {
 public:
  pass_array_detect(gcc::context* ctxt)
      : ipa_opt_pass_d(array_detect_pass_data, ctxt,
                       NULL,  // generate_summary
                       NULL,  // write_summary
                       NULL,  // read_summary
                       NULL,  // write_optimization_summary
                       NULL,  // read_optimization_summary
                       NULL,  // stmt_fixup
                       0,     // function_transform_todo_flags_start
                       NULL,  // function_transform
                       NULL)  // variable_transform
  {}

  opt_pass* clone() override { return new pass_array_detect(g); }

  unsigned int execute(function*) override {
    // Regular IPA passes in WPA mode call execute() with NULL function
    return array_detect_execute (cutie_ns::g_cutie_ctx) != ::cutie_ns::OK;
  }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// 插件初始化
// ----------------------------------------------------------------------------

int plugin_init(struct plugin_name_args* plugin_info,
                struct plugin_gcc_version* version) {
  // 版本检查
  if (!plugin_default_version_check(version, &gcc_version)) {
    return 1;
  }

  struct register_pass_info pass_info;
  pass_info.pass = new pass_array_detect(g);
  pass_info.reference_pass_name = "cdtor";        // 在此 pass 之后插入
  pass_info.ref_pass_instance_number = 1;         // 符合规则
  pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback(plugin_info->base_name,
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,
                    &pass_info);

  return 0;
}

int plugin_is_GPL_compatible = 1;
