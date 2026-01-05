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
  size_t total_fields = 0;
  ArrayDetectErrorCode count_err = getFieldCount (detector, AD_ARGS, &total_fields);
  if (count_err != OK) {
    AD_DEBUG_PRINT ("ERROR: getFieldCount failed");
    ecode = count_err;
    AD_RETURN ();
  }

  if (total_fields == 0) {
    AD_RETURNE (OK);
  }

  FILE * output_file = fopen ("array-detect-results.txt", "w");
  if (!output_file) {
    AD_DEBUG_PRINT ("ERROR: failed to open output file");
    AD_RETURNE (RESOURCE_ERROR);
  }
  
  fprintf (output_file, "=== Array Detection Results ===\n\n");
  
  size_t array_candidates = 0;
  
  // 遍历所有字段，输出分析结果
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo * field = nullptr;
    ArrayDetectErrorCode field_err = getField (detector, AD_ARGS, i, &field);
    if (field_err != OK || !field) {
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
      // 如果没有调试输出文件，直接返回
      AD_RETURNE (OK);
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
  (void)capture;

  if (!source_info) {
    AD_RETURNE (OK);
  }

  char const * type_name = NULL;
  AD_TRY (gcc_ext_util::formatTypeNameWithNamespace (AD_ARGS, type, type_name));
  char const * field_name = NULL;
  AD_TRY (gcc_ext_util::getFieldName (AD_ARGS, field_decl, field_name));

  // 根据来源类型输出单行摘要
  LET_SOURCE_FUNCTION_CALL (call, *source_info) {
    char const * call_type_str = "?";
    if (call.call_type == ::array_detect_ns::CALL_VIRTUAL) call_type_str = "V";
    else if (call.call_type == ::array_detect_ns::CALL_DIRECT) call_type_str = "D";
    else if (call.call_type == ::array_detect_ns::CALL_INDIRECT) call_type_str = "I";

    char const * display_function_name = call.function_name;
    if (call.call_stmt &&
        (!call.function_name || strcmp (call.function_name, "<virtual-call>") == 0)) {
      char const * extracted_name = NULL;
      if (extractVirtualCallFunctionName (AD_ARGS, call.call_stmt, extracted_name) == OK && extracted_name) {
        display_function_name = extracted_name;
      }
    }

    AD_DEBUG_PRINT ("fieldWrite: %s::%s <- CALL(%s) %s", type_name, field_name, call_type_str,
                    display_function_name ? display_function_name : "?");
  } END_LET ()
  else LET_SOURCE_CONSTANT (constant, *source_info) {
    AD_DEBUG_PRINT ("fieldWrite: %s::%s <- CONST %s", type_name, field_name,
                    constant.constant_str ? constant.constant_str : "?");
  } END_LET ()
  else LET_SOURCE_FIELD_ACCESS (field_access, *source_info) {
    AD_DEBUG_PRINT ("fieldWrite: %s::%s <- FIELD %s::%s", type_name, field_name,
                    field_access.type_name ? field_access.type_name : "?",
                    field_access.field_name ? field_access.field_name : "?");
  } END_LET ()
  else LET_SOURCE_COMPUTATION (comp, *source_info) {
    AD_DEBUG_PRINT ("fieldWrite: %s::%s <- COMP", type_name, field_name);
  } END_LET ()
  else LET_SOURCE_PHI (phi, *source_info) {
    AD_DEBUG_PRINT ("fieldWrite: %s::%s <- PHI %s", type_name, field_name,
                    phi.var_name ? phi.var_name : "?");
  } END_LET ()

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

  tree current_expr = fn;

  // 如果 fn 是 SSA_NAME，尝试追踪其定义
  if (TREE_CODE (current_expr) == SSA_NAME) {
    gimple * def_stmt = SSA_NAME_DEF_STMT (current_expr);
    if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (rhs) {
        current_expr = rhs;
      }
    }
  }

  // 检查是否是 OBJ_TYPE_REF
  if (TREE_CODE (current_expr) == OBJ_TYPE_REF) {
#ifdef OBJ_TYPE_REF_TOKEN
    tree obj_type_ref_token = OBJ_TYPE_REF_TOKEN (current_expr);
    if (obj_type_ref_token) {
      enum tree_code token_code = TREE_CODE (obj_type_ref_token);

      if (token_code == FUNCTION_DECL && DECL_NAME (obj_type_ref_token)) {
        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (obj_type_ref_token)));
        AD_DEBUG_PRINT ("extractVCall: %s (from TOKEN)", function_name);
        AD_RETURNE (OK);
      } else if (token_code == INTEGER_CST) {
        long long vtable_index = (long long)TREE_INT_CST_LOW (obj_type_ref_token);

        tree obj_type_ref_object = OBJ_TYPE_REF_OBJECT (current_expr);
        if (obj_type_ref_object) {
          tree object_type = TREE_TYPE (obj_type_ref_object);
          if (object_type) {
            if (TREE_CODE (object_type) == POINTER_TYPE || TREE_CODE (object_type) == REFERENCE_TYPE) {
              object_type = TREE_TYPE (object_type);
            }
            if (object_type) {
              object_type = TYPE_MAIN_VARIANT (object_type);

              if (TREE_CODE (object_type) == RECORD_TYPE) {
                // 优先使用 BINFO_VIRTUALS
                tree binfo = TYPE_BINFO (object_type);
                if (binfo) {
                  tree virtuals = BINFO_VIRTUALS (binfo);
                  if (virtuals) {
                    int virtuals_index = 0;
                    for (tree virt = virtuals; virt; virt = TREE_CHAIN (virt), virtuals_index++) {
                      tree fn_decl = TREE_VALUE (virt);
                      if (fn_decl && TREE_CODE (fn_decl) != FUNCTION_DECL) {
                        tree alt_fn = TREE_PURPOSE (virt);
                        if (alt_fn && TREE_CODE (alt_fn) == FUNCTION_DECL) {
                          fn_decl = alt_fn;
                        }
                      }
                      if (fn_decl && TREE_CODE (fn_decl) == FUNCTION_DECL && virtuals_index == vtable_index) {
                        if (DECL_NAME (fn_decl)) {
                          function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fn_decl)));
                          AD_DEBUG_PRINT ("extractVCall: %s (vtable[%lld])", function_name, vtable_index);
                          AD_RETURNE (OK);
                        }
                      }
                    }
                  }
                }

                // 后备方案：TYPE_FIELDS
                if (!function_name) {
                  int method_index = 0;
                  for (tree decl = TYPE_FIELDS (object_type); decl; decl = DECL_CHAIN (decl)) {
                    if (TREE_CODE (decl) == FUNCTION_DECL && DECL_VIRTUAL_P (decl)) {
                      if (method_index == vtable_index && DECL_NAME (decl)) {
                        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (decl)));
                        AD_DEBUG_PRINT ("extractVCall: %s (TYPE_FIELDS[%d])", function_name, method_index);
                        AD_RETURNE (OK);
                      }
                      method_index++;
                    }
                  }

                  // 参数匹配后备
                  unsigned int call_args = gimple_call_num_args (call_stmt);
                  for (tree decl = TYPE_FIELDS (object_type); decl; decl = DECL_CHAIN (decl)) {
                    if (TREE_CODE (decl) == FUNCTION_DECL && DECL_VIRTUAL_P (decl)) {
                      int decl_param_count = 0;
                      tree decl_type = TREE_TYPE (decl);
                      if (decl_type) {
                        for (tree a = TYPE_ARG_TYPES (decl_type); a; a = TREE_CHAIN (a)) {
                          if (!TREE_VALUE (a)) break;
                          decl_param_count++;
                        }
                      }
                      if ((int)call_args == decl_param_count && DECL_NAME (decl)) {
                        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (decl)));
                        AD_DEBUG_PRINT ("extractVCall: %s (sig match)", function_name);
                        AD_RETURNE (OK);
                      }
                    }
                  }
                }

