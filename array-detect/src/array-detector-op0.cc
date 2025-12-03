#include "prelude.hh"
#include "state.hh"
#include "array-detector-op0.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "context-init.hh"

namespace cutie_ns {
CutieErrorCode array_detect_execute (CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  CUTIE_TRY (initWithStderr(CUTIE_ARGS));
  
  CUTIE_TRY (array_detect_analysis (CUTIE_ARGS));
  
  ecode = ::cutie_ns::OK;
  cleanup:
  cutie_ns::deinit(CUTIE_ARGS);
} CUTIE_FUNCTION_END2

CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  // 重命名标签以避免与宏中的cleanup冲突
  CUTIE_DEBUG_PRINT("Starting array member detection analysis");
  
  // 创建数组检测器
  array_detector::ArrayDetector detector;
  
  // 执行分析
  CUTIE_TRY_LABEL(trace_field_assignments(CUTIE_ARGS, &detector), analysis_cleanup);
  
  CUTIE_TRY_LABEL(cutie_ns::print_results(CUTIE_ARGS, &detector), analysis_cleanup);
  
  analysis_cleanup:
  // 使用显式清理函数替代析构函数
  deinit(detector);
  
  CUTIE_DEBUG_PRINT("Array member detection analysis completed");
  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  // 追踪字段的赋值操作
  CUTIE_DEBUG_PRINT("Tracing field assignments");
  
  // 第一步：收集所有类型和字段
  CUTIE_TRY (collect_all_types_and_fields (CUTIE_ARGS, detector));
  
  // 第二步：分析字段赋值
  CUTIE_TRY (analyze_field_assignments_in_functions (*detector, CUTIE_ARGS));

  // 第三步：分析使用情况，判断是否是数组候选
  CUTIE_TRY (analyze_usage (*detector, CUTIE_ARGS));

  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

} // namespace cutie_ns

namespace array_detector {

CutieErrorCode analyze_field_assignments_in_functions(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Analyzing field assignments in functions");
  
  // 遍历所有函数
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun();
    if (!fn) continue;
    
    // 获取函数名称（尝试获取可读的名称）
    const char* func_name = node->name();
    tree decl = node->decl;
    if (decl && DECL_NAME(decl)) {
      func_name = IDENTIFIER_POINTER(DECL_NAME(decl));
    }
    // 如果还是空，使用mangled name
    if (!func_name || strlen(func_name) == 0) {
      func_name = node->name();
    }
    
    // 获取函数所属的类型（对于成员函数）
    const char* containing_type_name = NULL;
    if (decl) {
      tree context = DECL_CONTEXT(decl);
      if (context) {
        if (TREE_CODE(context) == RECORD_TYPE || TREE_CODE(context) == UNION_TYPE) {
          containing_type_name = get_type_name(context);
        } else if (TREE_CODE(context) == NAMESPACE_DECL) {
          // 命名空间中的函数
          if (DECL_NAME(context)) {
            containing_type_name = IDENTIFIER_POINTER(DECL_NAME(context));
          }
        }
      }
    }
    
    // 输出调试信息，包含函数所属类型
    if (containing_type_name) {
      CUTIE_DEBUG_PRINT("Analyzing function: %s::%s", containing_type_name, func_name);
    } else {
      CUTIE_DEBUG_PRINT("Analyzing function: %s", func_name);
    }
    
    // 遍历函数中的所有基本块
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        
        // 检查是否是赋值语句
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          analyze_gimple_assignment(CUTIE_ARGS, stmt, detector, func_name, decl);
        }
        // 检查是否是GIMPLE_CALL语句（可能是通过调用赋值）
        else if (gimple_code(stmt) == GIMPLE_CALL) {
          // 这里可以处理通过函数调用返回值的赋值
          // 简化处理：暂时跳过
        }
      }
    }
  }
  
  ecode = cutie_ns::OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

}
