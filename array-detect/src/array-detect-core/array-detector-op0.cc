#include "prelude.hh"
#include "state.hh"
#include "array-detector-op0.hh"
#include "array-detector.hh"
#include "info-print.hh"
#include "context-init.hh"
#include "gcc-ext-util.hh"

namespace cutie_ns {

// 直接执行分析：接收已初始化的 ArrayDetector 对象
// 语义：跳过对象创建，直接执行分析流程
// 调用者负责：对象的创建和初始化
// 本函数负责：分析执行和清理
CutieErrorCode array_detect_execute_with_detector(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Starting array member detection with provided detector");
  
  // 检查 detector 是否有效
  if (detector == nullptr) {
    CUTIE_DEBUG_PRINT("Error: detector is null");
    CUTIE_RETURNV(LOGICAL_ERROR);
  }
  
  // 第一步：收集所有类型和字段
  // 语义：遍历所有函数，提取类型和字段信息
  CUTIE_TRY_LABEL(collect_all_types_and_fields(CUTIE_ARGS, detector), analysis_cleanup);
  
  // 第二步：追踪字段赋值
  // 语义：分析字段的赋值来源，判断是否为 owned 数组
  CUTIE_TRY_LABEL(trace_field_assignments(CUTIE_ARGS, detector), analysis_cleanup);
  
  // 第三步：输出分析结果
  // 语义：生成并输出分析报告
  CUTIE_TRY_LABEL(cutie_ns::print_results(CUTIE_ARGS, detector), analysis_cleanup);
  
  analysis_cleanup:
  CUTIE_DEBUG_PRINT("Array member detection completed");
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 原始分析入口（保留以兼容）
// 语义：创建 ArrayDetector 对象并执行分析
CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Starting array member detection analysis");
  
  // 创建数组检测器对象（在栈上）
  // 语义：分配 ArrayDetector 结构体，初始化为 nullptr
  array_detector::ArrayDetector detector;
  
  // 延迟初始化：在使用前分配 vec 指针
  // 语义：分配 GCC 管理的 vec 容器，用于存储字段信息
  CUTIE_TRY_LABEL(array_detector::init(detector, CUTIE_ARGS), analysis_cleanup);
  
  // 执行分析流程
  // 语义：调用优化后的执行函数
  CUTIE_TRY_LABEL(array_detect_execute_with_detector(CUTIE_ARGS, &detector), analysis_cleanup);
  
  analysis_cleanup:
  // 清理资源
  // 语义：释放 vec 容器，设置指针为 nullptr
  array_detector::deinit(detector, CUTIE_ARGS);
  
  CUTIE_DEBUG_PRINT("Array member detection analysis completed");
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 字段赋值追踪：分析字段赋值来源
// 语义：执行两步分析 - 赋值分析 -> 候选判断
// 前置条件：字段已通过 collect_all_types_and_fields 收集
CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Tracing field assignments");
  
  // 检查 detector 是否有效
  if (detector == nullptr) {
    CUTIE_DEBUG_PRINT("Error: detector is null");
    CUTIE_RETURNV(LOGICAL_ERROR);
  }
  
  // 第一步：分析字段赋值
  // 语义：遍历所有函数，追踪字段的赋值操作和来源
  CUTIE_TRY(analyze_field_assignments_in_functions(*detector, CUTIE_ARGS));

  // 第二步：分析使用情况，判断是否是数组候选
  // 语义：根据赋值来源判断字段是否为 owned 数组
  CUTIE_TRY(analyze_usage(*detector, CUTIE_ARGS));

  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 收集所有类型和字段：遍历编译单元提取类型信息
// 语义：扫描所有函数，从字段访问中提取类型和字段定义
// 输出：填充 detector->m_fields 容器
// 垃圾回收：使用 ggc_alloc 分配的 hash_set 由 GCC 自动管理
CutieErrorCode collect_all_types_and_fields(CUTIE_FUNC_ARGS, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Collecting all types and fields");
  
  // 检查 detector 是否有效
  if (detector == nullptr) {
    CUTIE_DEBUG_PRINT("Error: detector is null");
    CUTIE_RETURNV(LOGICAL_ERROR);
  }
  
  // 使用 hash_set 避免重复处理同一类型
  // 语义：维护已处理类型集合，防止重复分析
  // 垃圾回收：create_ggc(0) 使用 GCC 的垃圾回收系统
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
          // 打印具体的赋值语句代码和位置信息
          CUTIE_TRY (gcc_ext_util::get_source_location_string (CUTIE_ARGS, gimple_location (stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
          fprintf(ctx.debug_file, "  Assignment statement at %s:\n", ctx.source_location_buffer);
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

namespace array_detector {

CutieErrorCode analyze_field_assignments_in_functions(ArrayDetector &detector, CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
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
          CUTIE_TRY (gcc_ext_util::get_type_name (CUTIE_ARGS, context, containing_type_name));
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
    
    // 添加函数基本信息调试
    CUTIE_DEBUG_PRINT("Function has %d basic blocks", n_basic_blocks_for_fn(fn));
    
    // 遍历函数中的所有基本块
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      CUTIE_DEBUG_PRINT("Processing basic block %d", bb->index);
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        CUTIE_DEBUG_PRINT("  Statement type: %s", gimple_code_name[gimple_code(stmt)]);
        
        // 检查是否是赋值语句
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          CUTIE_DEBUG_PRINT("  Found assignment statement, analyzing...");
          // 打印具体的赋值语句代码和位置信息
          CUTIE_TRY (gcc_ext_util::get_source_location_string (CUTIE_ARGS, gimple_location (stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
          fprintf(ctx.debug_file, "  Assignment statement at %s:\n", ctx.source_location_buffer);
          print_gimple_stmt(ctx.debug_file, stmt, 4, TDF_DETAILS);
          gcc_ext_util::analyze_gimple_assignment(CUTIE_ARGS, stmt, &detector, func_name, decl);
        }
        // 检查是否是GIMPLE_CALL语句（可能是通过调用赋值）
        else if (gimple_code(stmt) == GIMPLE_CALL) {
          CUTIE_DEBUG_PRINT("  Found call statement, skipping for now");
          // 这里可以处理通过函数调用返回值的赋值
          // 简化处理：暂时跳过
        }
        else {
          CUTIE_DEBUG_PRINT("  Skipping statement type: %s", gimple_code_name[gimple_code(stmt)]);
        }
      }
    }
  }
  
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

}
