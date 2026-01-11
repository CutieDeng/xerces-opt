#pragma once

#include "gcc-common.hh"

// ============================================================================
// TypeFieldKey 的 hash_map trait 类型定义
// ============================================================================
// 为 hash_map<TypeFieldKey, FieldWrapper*> 提供专门的 trait 类型
// 使用 GCC 的 simple_hashmap_traits 包装 default_hash_traits
//
// 注意：这个文件必须在 array-detector.hh 的命名空间定义之后包含
// 因为需要完整的 TypeFieldKey 和 default_hash_traits<TypeFieldKey> 定义
// ============================================================================

// 前向声明
namespace array_detector {
  struct TypeFieldKey;
}

namespace field_analysis {
  struct Wrapper_FieldEscapeConclude_OwnershipConclude;
}

// 注意：这个文件在 array-detector.hh 末尾被包含
// 此时 TypeFieldKey 已经完整定义，default_hash_traits<TypeFieldKey> 也已经特化
// 所以可以直接使用

namespace array_detector {

// TypeFieldKey 的 hash_map trait 类型
// 使用 simple_hashmap_traits 包装 default_hash_traits<TypeFieldKey>
// Value 类型使用 field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude*
typedef simple_hashmap_traits<default_hash_traits<TypeFieldKey>, ::field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude*> TypeFieldHashMapTraits;

// 向后兼容别名
using TypeFieldAnalysisData = ::field_analysis::Wrapper_FieldEscapeConclude_OwnershipConclude;

} // namespace array_detector
