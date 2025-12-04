#include "prelude.hh"
#include "array-detector.hh"
#include "state.hh"

namespace array_detector {

bool check_all_src_values(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field, const char** out_unique_source) {
  (void)self; // Unused parameter
  CUTIE_ARGS_WARN_DENY;
  if (!field || !field->function_assignments) return false;

  bool all_from_function_call = true;
  const char* unique_source = NULL;
  
  for (unsigned int j = 0; j < field->function_assignments->length(); j++) {
    FunctionAssignment* fa = (*field->function_assignments)[j];
    if (!fa) continue;
    
    // 检查该函数中的所有赋值来源
    if (!fa->sources || fa->sources->length() == 0) {
      all_from_function_call = false;
      break;
    }
    
    // 检查该函数中的所有赋值是否都来自函数调用，且来源相同
    const char* func_unique_source = NULL;
    for (unsigned int k = 0; k < fa->sources->length(); k++) {
      const char* source = (*fa->sources)[k];
      
      // 检查是否是函数调用
      bool is_call = (strcmp(source, "<call-expr>") == 0) ||
                    (strstr(source, "allocate") != NULL) ||
                    (strcmp(source, "<unknown-call>") != 0 && 
                     strcmp(source, "<other-expr>") != 0 &&
                     strcmp(source, "<null>") != 0);
      
      if (!is_call) {
        all_from_function_call = false;
        break;
      }
      
      // 检查该函数中的所有赋值来源是否相同
      if (func_unique_source == NULL) {
        func_unique_source = source;
      } else if (strcmp(func_unique_source, source) != 0) {
        // 该函数中有不同的赋值来源，不是唯一来源
        all_from_function_call = false;
        break;
      }
    }
    
    if (!all_from_function_call) {
      break;
    }
    
    // 检查所有函数中的赋值来源是否相同（唯一来源）
    if (unique_source == NULL) {
      unique_source = func_unique_source;
    } else if (strcmp(unique_source, func_unique_source) != 0) {
      // 不同函数中的赋值来源不同，不是唯一来源
      all_from_function_call = false;
      break;
    }
  }

  if (out_unique_source) *out_unique_source = unique_source;
  return all_from_function_call;
}

::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  (void )ctx;
  CUTIE_DEBUG_PRINT ("start analyze fields usage");
  // 分析使用情况，判断是否是数组候选
  for (unsigned int i = 0; i < self.m_fields.length(); i++) {
    FieldInfo* field = self.m_fields[i];
    // TODO: if null, warning this situation
    if (!field) {
      CUTIE_DEBUG_PRINT("Warning: NULL field at index %u", i);
      continue;
    }

    // TODO: add field type/name log, if not null
    CUTIE_DEBUG_PRINT("Analyzing field: %s::%s", field->containing_type, field->field_name);

    // TODO: add log about non array candidate judge, with evidence: non pointer type
    if (!field->is_pointer) {
      CUTIE_DEBUG_PRINT("  -> Not array candidate: Not a pointer type");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: add log
    // 检查是否有冲突赋值（某个函数中有多个赋值）
    if (field->conflicting_assigns && field->conflicting_assigns->length() > 0) {
      CUTIE_DEBUG_PRINT("  -> Not array candidate: Conflicting assignments found");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: add log
    // 基于函数级别的赋值信息判断
    if (!field->function_assignments || field->function_assignments->length() == 0) {
      // 没有赋值信息，不是数组候选
      // TODO: change the available set
      // no assignment doesn't mean no array candidate! (just ignored, can set as unrelated, but not yes or no)
      CUTIE_DEBUG_PRINT("  -> Ignored: No assignment information available");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: wrap in a new function to check the all src values
    // 检查每个函数中的赋值是否都来自函数调用，且所有函数中的赋值来源相同（唯一来源）
    const char* unique_source = NULL;
    bool all_from_function_call = check_all_src_values (self, CUTIE_ARGS, field, &unique_source);
    
    // 如果所有函数中的赋值都来自函数调用，且所有赋值来源相同，则是数组候选
    if (all_from_function_call && unique_source) {
      CUTIE_DEBUG_PRINT("  -> Is array candidate: Unique source '%s'", unique_source);
      field->is_array_candidate = true;
    } else {
      CUTIE_DEBUG_PRINT("  -> Not array candidate: Assignments not consistent or not from function calls");
      field->is_array_candidate = false;
    }
  }
  
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

void deinit(ArrayDetector &self) {
    // 显式清理资源，替代析构函数（遵循禁用RAII的规范）
    // GCC的ggc_alloc分配的内存会自动管理，不需要显式释放
    self.m_fields.release();
}

::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info) CUTIE_FUNCTION_BEGIN {
  (void)ctx;
  if (!field_info) {
    CUTIE_RETURNV(OK);
  }
  
  // 添加字段信息
  self.m_fields.safe_push(field_info);
  // 调试信息：输出字段添加情况
  fprintf(stderr, "[ArrayDetector] Added field: %s::%s (total: %u)\n", 
          field_info->containing_type, field_info->field_name, 
          (unsigned)self.m_fields.length());

  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

size_t get_field_count(ArrayDetector const &self) {
    return self.m_fields.length();
}

FieldInfo* get_field(ArrayDetector const &self, size_t index) {
  if (index < self.m_fields.length()) {
    return self.m_fields[index];
  }
  return nullptr;
}

} // namespace array_detector
