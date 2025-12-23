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
    AD_DEBUG_PRINT ("  Call function name: %s", call.function_name ? call.function_name : "<unknown>");
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

} // namespace array_detect_ns
