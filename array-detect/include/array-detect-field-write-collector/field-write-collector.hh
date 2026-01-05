#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detector {

using namespace ::array_detect_ns;

class ArrayDetector;

// 收集所有类型和字段信息：遍历编译单元提取字段写入操作
// 语义：扫描所有函数，对于每处 field access 的写入进行处理
// 输出：填充 detector.m_type_field_writes 容器，存储 (type, field, source-info, origin-src) 记录
ArrayDetectErrorCode collectTypesAndFields (ArrayDetector &detector, AD_FUNC_ARGS);

} // namespace array_detector
