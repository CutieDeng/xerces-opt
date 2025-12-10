#include "prelude.hh"
#include "array-detector.hh"
#include "state.hh"

namespace array_detector {

ArrayDetectErrorCode initializeDetector (ArrayDetector &self, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  // 延迟初始化：在 init 函数中分配 vec 指针
  if (self.m_fields == nullptr) {
    self.m_fields = ggc_alloc<vec<FieldInfo*>>();
    if (self.m_fields == nullptr) {
      AD_RETURNE(MEMORY_ERROR);
    }
    self.m_fields->create(0); // 初始化 vec 容器
    AD_DEBUG_PRINT("ArrayDetector initialized: m_fields allocated and created");
  } else {
    AD_DEBUG_PRINT("ArrayDetector already initialized");
  }
  AD_RETURNE(OK);
} AD_FUNCTION_END

bool checkAllAssignmentsFromSameSource(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field, const char** out_unique_source) {
  (void)self;
  AD_ARGS_WARN_DENY;
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

::array_detect_ns::ArrayDetectErrorCode analyzeFieldUsage(ArrayDetector &self, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  (void )ctx;
  AD_DEBUG_PRINT ("start analyze fields usage");
  
  // 检查 m_fields 是否已初始化
  if (self.m_fields == nullptr) {
    AD_DEBUG_PRINT("Warning: ArrayDetector not initialized, m_fields is null");
    AD_RETURNE(OK);
  }
  
  // 分析使用情况，判断是否是数组候选
  for (unsigned int i = 0; i < self.m_fields->length(); i++) {
    FieldInfo* field = (*self.m_fields)[i];
    // TODO: if null, warning this situation
    if (!field) {
      AD_DEBUG_PRINT("Warning: NULL field at index %u", i);
      continue;
    }

    // TODO: add field type/name log, if not null
    AD_DEBUG_PRINT("Analyzing field: %s::%s", field->containing_type, field->field_name);

    // TODO: add log about non array candidate judge, with evidence: non pointer type
    if (!field->is_pointer) {
      AD_DEBUG_PRINT("  -> Not array candidate: Not a pointer type");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: add log
    // 检查是否有冲突赋值（某个函数中有多个赋值）
    if (field->conflicting_assigns && field->conflicting_assigns->length() > 0) {
      AD_DEBUG_PRINT("  -> Not array candidate: Conflicting assignments found");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: add log
    // 基于函数级别的赋值信息判断
    if (!field->function_assignments || field->function_assignments->length() == 0) {
      // 没有赋值信息，不是数组候选
      // TODO: change the available set
      // no assignment doesn't mean no array candidate! (just ignored, can set as unrelated, but not yes or no)
      AD_DEBUG_PRINT("  -> Ignored: No assignment information available");
      field->is_array_candidate = false;
      continue;
    }

    // TODO: wrap in a new function to check the all src values
    // 检查每个函数中的赋值是否都来自函数调用，且所有函数中的赋值来源相同（唯一来源）
    const char* unique_source = NULL;
    bool all_from_function_call = checkAllAssignmentsFromSameSource (self, AD_ARGS, field, &unique_source);
    
    // 如果所有函数中的赋值都来自函数调用，且所有赋值来源相同，则是数组候选
    if (all_from_function_call && unique_source) {
      AD_DEBUG_PRINT("  -> Is array candidate: Unique source '%s'", unique_source);
      field->is_array_candidate = true;
    } else {
      AD_DEBUG_PRINT("  -> Not array candidate: Assignments not consistent or not from function calls");
      field->is_array_candidate = false;
    }
  }
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

void cleanupDetector(ArrayDetector &self, AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  // 显式清理资源，替代析构函数（遵循禁用RAII的规范）
  if (self.m_fields != nullptr) {
    self.m_fields->release();
    // GCC的ggc_alloc分配的内存会自动管理，不需要显式释放
    self.m_fields = nullptr;
    AD_DEBUG_PRINT("ArrayDetector deinitialized: m_fields released");
  } else {
    AD_DEBUG_PRINT("ArrayDetector already deinitialized or was never initialized");
  }
}

::array_detect_ns::ArrayDetectErrorCode addField(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!field_info) {
    AD_RETURNE(OK);
  }
  
  // 检查 m_fields 是否已初始化
  if (self.m_fields == nullptr) {
    AD_DEBUG_PRINT("Error: ArrayDetector not initialized, m_fields is null");
    AD_RETURNE(LOGICAL_ERROR);
  }
  
  // 添加字段信息
  self.m_fields->safe_push(field_info);
  // 调试信息：输出字段添加情况
  fprintf(stderr, "[ArrayDetector] Added field: %s::%s (total: %u)\n", 
          field_info->containing_type, field_info->field_name, 
          (unsigned)self.m_fields->length());

  AD_RETURNE(OK);
} AD_FUNCTION_END

::array_detect_ns::ArrayDetectErrorCode getFieldCount(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count) AD_FUNCTION_BEGIN {
  if (!out_count) {
    AD_DEBUG_PRINT("Error: out_count parameter is null");
    AD_RETURNE(INVALID_PARAMETER);
  }

  if (self.m_fields == nullptr) {
    *out_count = 0;
    AD_DEBUG_PRINT("ArrayDetector not initialized, returning 0 fields");
    AD_RETURNE(OK);
  }

  *out_count = self.m_fields->length();
  AD_DEBUG_PRINT("ArrayDetector field count retrieved successfully");
  AD_RETURNE(OK);
} AD_FUNCTION_END

::array_detect_ns::ArrayDetectErrorCode getField(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field) AD_FUNCTION_BEGIN {
  if (!out_field) {
    AD_DEBUG_PRINT("Error: out_field parameter is null");
    AD_RETURNE(INVALID_PARAMETER);
  }

  if (self.m_fields == nullptr) {
    *out_field = nullptr;
    AD_DEBUG_PRINT("ArrayDetector not initialized, returning null field");
    AD_RETURNE(NOT_INITIALIZED);
  }

  if (index < self.m_fields->length()) {
    *out_field = (*self.m_fields)[index];
    AD_DEBUG_PRINT("ArrayDetector field found successfully");
    AD_RETURNE(OK);
  }

  *out_field = nullptr;
  AD_DEBUG_PRINT("ArrayDetector index out of bounds");
  AD_RETURNE(INDEX_OUT_OF_BOUNDS);
} AD_FUNCTION_END

} // namespace array_detector
