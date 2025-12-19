#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detector-driver.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

class ArrayDetector;

// 检查字段的所有赋值是否来自同一源（用于判断数组候选）
bool checkAllAssignmentsFromSameSource(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

// 分析字段的使用模式
::array_detect_ns::ArrayDetectErrorCode analyzeFieldUsage(ArrayDetector &self, AD_FUNC_ARGS);

// 清理检测器资源
void deinit(ArrayDetector &self, AD_FUNC_ARGS);

// 添加字段到检测器
::array_detect_ns::ArrayDetectErrorCode addField(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info);

// 获取检测器中的字段数量
::array_detect_ns::ArrayDetectErrorCode getFieldCount(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count);
// 通过索引获取字段信息
::array_detect_ns::ArrayDetectErrorCode getField(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);

// 初始化检测器（非延迟，直接分配并创建容器，不做空指针检查）
ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS);

}

namespace array_detector {

struct ArrayDetector {

  friend ::array_detect_ns::ArrayDetectErrorCode getFieldCount(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count);
  friend ::array_detect_ns::ArrayDetectErrorCode getField(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);
  friend ::array_detect_ns::ArrayDetectErrorCode analyzeFieldUsage(ArrayDetector &self, AD_FUNC_ARGS);
  friend void deinit(ArrayDetector &self, AD_FUNC_ARGS);
  friend ::array_detect_ns::ArrayDetectErrorCode addField(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info);

  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化

  ArrayDetector() : m_fields(nullptr) {
    // 不在构造函数中初始化，使用延迟初始化
  }
  
};

}

