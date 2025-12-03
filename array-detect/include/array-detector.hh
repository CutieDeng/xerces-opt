#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detector-op0.hh"
#include "info.hh"

namespace array_detector {

::cutie_ns::CutieErrorCode check_all_src_values(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

::cutie_ns::CutieErrorCode analyze_usage(ArrayDetector &self, CUTIE_FUNC_ARGS);

void deinit(ArrayDetector &self);

::cutie_ns::CutieErrorCode add_field(ArrayDetector &self, CUTIE_FUNC_ARGS, FieldInfo* field_info);

size_t get_field_count(ArrayDetector const &self);
FieldInfo* get_field(ArrayDetector const &self, size_t index);

}

namespace array_detector {

class ArrayDetector {

private:
  vec<FieldInfo*> m_fields; // 使用GCC框架的vec容器存储字段信息

public:
  ArrayDetector() {
    m_fields.create(0); // 提供初始大小参数
  }
  
}

}

