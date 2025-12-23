#include "prelude.hh"
#include "state.hh"
#include "field-assignment-debugger.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "context-init.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// 调试功能：分析函数中的字段赋值模式
// 用于详细的调试输出，分析每个函数中的字段赋值情况
ArrayDetectErrorCode analyzeFieldAssignmentsInFunctions (ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing field assignments in functions");
  
  // 遍历所有函数
  struct cgraph_node * node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    function * fn = node->get_fun ();
    if (!fn) continue;
    
    // 获取函数名称（尝试获取可读的名称）
    char const * func_name = node->name ();
    tree decl = node->decl;
    if (decl && DECL_NAME (decl)) {
      func_name = IDENTIFIER_POINTER (DECL_NAME (decl));
    }
    // 如果还是空，使用mangled name
    if (!func_name || strlen (func_name) == 0) {
      func_name = node->name ();
    }
    
    // 获取函数所属的类型（对于成员函数）
    char const * containing_type_name = NULL;
    if (decl) {
      tree context = DECL_CONTEXT (decl);
      if (context) {
        if (TREE_CODE (context) == RECORD_TYPE || TREE_CODE (context) == UNION_TYPE) {
          AD_TRY (gcc_ext_util::get_type_name (AD_ARGS, context, containing_type_name));
        } else if (TREE_CODE (context) == NAMESPACE_DECL) {
          // 命名空间中的函数
          if (DECL_NAME (context)) {
            containing_type_name = IDENTIFIER_POINTER (DECL_NAME (context));
          }
        }
      }
    }
    
    // 输出调试信息，包含函数所属类型
    if (containing_type_name) {
      AD_DEBUG_PRINT ("Analyzing function: %s::%s", containing_type_name, func_name);
    } else {
      AD_DEBUG_PRINT ("Analyzing function: %s", func_name);
    }
    
    // 添加函数基本信息调试
    AD_DEBUG_PRINT ("Function has %d basic blocks", n_basic_blocks_for_fn (fn));
    
    // 遍历函数中的所有基本块
    basic_block bb;
    FOR_EACH_BB_FN (bb, fn) {
      AD_DEBUG_PRINT ("Processing basic block %d", bb->index);
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
        gimple * stmt = gsi_stmt (gsi);
        AD_DEBUG_PRINT ("  Statement type: %s", gimple_code_name[gimple_code (stmt)]);
        
        // 检查是否是赋值语句
        if (gimple_code (stmt) == GIMPLE_ASSIGN) {
          AD_DEBUG_PRINT ("  Found assignment statement, analyzing...");
          // 打印具体的赋值语句代码和位置信息
          AD_TRY (gcc_ext_util::get_source_location_string (AD_ARGS, gimple_location (stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
          fprintf (ctx.debug_file, "  Assignment statement at %s:\n", ctx.source_location_buffer);
          print_gimple_stmt (ctx.debug_file, stmt, 4, TDF_DETAILS);
          gcc_ext_util::analyze_gimple_assignment (AD_ARGS, stmt, detector, func_name, decl);
        }
        // 检查是否是GIMPLE_CALL语句（可能是通过调用赋值）
        else if (gimple_code (stmt) == GIMPLE_CALL) {
          AD_DEBUG_PRINT ("  Found call statement, skipping for now");
          // 这里可以处理通过函数调用返回值的赋值
          // 简化处理：暂时跳过
        }
        else {
          AD_DEBUG_PRINT ("  Skipping statement type: %s", gimple_code_name[gimple_code (stmt)]);
        }
      }
    }
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
