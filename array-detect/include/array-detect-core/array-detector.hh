#pragma once

#include "array-detect-utils/prelude.hh"
#include "array-detect-context/context.hh"
#include "array-detect-context/state.hh"
#include "array-detector-op0.hh"
#include "array-detect-utils/info.hh"

namespace array_detector {

class ArrayDetector;

bool check_all_src_values(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS);

void deinit(ArrayDetector &self);

::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info);

size_t get_field_count(ArrayDetector const &self);
FieldInfo* get_field(ArrayDetector const &self, size_t index);

CutieErrorCode init (ArrayDetector &self, CUTIE_FUNC_ARGS);

}

namespace array_detector {

struct ArrayDetector {

  friend size_t get_field_count(ArrayDetector const &self);
  friend FieldInfo* get_field(ArrayDetector const &self, size_t index);
  friend ::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS);
  friend void deinit(ArrayDetector &self);
  friend ::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info);

  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化

  ArrayDetector() : m_fields(nullptr) {
    // 不在构造函数中初始化，使用延迟初始化
  }
  
};

}

