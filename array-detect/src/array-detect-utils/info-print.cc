#include "info-print.hh"
#include "array-detector.hh"
#include "virtual-call-analysis.hh"
#include "gcc-ext-util.hh"
#include "field-source-variant.hh"
#include "analysis-data.hh"

using array_detector::ArrayDetector;
using namespace array_detector;  // 为了使用 LET 宏中的类型

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace array_detect_ns {

ArrayDetectErrorCode printResults (AD_FUNC_ARGS, ArrayDetector &detector) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Printing results");
  
  // 统计信息
  size_t total_fields = 0;
  ArrayDetectErrorCode count_err = getFieldCount (detector, AD_ARGS, &total_fields);
  if (count_err != OK) {
    AD_DEBUG_PRINT ("Error: Failed to get field count");
    ecode = count_err;
    AD_RETURN ();
  }
  AD_DEBUG_PRINT ("Processing total fields in detector");
  
  if (total_fields == 0) {
    AD_RETURNE (OK);
  }
  
  // 打开输出文件（写入模式，每次覆盖，因为每个编译单元独立分析）
  FILE * output_file = fopen ("array-detect-results.txt", "w");
  if (!output_file) {
    AD_DEBUG_PRINT ("Failed to open output file");
    AD_RETURNE (RESOURCE_ERROR);
  }
  
  fprintf (output_file, "=== Array Detection Results ===\n\n");
  
  size_t array_candidates = 0;
  
  // 遍历所有字段，输出分析结果
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo * field = nullptr;
    ArrayDetectErrorCode field_err = getField (detector, AD_ARGS, i, &field);
    if (field_err != OK || !field) {
      AD_DEBUG_PRINT ("Warning: Failed to get field");
      continue;
    }
    
    fprintf (output_file, "Type: %s, Field: %s\n", field->containing_type, field->field_name);
    fprintf (output_file, "  - Is pointer: %s\n", field->is_pointer ? "yes" : "no");
    
    if (field->is_pointer) {
      fprintf (output_file, "  - Array candidate: %s\n", field->is_array_candidate ? "yes" : "no");
      fprintf (output_file, "  - Total assignment count: %d\n", field->source_count);
      
      // 显示函数级别的赋值信息
      if (field->function_assignments && field->function_assignments->length () > 0) {
        fprintf (output_file, "  - Assignments by function:\n");
        for (unsigned int j = 0; j < field->function_assignments->length (); j++) {
          FunctionAssignment * fa = (*field->function_assignments)[j];
          if (!fa) continue;
          fprintf (output_file, "    Function: %s", fa->function_name);
          if (fa->function_id && strstr (fa->function_id, "@")) {
            // 显示函数ID以区分同名函数
            fprintf (output_file, " (ID: %s)", fa->function_id);
          }
          fprintf (output_file, "\n");
          fprintf (output_file, "      Assignment count: %d\n", fa->assignment_count);
          
          // 显示详细的赋值信息
          if (fa->assignment_details && fa->assignment_details->length () > 0) {
            fprintf (output_file, "      Detailed assignments:\n");
            for (unsigned int k = 0; k < fa->assignment_details->length (); k++) {
              AssignmentDetail * detail = (*fa->assignment_details)[k];
              if (!detail) continue;
              fprintf (output_file, "        Assignment #%d:\n", k + 1);
              fprintf (output_file, "          Source: %s\n", detail->source);
              fprintf (output_file, "          Is function call: %s\n", detail->is_call ? "yes" : "no");
              fprintf (output_file, "          RHS tree code: %d\n", detail->tree_code);
              if (detail->location_info) {
                fprintf (output_file, "          Location: %s\n", detail->location_info);
              }
              if (detail->rhs_description) {
                fprintf (output_file, "          RHS description: %s\n", detail->rhs_description);
              }
            }
          } else if (fa->sources && fa->sources->length () > 0) {
            // 回退到简化版显示
            fprintf (output_file, "      Sources:\n");
            for (unsigned int k = 0; k < fa->sources->length (); k++) {
              fprintf (output_file, "        - %s\n", (*fa->sources)[k]);
            }
          }
        }
      }
      
      if (field->is_array_candidate) {
        array_candidates++;
        fprintf (output_file, "  - DETECTED: This is likely an owned array member!\n");
        fprintf (output_file, "  - Reason: All functions have single assignment from function call\n");
      } else {
        // 说明为什么不是数组候选
        if (field->source_count == 0) {
          fprintf (output_file, "  - Reason: No assignment sources found\n");
        } else if (field->conflicting_assigns && field->conflicting_assigns->length () > 0) {
          fprintf (output_file, "  - Reason: Conflicting assignments detected\n");
          for (unsigned int j = 0; j < field->conflicting_assigns->length (); j++) {
            fprintf (output_file, "    - %s\n", (*field->conflicting_assigns)[j]);
          }
        } else {
          // 检查是否是多个函数中的赋值来源不同
          bool has_multiple_functions = (field->function_assignments && 
                                        field->function_assignments->length () > 1);
          bool has_multiple_assignments_in_function = false;
          if (field->function_assignments) {
            for (unsigned int j = 0; j < field->function_assignments->length (); j++) {
              FunctionAssignment * fa = (*field->function_assignments)[j];
              if (fa && fa->assignment_count > 1) {
                has_multiple_assignments_in_function = true;
                break;
              }
            }
          }
          
          if (has_multiple_assignments_in_function) {
            fprintf (output_file, "  - Reason: Some function has multiple assignments\n");
          } else if (has_multiple_functions) {
            fprintf (output_file, "  - Reason: Assignments in multiple functions with different sources\n");
          } else {
            fprintf (output_file, "  - Reason: Source is not a function call\n");
          }
        }
      }
    } else {
      fprintf (output_file, "  - Array candidate: no (not a pointer)\n");
    }
    
    fprintf (output_file, "\n");
  }
  
  // 写入汇总信息
  fprintf (output_file, "--- Array Member Detection Results ---\n");
  fprintf (output_file, "Total fields analyzed: %zu\n", total_fields);
  fprintf (output_file, "Array candidates: %zu\n", array_candidates);
  
  // 按类型分组输出
  fprintf (output_file, "\n--- Results by Type ---\n");
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo * field = nullptr;
    ArrayDetectErrorCode field_err = getField (detector, AD_ARGS, i, &field);
    if (field_err != OK || !field) {
      AD_DEBUG_PRINT ("Warning: Failed to get field for type summary");
      continue;
    }
    
    fprintf (output_file, "\nType: %s\n", field->containing_type);
    fprintf (output_file, "  Field: %s - Is pointer: %s - Is array candidate: %s\n",
            field->field_name,
            field->is_pointer ? "Yes" : "No",
            field->is_array_candidate ? "Yes" : "No");
  }
  
  fclose (output_file);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// printGimpleCallDetails 函数：打印 GIMPLE_CALL 语句的详细信息
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printGimpleCallDetails (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  FILE * output_file
) AD_FUNCTION_BEGIN {
  if (!call_stmt || gimple_code (call_stmt) != GIMPLE_CALL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  if (!output_file) {
    output_file = ctx.debug_file;
    if (!output_file) {
      output_file = stderr;
    }
  }
  
  fprintf (output_file, "=== GIMPLE_CALL Details ===\n");
  
  // 基本信息
  location_t loc = gimple_location (call_stmt);
  fprintf (output_file, "Location: ");
  if (loc != UNKNOWN_LOCATION) {
    if (ctx.source_location_buffer && ctx.source_location_buffer_size > 0) {
      gcc_ext_util::get_source_location_string (AD_ARGS, loc,
        ctx.source_location_buffer, ctx.source_location_buffer_size);
      fprintf (output_file, "%s\n", ctx.source_location_buffer);
    } else {
      fprintf (output_file, "<location available but buffer not set>\n");
    }
  } else {
    fprintf (output_file, "<unknown>\n");
  }
  
  // 获取函数表达式
  tree fn = gimple_call_fn (call_stmt);
  if (!fn) {
    fprintf (output_file, "Function expression: NULL_TREE\n");
    fprintf (output_file, "Call type: <unknown> (NULL function expression)\n");
    AD_RETURNE (OK);
  }
  
  fprintf (output_file, "Function expression tree code: %s\n", get_tree_code_name (TREE_CODE (fn)));
  
  // 使用 match-API 匹配调用表达式
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);
  
  if (match_ecode != OK) {
    fprintf (output_file, "Call type: <match failed>\n");
    fprintf (output_file, "Match error code: %lld\n", (long long)match_ecode);
    fprintf (output_file, "Raw function expression:\n");
    print_generic_expr (output_file, fn, TDF_DETAILS);
    fprintf (output_file, "\n");
    AD_RETURNE (OK);
  }
  
  // 根据调用类型输出详细信息
  switch (match_result.call_type) {
    case CALL_VIRTUAL: {
      fprintf (output_file, "Call type: VIRTUAL (C++ virtual function call)\n");
      
      AD_MATCH_VIRTUAL_CALL (match_result, virtual_info) {
        fprintf (output_file, "  Method declaration:\n");
        if (virtual_info.method_decl) {
          if (DECL_NAME (virtual_info.method_decl)) {
            fprintf (output_file, "    Name: %s\n", IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
          } else {
            fprintf (output_file, "    Name: <unnamed>\n");
          }
          fprintf (output_file, "    Tree:\n    ");
          print_generic_expr (output_file, virtual_info.method_decl, TDF_DETAILS);
          fprintf (output_file, "\n");
        } else {
          fprintf (output_file, "    <null>\n");
        }
        
        fprintf (output_file, "  Object expression:\n    ");
        if (virtual_info.object) {
          print_generic_expr (output_file, virtual_info.object, TDF_DETAILS);
          fprintf (output_file, "\n");
        } else {
          fprintf (output_file, "<null>\n");
        }
        
        fprintf (output_file, "  Object type:\n    ");
        if (virtual_info.object_type) {
          print_generic_expr (output_file, virtual_info.object_type, TDF_DETAILS);
          fprintf (output_file, "\n");
        } else {
          fprintf (output_file, "<null>\n");
        }
        
        fprintf (output_file, "  VTable type:\n    ");
        if (virtual_info.vtable_type) {
          print_generic_expr (output_file, virtual_info.vtable_type, TDF_DETAILS);
          fprintf (output_file, "\n");
        } else {
          fprintf (output_file, "<null>\n");
        }
      } AD_MATCH_END ()
      
      break;
    }
    
    case CALL_DIRECT: {
      fprintf (output_file, "Call type: DIRECT (direct function call)\n");
      
      AD_MATCH_DIRECT_CALL (match_result, direct_info) {
        fprintf (output_file, "  Function declaration:\n");
        if (direct_info.function_decl) {
          if (DECL_NAME (direct_info.function_decl)) {
            fprintf (output_file, "    Name: %s\n", IDENTIFIER_POINTER (DECL_NAME (direct_info.function_decl)));
          } else {
            fprintf (output_file, "    Name: <unnamed>\n");
          }
          fprintf (output_file, "    Tree:\n    ");
          print_generic_expr (output_file, direct_info.function_decl, TDF_DETAILS);
          fprintf (output_file, "\n");
        } else {
          fprintf (output_file, "    <null>\n");
        }
      } AD_MATCH_END ()
      
      break;
    }
    
    case CALL_INDIRECT: {
      fprintf (output_file, "Call type: INDIRECT (indirect function call via function pointer)\n");
      
      AD_MATCH_INDIRECT_CALL (match_result, indirect_info) {
        fprintf (output_file, "  Function expression:\n    ");
        if (indirect_info.function_expr) {
          fprintf (output_file, "Tree code: %s\n    ", get_tree_code_name (TREE_CODE (indirect_info.function_expr)));
          print_generic_expr (output_file, indirect_info.function_expr, TDF_DETAILS);
          fprintf (output_file, "\n");
          
          // 如果是 SSA_NAME，尝试追踪其定义
          if (TREE_CODE (indirect_info.function_expr) == SSA_NAME) {
            gimple * def_stmt = SSA_NAME_DEF_STMT (indirect_info.function_expr);
            if (def_stmt) {
              fprintf (output_file, "  Definition statement:\n    ");
              print_gimple_stmt (output_file, def_stmt, 0, TDF_DETAILS);
              fprintf (output_file, "\n");
            }
          }
        } else {
          fprintf (output_file, "<null>\n");
        }
      } AD_MATCH_END ()
      
      break;
    }
    
    case CALL_UNKNOWN:
    default: {
      fprintf (output_file, "Call type: UNKNOWN\n");
      break;
    }
  }
  
  // 输出调用参数信息
  unsigned int num_args = gimple_call_num_args (call_stmt);
  fprintf (output_file, "Number of arguments: %u\n", num_args);
  if (num_args > 0) {
    fprintf (output_file, "Arguments:\n");
    for (unsigned int i = 0; i < num_args; i++) {
      tree arg = gimple_call_arg (call_stmt, i);
      fprintf (output_file, "  Arg[%u]: ", i);
      if (arg) {
        fprintf (output_file, "Tree code: %s\n    ", get_tree_code_name (TREE_CODE (arg)));
        print_generic_expr (output_file, arg, TDF_DETAILS);
        fprintf (output_file, "\n");
      } else {
        fprintf (output_file, "<null>\n");
      }
    }
  }
  
  // 输出返回值信息
  tree lhs = gimple_call_lhs (call_stmt);
  if (lhs) {
    fprintf (output_file, "Return value (LHS):\n  ");
    fprintf (output_file, "Tree code: %s\n  ", get_tree_code_name (TREE_CODE (lhs)));
    print_generic_expr (output_file, lhs, TDF_DETAILS);
    fprintf (output_file, "\n");
  } else {
    fprintf (output_file, "Return value: <void>\n");
  }
  
  // 输出完整的 GIMPLE 语句
  fprintf (output_file, "Full GIMPLE statement:\n  ");
  print_gimple_stmt (output_file, call_stmt, 0, TDF_DETAILS);
  fprintf (output_file, "\n");
  
  fprintf (output_file, "=== End GIMPLE_CALL Details ===\n\n");
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// 调试输出函数：打印字段写入来源信息
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printFieldWriteSourceInfo (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  ::array_detect_ns::FieldWriteCapture const &capture,
  ::array_detector::FieldSourceInfo *source_info
) AD_FUNCTION_BEGIN {
  if (!source_info) {
    AD_DEBUG_PRINT ("Field write source info: <null>");
    AD_RETURNE (OK);
  }
  
  // 获取类型名和字段名（使用已有的工具函数，不创建临时缓冲区）
  char const * type_name = NULL;
  AD_TRY (gcc_ext_util::formatTypeNameWithNamespace (AD_ARGS, type, type_name));
  char const * field_name = NULL;
  AD_TRY (gcc_ext_util::getFieldName (AD_ARGS, field_decl, field_name));
  
  // 输出基本信息
  AD_DEBUG_PRINT ("Extracted source for field write:");
  AD_DEBUG_PRINT ("  Type: %s", type_name ? type_name : "<unknown>");
  AD_DEBUG_PRINT ("  Field: %s", field_name ? field_name : "<unknown>");
  AD_DEBUG_PRINT ("  Function: %s", capture.function_name ? capture.function_name : "<unknown>");
  AD_DEBUG_PRINT ("  BB index: %d", capture.bb_index);
  
  // 输出原始代码位置
  if (capture.location != UNKNOWN_LOCATION) {
    if (ctx.source_location_buffer && ctx.source_location_buffer_size > 0) {
      AD_TRY (gcc_ext_util::get_source_location_string (AD_ARGS, capture.location,
                                                        ctx.source_location_buffer, ctx.source_location_buffer_size));
      AD_DEBUG_PRINT ("  Location: %s", ctx.source_location_buffer);
    }
  }
  
  // 根据来源类型输出详细信息（使用 LET 宏）
  LET_SOURCE_FUNCTION_CALL (call, *source_info) {
    char const * call_type_str = "UNKNOWN";
    if (call.call_type == ::array_detect_ns::CALL_VIRTUAL) call_type_str = "VIRTUAL";
    else if (call.call_type == ::array_detect_ns::CALL_DIRECT) call_type_str = "DIRECT";
    else if (call.call_type == ::array_detect_ns::CALL_INDIRECT) call_type_str = "INDIRECT";
    AD_DEBUG_PRINT ("  Source type: FUNCTION_CALL (%s)", call_type_str);
    
    // 对于虚函数调用，如果 function_name 是 <virtual-call>，尝试提取实际的函数名
    // 注意：即使 call_type 不是 CALL_VIRTUAL（可能被错误识别为 CALL_INDIRECT），
    // 只要 function_name 是 <virtual-call>，也应该尝试提取
    char const * display_function_name = call.function_name;
    if (call.call_stmt && 
        (!call.function_name || strcmp (call.function_name, "<virtual-call>") == 0)) {
      char const * extracted_name = NULL;
      if (extractVirtualCallFunctionName (AD_ARGS, call.call_stmt, extracted_name) == OK && extracted_name) {
        display_function_name = extracted_name;
      }
    }
    
    AD_DEBUG_PRINT ("  Call function name: %s", display_function_name ? display_function_name : "<unknown>");
  } END_LET ()
  else LET_SOURCE_VARIABLE (var, *source_info) {
    AD_DEBUG_PRINT ("  Source type: VARIABLE");
    AD_DEBUG_PRINT ("  Variable name: %s", var.var_name ? var.var_name : "<unknown>");
  } END_LET ()
  else LET_SOURCE_CONSTANT (constant, *source_info) {
    AD_DEBUG_PRINT ("  Source type: CONSTANT");
    AD_DEBUG_PRINT ("  Constant value: %s", constant.constant_str ? constant.constant_str : "<unknown>");
  } END_LET ()
  else LET_SOURCE_COMPUTATION (comp, *source_info) {
    AD_DEBUG_PRINT ("  Source type: COMPUTATION");
    AD_DEBUG_PRINT ("  Description: %s", comp.description ? comp.description : "<unknown>");
  } END_LET ()
  else LET_SOURCE_PHI (phi, *source_info) {
    AD_DEBUG_PRINT ("  Source type: PHI");
    AD_DEBUG_PRINT ("  Variable name: %s", phi.var_name ? phi.var_name : "<unknown>");
  } END_LET ()
  else {
    AD_DEBUG_PRINT ("  Source type: UNKNOWN");
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// 辅助函数：提取虚函数调用的函数名
// ----------------------------------------------------------------------------

ArrayDetectErrorCode extractVirtualCallFunctionName (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  char const * &function_name
) AD_FUNCTION_BEGIN {
  function_name = NULL;
  
  if (!call_stmt || gimple_code (call_stmt) != GIMPLE_CALL) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  tree fn = gimple_call_fn (call_stmt);
  if (!fn) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] gimple_call_fn tree code: %s",
                  get_tree_code_name (TREE_CODE (fn)));
  
  // 首先尝试直接检查 OBJ_TYPE_REF（虚函数调用的直接形式）
  tree current_expr = fn;
  
  // 如果 fn 是 SSA_NAME，尝试追踪其定义
  if (TREE_CODE (current_expr) == SSA_NAME) {
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] fn is SSA_NAME, tracing definition");
    gimple * def_stmt = SSA_NAME_DEF_STMT (current_expr);
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] def_stmt: %p, gimple_code: %d",
                    (void*)def_stmt, def_stmt ? (int)gimple_code (def_stmt) : -1);
    if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] rhs tree code: %s",
                      rhs ? get_tree_code_name (TREE_CODE (rhs)) : "<null>");
      if (rhs) {
        current_expr = rhs;
      }
    }
  }
  
  // 检查是否是 OBJ_TYPE_REF
  AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] current_expr tree code: %s",
                  get_tree_code_name (TREE_CODE (current_expr)));
  if (TREE_CODE (current_expr) == OBJ_TYPE_REF) {
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found OBJ_TYPE_REF!");
    
    // 尝试从 OBJ_TYPE_REF 的类型信息中提取函数名
    tree obj_type_ref_type = TREE_TYPE (current_expr);
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF type: %s",
                    obj_type_ref_type ? get_tree_code_name (TREE_CODE (obj_type_ref_type)) : "<null>");
    
    // 首先尝试从 OBJ_TYPE_REF_TOKEN 获取函数名（如果可用）
    // OBJ_TYPE_REF_TOKEN 可能包含原始的方法声明信息
