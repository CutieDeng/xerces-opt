#pragma once

#include "gcc-common.hh"

// ============================================================================
// TypeFieldKey 的 hash_map trait 类型定义
// ============================================================================
// 为 hash_map<TypeFieldKey, TypeFieldWriteOps*> 提供专门的 trait 类型
// 使用 GCC 的 simple_hashmap_traits 包装 default_hash_traits
// 
// 注意：这个文件必须在 array-detector.hh 的命名空间定义之后包含
// 因为需要完整的 TypeFieldKey 和 default_hash_traits<TypeFieldKey> 定义
// ============================================================================

// 前向声明
namespace array_detector {
  struct TypeFieldKey;
  struct TypeFieldWriteOps;
}

// 注意：这个文件在 array-detector.hh 末尾被包含
// 此时 TypeFieldKey 已经完整定义，default_hash_traits<TypeFieldKey> 也已经特化
// 所以可以直接使用

namespace array_detector {

// TypeFieldKey 的 hash_map trait 类型
// 使用 simple_hashmap_traits 包装 default_hash_traits<TypeFieldKey>
// 这样 hash_map 可以使用我们定义的 hash 和 equal 函数
typedef simple_hashmap_traits<default_hash_traits<TypeFieldKey>, TypeFieldWriteOps*> TypeFieldHashMapTraits;

} // namespace array_detector