#ifdef TYPE_METHODS
                if (!function_name) {
                  for (tree method = TYPE_METHODS (object_type); method; method = DECL_CHAIN (method)) {
                    if (TREE_CODE (method) == FUNCTION_DECL && DECL_VIRTUAL_P (method)) {
                      int decl_param_count = 0;
                      tree decl_type = TREE_TYPE (method);
                      if (decl_type) {
                        for (tree a = TYPE_ARG_TYPES (decl_type); a; a = TREE_CHAIN (a)) {
                          if (!TREE_VALUE (a)) break;
                          decl_param_count++;
                        }
                      }
                      unsigned int call_args = gimple_call_num_args (call_stmt);
                      if ((int)call_args == decl_param_count && DECL_NAME (method)) {
                        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (method)));
                        AD_DEBUG_PRINT ("extractVCall: %s (TYPE_METHODS)", function_name);
                        AD_RETURNE (OK);
                      }
                    }
                  }
                }
#endif
              }
            }
          }
        }
      }
    }
#endif

    tree method = OBJ_TYPE_REF_EXPR (current_expr);
    tree method_decl = method;
    if (method && TREE_CODE (method) == SSA_NAME) {
      gimple * method_def = SSA_NAME_DEF_STMT (method);
      if (method_def && gimple_code (method_def) == GIMPLE_ASSIGN) {
        tree method_rhs = gimple_assign_rhs1 (method_def);
        if (method_rhs && TREE_CODE (method_rhs) == FUNCTION_DECL) {
          method_decl = method_rhs;
        } else if (method_rhs && TREE_CODE (method_rhs) == ADDR_EXPR) {
          tree addr_operand = TREE_OPERAND (method_rhs, 0);
          if (addr_operand && TREE_CODE (addr_operand) == FUNCTION_DECL) {
            method_decl = addr_operand;
          }
        } else if (method_rhs && TREE_CODE (method_rhs) == MEM_REF) {
          tree fndecl_from_call = gimple_call_fndecl (call_stmt);
          if (fndecl_from_call && TREE_CODE (fndecl_from_call) == FUNCTION_DECL && DECL_NAME (fndecl_from_call)) {
            function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fndecl_from_call)));
            AD_DEBUG_PRINT ("extractVCall: %s (fndecl)", function_name);
            AD_RETURNE (OK);
          }
        }
      }
    }

    if (method_decl && TREE_CODE (method_decl) == FUNCTION_DECL && DECL_NAME (method_decl)) {
      function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (method_decl)));
      AD_DEBUG_PRINT ("extractVCall: %s (method_decl)", function_name);
      AD_RETURNE (OK);
    }
  }

  // 如果直接检查失败，尝试使用 match-API
  CallMatchResult match_result;
  ArrayDetectErrorCode match_ecode = matchCallExpression (AD_ARGS, fn, match_result);

  if (match_ecode == OK && match_result.call_type == CALL_VIRTUAL) {
    AD_MATCH_VIRTUAL_CALL (match_result, virtual_info) {
      if (virtual_info.method_decl && DECL_NAME (virtual_info.method_decl)) {
        function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (virtual_info.method_decl)));
        AD_DEBUG_PRINT ("extractVCall: %s (match-API)", function_name);
        AD_RETURNE (OK);
      }
    } AD_MATCH_END ()
  }

  AD_RETURNE (OK);  // 返回 OK 但 function_name 为 NULL
} AD_FUNCTION_END

} // namespace array_detect_ns
