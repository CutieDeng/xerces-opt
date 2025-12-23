#include "type-field-hash.hh"
#include "array-detector.hh"

namespace array_detector {

// TypeFieldKey 的哈希函数实现
// GCC 的 tree 类型本质是指针，可以直接使用指针值进行哈希
size_t hashTypeFieldKey (TypeFieldKey const *key) {
  if (!key) {
    return 0;
  }
  // tree 是指针类型，可以直接转换为整数进行哈希
  size_t h1 = (size_t)(key->type);
  size_t h2 = (size_t)(key->field_decl);
  // 组合哈希：使用位旋转和异或，避免简单的加法导致的冲突
  // h2 << 1: 左移一位，相当于乘以2
  // h2 >> (sizeof (size_t) * 8 - 1): 右移，取最高位，实现位旋转效果
  return h1 ^ (h2 << 1) ^ (h2 >> (sizeof (size_t) * 8 - 1));
}

// TypeFieldKey 的相等比较函数实现
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2) {
  if (!key1 || !key2) {
    return key1 == key2;
  }
  return key1->type == key2->type && key1->field_decl == key2->field_decl;
}

} // namespace array_detector