#ifdef OBJ_TYPE_REF_TOKEN
    tree obj_type_ref_token = OBJ_TYPE_REF_TOKEN (current_expr);
    if (obj_type_ref_token) {
      enum tree_code token_code = TREE_CODE (obj_type_ref_token);
      char const * token_code_name = get_tree_code_name (token_code);
      AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF_TOKEN: %p, tree code: %s (%d)",
                      (void*)obj_type_ref_token, token_code_name ? token_code_name : "<unknown>", (int)token_code);
      
      if (token_code == FUNCTION_DECL && DECL_NAME (obj_type_ref_token)) {
        char const * method_name = IDENTIFIER_POINTER (DECL_NAME (obj_type_ref_token));
        AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found function name from OBJ_TYPE_REF_TOKEN: %s", method_name);
        function_name = ggc_strdup (method_name);
        AD_RETURNE (OK);
      } else if (token_code == INTEGER_CST) {
        // OBJ_TYPE_REF_TOKEN 是虚表索引（整数常量）
        // 无法直接从索引获取函数名，需要从对象类型和虚表信息推断
        long long vtable_index = (long long)TREE_INT_CST_LOW (obj_type_ref_token);
        AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF_TOKEN is INTEGER_CST (vtable index): %lld",
                        vtable_index);
        
        // 尝试从对象类型和虚表信息推断函数名
        // 获取对象类型
        tree obj_type_ref_object = OBJ_TYPE_REF_OBJECT (current_expr);
        if (obj_type_ref_object) {
          tree object_type = TREE_TYPE (obj_type_ref_object);
          if (object_type) {
            // 处理指针和引用类型
            if (TREE_CODE (object_type) == POINTER_TYPE || TREE_CODE (object_type) == REFERENCE_TYPE) {
              object_type = TREE_TYPE (object_type);
            }
            if (object_type) {
              object_type = TYPE_MAIN_VARIANT (object_type);
              AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Object type: %s",
                              get_tree_code_name (TREE_CODE (object_type)));
              
              // 如果是 RECORD_TYPE，尝试从类型的声明列表中查找虚函数
              if (TREE_CODE (object_type) == RECORD_TYPE) {
                // 遍历类型的所有声明（包括方法和字段）
                int method_index = 0;
                for (tree decl = TYPE_FIELDS (object_type); decl; decl = DECL_CHAIN (decl)) {
                  if (TREE_CODE (decl) == FUNCTION_DECL && DECL_VIRTUAL_P (decl)) {
                    if (method_index == vtable_index) {
                      // 找到匹配的虚函数
                      if (DECL_NAME (decl)) {
                        char const * method_name = IDENTIFIER_POINTER (DECL_NAME (decl));
                        AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found function name from vtable index: %s", method_name);
                        function_name = ggc_strdup (method_name);
                        AD_RETURNE (OK);
                      }
                    }
                    method_index++;
                  }
                }
                AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Could not find function at vtable index %lld (checked %d methods)",
                                vtable_index, method_index);
              }
            }
          }
        }
      }
    }
