#pragma once

#include "gcc-common.hh"

// 前向声明
namespace array_detector {
  struct TypeFieldKey;
  size_t hashTypeFieldKey(TypeFieldKey const *key);
  bool equalTypeFieldKey(TypeFieldKey const *key1, TypeFieldKey const *key2);
}

// 需要包含完整的 TypeFieldKey 定义（在 array-detector.hh 中）
// 但为了避免循环依赖，我们在 array-detector.hh 末尾包含此文件
// 这样 array-detector.hh 先定义 TypeFieldKey，然后此文件特化 default_hash_traits

// 为 GCC 的 hash_map 提供 hash_traits 特化（必须在全局命名空间）
// GCC 的 hash_map 需要 default_hash_traits 来支持自定义键类型
// 注意：GCC 的 default_hash_traits 继承自键类型，所以我们需要让 TypeFieldKey 本身提供这些方法
// 但用户不喜欢成员函数，所以我们使用特化来提供静态方法

// 首先，为 TypeFieldKey 添加必要的类型定义（通过特化 default_hash_traits）
// 注意：这个特化必须在 TypeFieldKey 完整定义之后
template<>
struct default_hash_traits<array_detector::TypeFieldKey> {
  typedef array_detector::TypeFieldKey value_type;
  typedef array_detector::TypeFieldKey key_type;
  static const bool empty_zero_p = false;  // 空值不能为零（因为 tree 指针可能为 NULL_TREE）
  
  static hashval_t hash(array_detector::TypeFieldKey const &key) {
    return (hashval_t)array_detector::hashTypeFieldKey(&key);
  }
  
  // 根据 GCC hash_map 的实现，hash_entry 使用 Traits::equal_keys 进行比较
  // 但 default_hash_traits 可能使用 equal 方法
  // 为了兼容，我们同时提供 equal 和 equal_keys
  static bool equal(array_detector::TypeFieldKey const &key1, array_detector::TypeFieldKey const &key2) {
    return array_detector::equalTypeFieldKey(&key1, &key2);
  }
  
  // 如果 hash_map 使用 equal_keys，提供这个方法
  static bool equal_keys(array_detector::TypeFieldKey const &key1, array_detector::TypeFieldKey const &key2) {
    return array_detector::equalTypeFieldKey(&key1, &key2);
  }
  
  static void remove(array_detector::TypeFieldKey &key) {
    mark_deleted(key);
  }
  
  static void mark_deleted(array_detector::TypeFieldKey &key) {
    key.type = NULL_TREE;
    key.field_decl = NULL_TREE;
  }
  
  static bool is_deleted(array_detector::TypeFieldKey const &key) {
    return key.type == NULL_TREE && key.field_decl == NULL_TREE;
  }
  
  static void mark_empty(array_detector::TypeFieldKey &key) {
    key.type = NULL_TREE;
    key.field_decl = NULL_TREE;
  }
  
  static bool is_empty(array_detector::TypeFieldKey const &key) {
    return key.type == NULL_TREE && key.field_decl == NULL_TREE;
  }
};
