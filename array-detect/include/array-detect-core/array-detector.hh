#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detector-op0.hh"
#include "info.hh"
#include "cutie-context-gcc-interface.hh"

namespace array_detector {

class ArrayDetector;

bool check_all_src_values(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS);

void deinit(ArrayDetector &self, CUTIE_FUNC_ARGS);

::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info);

::cutie_ns::CutieErrorCode get_field_count(ArrayDetector const &self, CUTIE_FUNC_ARGS, size_t* out_count);
::cutie_ns::CutieErrorCode get_field(ArrayDetector const &self, CUTIE_FUNC_ARGS, size_t index, FieldInfo** out_field);

CutieErrorCode init (ArrayDetector &self, CUTIE_FUNC_ARGS);

}

namespace array_detector {

struct ArrayDetector {

  friend ::cutie_ns::CutieErrorCode get_field_count(ArrayDetector const &self, CUTIE_FUNC_ARGS, size_t* out_count);
  friend ::cutie_ns::CutieErrorCode get_field(ArrayDetector const &self, CUTIE_FUNC_ARGS, size_t index, FieldInfo** out_field);
  friend ::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS);
  friend void deinit(ArrayDetector &self, CUTIE_FUNC_ARGS);
  friend ::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info);

  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化

  ArrayDetector() : m_fields(nullptr) {
    // 不在构造函数中初始化，使用延迟初始化
  }
  
};

}

