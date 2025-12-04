#include "gcc-common.hh"
#include "plugin-version.h"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "array-detector.hh"
#include "array-detector-op0.hh"

#include "gcc-ext-util.hh"

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

// 检查是否是函数调用表达式（未使用，但保留以备将来使用）
// static bool is_function_call(tree expr) {
//   if (!expr) return false;
//   return TREE_CODE(expr) == CALL_EXPR;
// }

// ----------------------------------------------------------------------------
// 业务逻辑函数
// ----------------------------------------------------------------------------

// 收集所有类型和字段
namespace cutie_ns {
CutieErrorCode collect_all_types_and_fields(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Collecting all types and fields");
  
  // 使用hash_set来避免重复处理同一类型
  hash_set<tree> processed_types;
  processed_types.create_ggc(0);
  
  // 方法：遍历所有函数，从函数体中的字段访问提取类型
  struct cgraph_node* node;
  size_t func_count = 0;
  size_t field_access_count = 0;
  
  CUTIE_DEBUG_PRINT("Starting function traversal...");
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun();
    if (!fn) continue;
    
    func_count++;
    // 获取函数名称
    const char* func_name = node->name();
    tree decl = node->decl;
    if (decl && DECL_NAME(decl)) {
      func_name = IDENTIFIER_POINTER(DECL_NAME(decl));
    }
    CUTIE_DEBUG_PRINT("Processing function %zu: %s", func_count, func_name);
    
    // 遍历函数中的语句，查找字段访问
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      CUTIE_DEBUG_PRINT("  Processing basic block %d", bb->index);
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        
        // 检查赋值语句中的类型
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          CUTIE_DEBUG_PRINT("  Found assignment statement");
          // 打印具体的赋值语句代码
          fprintf(ctx.debug_file, "  Assignment statement code:\n");
          print_gimple_stmt(ctx.debug_file, stmt, 4, TDF_DETAILS);
          tree lhs = gimple_assign_lhs(stmt);
          
          // 检查是否是字段访问
          tree field_decl = NULL_TREE;
          tree object = NULL_TREE;
          bool is_field_access0;
          CUTIE_TRY (gcc_ext_util::is_field_access (CUTIE_ARGS, lhs, &field_decl, &object, is_field_access0));
          if (is_field_access0) {
            field_access_count++;
            CUTIE_DEBUG_PRINT("  Found field access (total: %zu)", field_access_count);
            
            // 找到字段访问，获取包含类型
            tree object_type = TREE_TYPE(object);
            if (!object_type) continue;
            
            // 如果是引用类型，获取其基础类型
            if (TREE_CODE(object_type) == REFERENCE_TYPE) {
              object_type = TREE_TYPE(object_type);
              CUTIE_DEBUG_PRINT("  Resolved reference type to: %s", TREE_CODE(object_type) == RECORD_TYPE ? "RECORD_TYPE" : 
                                                                 TREE_CODE(object_type) == UNION_TYPE ? "UNION_TYPE" :
                                                                 TREE_CODE(object_type) == POINTER_TYPE ? "POINTER_TYPE" :
                                                                 "OTHER_TYPE");
            }
            
            // 如果是指针类型，获取其指向的类型
            if (TREE_CODE(object_type) == POINTER_TYPE) {
              object_type = TREE_TYPE(object_type);
              CUTIE_DEBUG_PRINT("  Resolved pointer type to: %s", TREE_CODE(object_type) == RECORD_TYPE ? "RECORD_TYPE" : 
                                                                 TREE_CODE(object_type) == UNION_TYPE ? "UNION_TYPE" :
                                                                 "OTHER_TYPE");
            }
            
            tree containing_type = TYPE_MAIN_VARIANT(object_type);
            if (containing_type) {
              const char* type_name;
              CUTIE_TRY(gcc_ext_util::get_type_name(CUTIE_ARGS, containing_type, type_name));
              CUTIE_DEBUG_PRINT("  Processing type: %s", type_name ? type_name : "<unknown>");
              
              CUTIE_TRY (gcc_ext_util::process_type_fields (CUTIE_ARGS, containing_type, detector, &processed_types));
            }
          }
        }
      }
    }
  }
  
  CUTIE_DEBUG_PRINT("Collection complete: %zu functions processed, %zu field accesses found", func_count, field_access_count);
  
  // hash_set使用GCC的垃圾回收，不需要显式释放
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END
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
