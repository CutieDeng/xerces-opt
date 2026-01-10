#include "prelude.hh"
#include "array-detector.hh"
#include "state.hh"

namespace array_detector {

ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  // 手动初始化：直接分配并创建容器，不做二次检查
  self.m_fields = ggc_alloc<vec<FieldInfo*>>();
  if (self.m_fields == nullptr) {
    AD_RETURNE (MEMORY_ERROR);
  }
  self.m_fields->create (0);
  
  // 初始化新的数据结构：使用 hash_map
  // 使用 ggc_alloc 分配内存（GCC 垃圾回收）
  // 注意：ggc_alloc<T>() 只分配内存，不调用构造函数
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* raw_ptr = 
      ggc_alloc<hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>>();
  if (raw_ptr == nullptr) {
    AD_RETURNE (MEMORY_ERROR);
  }
  // 使用 placement new 在已分配的内存上构造 hash_map 对象
  // 这是关键：ggc_alloc 只分配内存，必须使用 placement new 调用构造函数
  self.m_type_field_writes = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>();
  if (self.m_type_field_writes == nullptr) {
    AD_RETURNE (MEMORY_ERROR);
  }
  // hash_map 构造后需要调用 create_ggc () 来初始化内部哈希表
  self.m_type_field_writes->create_ggc (0);

  AD_RETURNE (OK);
} AD_FUNCTION_END

bool checkAllAssignmentsFromSameSource (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field, char const ** out_unique_source) {
  (void)self;
  AD_ARGS_WARN_DENY;
  if (!field || !field->function_assignments) return false;

  bool all_from_function_call = true;
  char const * unique_source = NULL;
  
  for (unsigned int j = 0; j < field->function_assignments->length (); j++) {
    FunctionAssignment * fa = (*field->function_assignments)[j];
    if (!fa) continue;
    
    // 检查该函数中的所有赋值来源
    if (!fa->sources || fa->sources->length () == 0) {
      all_from_function_call = false;
      break;
    }
    
    // 检查该函数中的所有赋值是否都来自函数调用，且来源相同
    char const * func_unique_source = NULL;
    for (unsigned int k = 0; k < fa->sources->length (); k++) {
      char const * source = (*fa->sources)[k];
      
      // 检查是否是函数调用
      bool is_call = (strcmp (source, "<call-expr>") == 0) ||
                    (strstr (source, "allocate") != NULL) ||
                    (strcmp (source, "<unknown-call>") != 0 && 
                     strcmp (source, "<other-expr>") != 0 &&
                     strcmp (source, "<null>") != 0);
      
      if (!is_call) {
        all_from_function_call = false;
        break;
      }
      
      // 检查该函数中的所有赋值来源是否相同
      if (func_unique_source == NULL) {
        func_unique_source = source;
      } else if (strcmp (func_unique_source, source) != 0) {
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
    } else if (strcmp (unique_source, func_unique_source) != 0) {
      // 不同函数中的赋值来源不同，不是唯一来源
      all_from_function_call = false;
      break;
    }
  }

  if (out_unique_source) *out_unique_source = unique_source;
  return all_from_function_call;
}

ArrayDetectErrorCode analyzeFieldUsage (ArrayDetector &self, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  (void )ctx;

  // 分析使用情况，判断是否是数组候选
  for (unsigned int i = 0; i < self.m_fields->length (); i++) {
    FieldInfo * field = (*self.m_fields)[i];
    if (!field) {
      continue;
    }

    if (!field->is_pointer) {
      field->is_array_candidate = false;
      continue;
    }

    // 检查是否有冲突赋值
    if (field->conflicting_assigns && field->conflicting_assigns->length () > 0) {
      field->is_array_candidate = false;
      continue;
    }

    // 基于函数级别的赋值信息判断
    if (!field->function_assignments || field->function_assignments->length () == 0) {
      field->is_array_candidate = false;
      continue;
    }

    // 检查每个函数中的赋值是否都来自函数调用
    char const * unique_source = NULL;
    bool all_from_function_call = checkAllAssignmentsFromSameSource (self, AD_ARGS, field, &unique_source);

    field->is_array_candidate = all_from_function_call && unique_source;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

void deinit (ArrayDetector &self, AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  // 显式清理资源
  if (self.m_fields != nullptr) {
    self.m_fields->release ();
    self.m_fields = nullptr;
  }
  if (self.m_type_field_writes != nullptr) {
    self.m_type_field_writes = nullptr;
  }
}

ArrayDetectErrorCode addField (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field_info) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!field_info) {
    AD_RETURNE (OK);
  }
  self.m_fields->safe_push (field_info);
  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode getFieldCount (ArrayDetector const &self, AD_FUNC_ARGS, size_t * out_count) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!out_count) {
    AD_RETURNE (INVALID_PARAMETER);
  }
  *out_count = self.m_fields->length ();
  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode getField (ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!out_field) {
    AD_RETURNE (INVALID_PARAMETER);
  }
  if (index < self.m_fields->length ()) {
    *out_field = (*self.m_fields)[index];
    AD_RETURNE (OK);
  }
  *out_field = nullptr;
  AD_RETURNE (INDEX_OUT_OF_BOUNDS);
} AD_FUNCTION_END

} // namespace array_detector
