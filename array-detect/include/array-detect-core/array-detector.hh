#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"
#include "analysis-data.hh"
#include "type-field-hash.hh"

namespace array_detector {

using namespace ::array_detect_ns;

class ArrayDetector;

// 检查字段的所有赋值是否来自同一源（用于判断数组候选）
bool checkAllAssignmentsFromSameSource(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field, const char** out_unique_source);

// 分析字段的使用模式
ArrayDetectErrorCode analyzeFieldUsage(ArrayDetector &self, AD_FUNC_ARGS);

// 清理检测器资源
void deinit (ArrayDetector &self, AD_FUNC_ARGS);

// 添加字段到检测器
ArrayDetectErrorCode addField(ArrayDetector &self, AD_FUNC_ARGS, FieldInfo* field_info);

// 获取检测器中的字段数量
ArrayDetectErrorCode getFieldCount(ArrayDetector const &self, AD_FUNC_ARGS, size_t* out_count);
// 通过索引获取字段信息
ArrayDetectErrorCode getField(ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);

// 初始化检测器（非延迟，直接分配并创建容器，不做空指针检查）
ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS);

}

namespace array_detector {

// 类型-字段映射：存储每个 type -> field 对应的字段写入捕获列表
struct TypeFieldWriteOps {
  tree type;                              // 类型（TYPE_MAIN_VARIANT，GCC 内部管理）
  tree field_decl;                        // 字段声明（FIELD_DECL，GCC 内部管理）
  vec<FieldWriteCapture*>* write_ops;    // 字段写入捕获列表（ggc_alloc<vec<...>>() 分配）
};

// 类型-字段键：用于 hash_map 的复合键（简单结构体）
// 注意：定义也在 type-field-hash-traits.hh 中，避免循环依赖
struct TypeFieldKey {
  tree type;        // 类型（TYPE_MAIN_VARIANT，GCC 内部管理）
  tree field_decl;  // 字段声明（FIELD_DECL，GCC 内部管理）
};

// 哈希和比较函数声明（实现在 type-field-hash.cc 中）
// 使用普通函数，不使用成员函数或 C++ 特性
size_t hashTypeFieldKey(TypeFieldKey const *key);
bool equalTypeFieldKey(TypeFieldKey const *key1, TypeFieldKey const *key2);

} // namespace array_detector

// 在全局命名空间中特化 default_hash_traits（必须在 array_detector 命名空间之后）
// 此时 TypeFieldKey 已经完整定义
#include "type-field-hash-traits.hh"

// 定义 hash_map trait 类型（必须在 default_hash_traits 特化之后）
#include "type-field-hashmap-traits.hh"

namespace array_detector {

struct ArrayDetector {

  // 旧的数据结构（保留用于兼容，后续可移除）
  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化
  
  // 新的数据结构：使用 hash_map 按 type -> field 存储写入操作记录
  // 使用 hash_map 提供 O(1) 的查找性能，而不是 vec 的 O(n) 线性查找
  // 使用 TypeFieldHashMapTraits 提供 hash 和 equal 函数
  // hash_map 支持迭代器遍历（begin/end），可以直接遍历所有条目
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* m_type_field_writes; // 使用指针类型，延迟初始化

};

}

