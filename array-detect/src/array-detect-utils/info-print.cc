#include "info-print.hh"

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace array_detect_ns {

ArrayDetectErrorCode printResults(AD_FUNC_ARGS, ArrayDetector &detector) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT("Printing results");
  
  // 统计信息
  size_t total_fields = 0;
  ArrayDetectErrorCode count_err = getFieldCount(detector, AD_ARGS, &total_fields);
  if (count_err != OK) {
    AD_DEBUG_PRINT("Error: Failed to get field count");
    ecode = count_err;
    AD_RETURN();
  }
  AD_DEBUG_PRINT("Processing total fields in detector");
  
  if (total_fields == 0) {
    AD_RETURNE(OK);
  }
  
  // 打开输出文件（写入模式，每次覆盖，因为每个编译单元独立分析）
  FILE* output_file = fopen("array-detect-results.txt", "w");
  if (!output_file) {
    AD_DEBUG_PRINT("Failed to open output file");
    AD_RETURNE(RESOURCE_ERROR);
  }
  
  fprintf(output_file, "=== Array Detection Results ===\n\n");
  
  size_t array_candidates = 0;
  
  // 遍历所有字段，输出分析结果
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo* field = nullptr;
    ArrayDetectErrorCode field_err = getField(detector, AD_ARGS, i, &field);
    if (field_err != OK || !field) {
      AD_DEBUG_PRINT("Warning: Failed to get field");
      continue;
    }
    
    fprintf(output_file, "Type: %s, Field: %s\n", field->containing_type, field->field_name);
    fprintf(output_file, "  - Is pointer: %s\n", field->is_pointer ? "yes" : "no");
    
    if (field->is_pointer) {
      fprintf(output_file, "  - Array candidate: %s\n", field->is_array_candidate ? "yes" : "no");
      fprintf(output_file, "  - Total assignment count: %d\n", field->source_count);
      
      // 显示函数级别的赋值信息
      if (field->function_assignments && field->function_assignments->length() > 0) {
        fprintf(output_file, "  - Assignments by function:\n");
        for (unsigned int j = 0; j < field->function_assignments->length(); j++) {
          FunctionAssignment* fa = (*field->function_assignments)[j];
          if (!fa) continue;
          fprintf(output_file, "    Function: %s", fa->function_name);
          if (fa->function_id && strstr(fa->function_id, "@")) {
            // 显示函数ID以区分同名函数
            fprintf(output_file, " (ID: %s)", fa->function_id);
          }
          fprintf(output_file, "\n");
          fprintf(output_file, "      Assignment count: %d\n", fa->assignment_count);
          
          // 显示详细的赋值信息
          if (fa->assignment_details && fa->assignment_details->length() > 0) {
            fprintf(output_file, "      Detailed assignments:\n");
            for (unsigned int k = 0; k < fa->assignment_details->length(); k++) {
              AssignmentDetail* detail = (*fa->assignment_details)[k];
              if (!detail) continue;
              fprintf(output_file, "        Assignment #%d:\n", k + 1);
              fprintf(output_file, "          Source: %s\n", detail->source);
              fprintf(output_file, "          Is function call: %s\n", detail->is_call ? "yes" : "no");
              fprintf(output_file, "          RHS tree code: %d\n", detail->tree_code);
              if (detail->location_info) {
                fprintf(output_file, "          Location: %s\n", detail->location_info);
              }
              if (detail->rhs_description) {
                fprintf(output_file, "          RHS description: %s\n", detail->rhs_description);
              }
            }
          } else if (fa->sources && fa->sources->length() > 0) {
            // 回退到简化版显示
            fprintf(output_file, "      Sources:\n");
            for (unsigned int k = 0; k < fa->sources->length(); k++) {
              fprintf(output_file, "        - %s\n", (*fa->sources)[k]);
            }
          }
        }
      }
      
      if (field->is_array_candidate) {
        array_candidates++;
        fprintf(output_file, "  - DETECTED: This is likely an owned array member!\n");
        fprintf(output_file, "  - Reason: All functions have single assignment from function call\n");
      } else {
        // 说明为什么不是数组候选
        if (field->source_count == 0) {
          fprintf(output_file, "  - Reason: No assignment sources found\n");
        } else if (field->conflicting_assigns && field->conflicting_assigns->length() > 0) {
          fprintf(output_file, "  - Reason: Conflicting assignments detected\n");
          for (unsigned int j = 0; j < field->conflicting_assigns->length(); j++) {
            fprintf(output_file, "    - %s\n", (*field->conflicting_assigns)[j]);
          }
        } else {
          // 检查是否是多个函数中的赋值来源不同
          bool has_multiple_functions = (field->function_assignments && 
                                        field->function_assignments->length() > 1);
          bool has_multiple_assignments_in_function = false;
          if (field->function_assignments) {
            for (unsigned int j = 0; j < field->function_assignments->length(); j++) {
              FunctionAssignment* fa = (*field->function_assignments)[j];
              if (fa && fa->assignment_count > 1) {
                has_multiple_assignments_in_function = true;
                break;
              }
            }
          }
          
          if (has_multiple_assignments_in_function) {
            fprintf(output_file, "  - Reason: Some function has multiple assignments\n");
          } else if (has_multiple_functions) {
            fprintf(output_file, "  - Reason: Assignments in multiple functions with different sources\n");
          } else {
            fprintf(output_file, "  - Reason: Source is not a function call\n");
          }
        }
      }
    } else {
      fprintf(output_file, "  - Array candidate: no (not a pointer)\n");
    }
    
    fprintf(output_file, "\n");
  }
  
  // 写入汇总信息
  fprintf(output_file, "--- Array Member Detection Results ---\n");
  fprintf(output_file, "Total fields analyzed: %zu\n", total_fields);
  fprintf(output_file, "Array candidates: %zu\n", array_candidates);
  
  // 按类型分组输出
  fprintf(output_file, "\n--- Results by Type ---\n");
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo* field = nullptr;
    ArrayDetectErrorCode field_err = getField(detector, AD_ARGS, i, &field);
    if (field_err != OK || !field) {
      AD_DEBUG_PRINT("Warning: Failed to get field for type summary");
      continue;
    }
    
    fprintf(output_file, "\nType: %s\n", field->containing_type);
    fprintf(output_file, "  Field: %s - Is pointer: %s - Is array candidate: %s\n",
            field->field_name,
            field->is_pointer ? "Yes" : "No",
            field->is_array_candidate ? "Yes" : "No");
  }
  
  fclose(output_file);
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
