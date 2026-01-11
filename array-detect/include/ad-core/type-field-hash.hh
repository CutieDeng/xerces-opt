#pragma once

#include "gcc-common.hh"

namespace array_detector {

// 前向声明
struct TypeFieldKey;

// ============================================================================
// TypeFieldKey 哈希函数实现
// ============================================================================
// GCC 的 tree 哈希机制说明：
// 
// 1. tree 类型的本质：
//    - tree 在 GCC 中是指向 tree_node 的指针类型（typedef tree_node * tree）
//    - tree_node 是 GCC 内部表示 AST 节点的结构体
//    - 每个 tree 节点在编译过程中有稳定的内存地址
//
// 2. GCC 的 tree 哈希工作原理：
//    - GCC 使用指针值本身作为哈希的基础
//    - 因为 tree 节点在编译过程中是稳定的（由 GCC 内存池管理）
//    - 相同的 tree 节点总是有相同的指针值
//    - 因此可以直接使用指针值进行哈希，无需深入节点内部
//
// 3. 复合键的哈希策略：
//    - 对于 (type, field_decl) 这样的复合键，需要组合两个指针的哈希值
//    - 使用位运算（异或、左移、右移）来组合，避免简单的加法导致的冲突
//    - 公式：h1 ^ (h2 << 1) ^ (h2 >> (sizeof (size_t) * 8 - 1))
//    - 这样既保证了分布均匀，又避免了冲突
//
// 4. 为什么这样有效：
//    - tree 节点由 GCC 的内存池（ggc）管理，地址分布相对均匀
//    - 指针值本身已经是一个很好的哈希种子
//    - 组合两个指针时，位运算能更好地混合信息
// ============================================================================

// TypeFieldKey 的哈希函数（普通函数，不使用成员函数）
// 输入：key - 类型字段键的指针
// 返回：哈希值（size_t）
size_t hashTypeFieldKey (TypeFieldKey const *key);

// TypeFieldKey 的相等比较函数（普通函数）
// 输入：key1, key2 - 两个类型字段键的指针
// 返回：是否相等（bool）
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2);

} // namespace array_detector
