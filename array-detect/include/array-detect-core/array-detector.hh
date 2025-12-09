#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detector-op0.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

class ArrayDetector;

bool check_all_src_values(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

::array_detect_ns::ArrayDetectErrorCode analyze_usage(ArrayDetector &self, AD_FUNC_ARGS);

void deinit(ArrayDetector &self, AD_FUNC_ARGS);

::array_detect_ns::ArrayDetectErrorCode add_field(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info);

::array_detect_ns::ArrayDetectErrorCode get_field_count(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count);
::array_detect_ns::ArrayDetectErrorCode get_field(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);

ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS);

}

namespace array_detector {

struct ArrayDetector {

  friend ::array_detect_ns::ArrayDetectErrorCode get_field_count(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count);
  friend ::array_detect_ns::ArrayDetectErrorCode get_field(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);
  friend ::array_detect_ns::ArrayDetectErrorCode analyze_usage(ArrayDetector &self, AD_FUNC_ARGS);
  friend void deinit(ArrayDetector &self, AD_FUNC_ARGS);
  friend ::array_detect_ns::ArrayDetectErrorCode add_field(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info);

  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化

  ArrayDetector() : m_fields(nullptr) {
    // 不在构造函数中初始化，使用延迟初始化
  }
  
};

}