#endif
    
    tree method = OBJ_TYPE_REF_EXPR (current_expr);
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF_EXPR: %p, method tree code: %s",
                    (void*)method, method ? get_tree_code_name (TREE_CODE (method)) : "<null>");
    
    // 尝试从 OBJ_TYPE_REF 的其他字段获取信息
    tree obj_type_ref_object = OBJ_TYPE_REF_OBJECT (current_expr);
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF_OBJECT: %p, tree code: %s",
                    (void*)obj_type_ref_object, obj_type_ref_object ? get_tree_code_name (TREE_CODE (obj_type_ref_object)) : "<null>");
    
    // OBJ_TYPE_REF_EXPR 可能返回 SSA_NAME 而不是直接的 FUNCTION_DECL
    // 需要追踪 SSA_NAME 的定义
    tree method_decl = method;
    if (method && TREE_CODE (method) == SSA_NAME) {
      AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] method is SSA_NAME, tracing definition");
      gimple * method_def = SSA_NAME_DEF_STMT (method);
      if (method_def && gimple_code (method_def) == GIMPLE_ASSIGN) {
        tree method_rhs = gimple_assign_rhs1 (method_def);
        AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] method_def rhs tree code: %s",
                        method_rhs ? get_tree_code_name (TREE_CODE (method_rhs)) : "<null>");
        if (method_rhs && TREE_CODE (method_rhs) == FUNCTION_DECL) {
          method_decl = method_rhs;
          AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found FUNCTION_DECL in SSA_NAME definition");
        } else if (method_rhs && TREE_CODE (method_rhs) == ADDR_EXPR) {
          tree addr_operand = TREE_OPERAND (method_rhs, 0);
          if (addr_operand && TREE_CODE (addr_operand) == FUNCTION_DECL) {
            method_decl = addr_operand;
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found FUNCTION_DECL in ADDR_EXPR");
          }
        } else if (method_rhs && TREE_CODE (method_rhs) == MEM_REF) {
          // mem_ref 表示从内存读取，可能是从虚表读取函数指针
          // 尝试从 OBJ_TYPE_REF 的类型信息中提取函数名
          AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] method_rhs is MEM_REF, trying to extract from OBJ_TYPE_REF type");
          tree obj_type_ref_type = TREE_TYPE (current_expr);
          if (obj_type_ref_type) {
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF type: %s",
                            get_tree_code_name (TREE_CODE (obj_type_ref_type)));
            // 如果是函数指针类型，尝试从类型中提取信息
            if (TREE_CODE (obj_type_ref_type) == POINTER_TYPE) {
              tree pointed_type = TREE_TYPE (obj_type_ref_type);
              if (pointed_type && TREE_CODE (pointed_type) == FUNCTION_TYPE) {
                AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF points to FUNCTION_TYPE");
                // 对于虚函数调用，类型信息可能不足以确定函数名
                // 需要从其他地方获取
              }
            }
          }
          
          // 尝试从 OBJ_TYPE_REF 的原始方法声明中获取
          // 即使 OBJ_TYPE_REF_EXPR 返回 SSA_NAME，原始的方法声明信息可能还在
          // 尝试通过类型系统查找
          tree method_var = SSA_NAME_VAR (method);
          if (method_var && TREE_CODE (method_var) == VAR_DECL) {
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found VAR_DECL from SSA_NAME_VAR");
            // 继续追踪...
          }
          
          // 如果 mem_ref 的 operand 是 SSA_NAME，继续追踪
          tree mem_ref_base = TREE_OPERAND (method_rhs, 0);
          if (mem_ref_base && TREE_CODE (mem_ref_base) == SSA_NAME) {
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] MEM_REF base is SSA_NAME, tracing further");
            gimple * mem_ref_def = SSA_NAME_DEF_STMT (mem_ref_base);
            if (mem_ref_def && gimple_code (mem_ref_def) == GIMPLE_ASSIGN) {
              tree mem_ref_rhs = gimple_assign_rhs1 (mem_ref_def);
              AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] MEM_REF base rhs tree code: %s",
                              mem_ref_rhs ? get_tree_code_name (TREE_CODE (mem_ref_rhs)) : "<null>");
            }
          }
          
          // 对于 mem_ref 的情况，尝试从 OBJ_TYPE_REF 的类型中提取
          // OBJ_TYPE_REF 的类型应该是函数指针类型，但可能无法直接获取函数名
          // 尝试使用 gimple_call_fndecl 作为备选方案
          tree fndecl_from_call = gimple_call_fndecl (call_stmt);
          AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] gimple_call_fndecl: %p",
                          (void*)fndecl_from_call);
          if (fndecl_from_call && TREE_CODE (fndecl_from_call) == FUNCTION_DECL && DECL_NAME (fndecl_from_call)) {
            char const * method_name = IDENTIFIER_POINTER (DECL_NAME (fndecl_from_call));
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Found function name via gimple_call_fndecl: %s", method_name);
            function_name = ggc_strdup (method_name);
            AD_RETURNE (OK);
          }
          
          // 尝试从 OBJ_TYPE_REF 的类型信息中提取
          // 对于虚函数调用，即使 OBJ_TYPE_REF_EXPR 返回 SSA_NAME，
          // OBJ_TYPE_REF 的类型应该包含函数签名信息
          if (obj_type_ref_type && TREE_CODE (obj_type_ref_type) == POINTER_TYPE) {
            tree pointed_type = TREE_TYPE (obj_type_ref_type);
            AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF pointed type: %s",
                            pointed_type ? get_tree_code_name (TREE_CODE (pointed_type)) : "<null>");
            if (pointed_type && TREE_CODE (pointed_type) == FUNCTION_TYPE) {
              // 函数类型不包含函数名，但我们可以尝试从其他方式获取
              // 检查是否有类型属性或其他信息
            } else if (pointed_type && TREE_CODE (pointed_type) == METHOD_TYPE) {
              // method_type 是 C++ 成员函数类型
              // 尝试从 method_type 中提取信息
              AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF points to METHOD_TYPE");
              tree basetype = TYPE_METHOD_BASETYPE (pointed_type);
              AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] METHOD_TYPE basetype: %s",
                              basetype ? get_tree_code_name (TREE_CODE (basetype)) : "<null>");
              // method_type 本身不包含函数名，但我们可以尝试从其他方式获取
            }
          }
          
          // 最后尝试：从 OBJ_TYPE_REF 的原始表达式中提取
          // 在某些情况下，OBJ_TYPE_REF 可能包含原始的方法声明引用
          // 尝试打印完整的 OBJ_TYPE_REF 结构用于调试
          AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] OBJ_TYPE_REF structure dump:");
          if (ctx.debug_file) {
            print_generic_expr (ctx.debug_file, current_expr, TDF_DETAILS);
            fprintf (ctx.debug_file, "\n");
          }
        }
      }
    }
    
    if (method_decl && TREE_CODE (method_decl) == FUNCTION_DECL && DECL_NAME (method_decl)) {
      char const * method_name = IDENTIFIER_POINTER (DECL_NAME (method_decl));
      AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Extracted virtual function name: %s", method_name);
      function_name = ggc_strdup (method_name);
      AD_RETURNE (OK);
    } else {
      AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Failed to extract method name from OBJ_TYPE_REF, method_decl tree code: %s",
                      method_decl ? get_tree_code_name (TREE_CODE (method_decl)) : "<null>");
    }
  } else {
    AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] current_expr is not OBJ_TYPE_REF");
  }
  
  // 如果直接检查失败，尝试使用 match-API
  AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] Trying matchCallExpression API");
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);
  AD_DEBUG_PRINT ("[extractVirtualCallFunctionName] matchCallExpression returned: %lld, call_type: %d",
                  (long long)match_ecode, (int)match_result.call_type);
  
  if (match_ecode == OK && match_result.call_type == CALL_VIRTUAL) {
    // 提取虚函数名
    AD_MATCH_VIRTUAL_CALL (match_result, virtual_info) {
      if (virtual_info.method_decl && DECL_NAME (virtual_info.method_decl)) {
        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
        AD_RETURNE (OK);
      }
    } AD_MATCH_END ()
  }
  
  // 无法提取函数名
  AD_RETURNE (OK);  // 返回 OK 但 function_name 为 NULL
} AD_FUNCTION_END

} // namespace array_detect_ns
