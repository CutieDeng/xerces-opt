#pragma once

#include "gcc-common.hh"
#include "prelude.hh"
#include "field-source-variant.hh"
#include "analysis-data.hh"
#include "array-detector.hh"
#include "type-field-hash.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// Write Operation 综合器
// ============================================================================
// 对 write-operation-trace 提取的二级信息进行整理，按 source 等价性分组
// ============================================================================

// Write Operation Fingerprint（简化的 source 信息，移除位置信息）
struct WriteOperationFingerprint {
  FieldSourceType source_type;
  
  union {
    // 函数调用 fingerprint（移除 call_stmt, location）
    struct {
      CallType call_type;
      char const *function_name;  // 可能为 NULL
    } function_call;
    
    // 变量 fingerprint（移除 ssa_name, var_decl, location）
    struct {
      char const *var_name;  // 可能为 NULL
    } variable;
    
    // 常量 fingerprint（移除 constant_value）
    struct {
      char const *constant_str;  // 可能为 NULL
    } constant;
    
    // 计算 fingerprint（移除 compute_stmt, compute_expr, location）
    struct {
      char const *description;  // 可能为 NULL
    } computation;
    
    // PHI fingerprint（移除 phi_stmt, ssa_name, var_decl, location）
    struct {
      char const *var_name;  // 可能为 NULL
    } phi;
  } data;
};

// Write Operation Detail（包含原始位置信息）
struct WriteOperationDetail {
  vec<FieldWriteCapture*> *write_operations;  // 包含位置信息的原始操作列表
};

// Fingerprint 哈希函数
size_t hashWriteOperationFingerprint (WriteOperationFingerprint const *fp);

// Fingerprint 相等比较函数
bool equalWriteOperationFingerprint (WriteOperationFingerprint const *fp1,
                                     WriteOperationFingerprint const *fp2);

// 为 WriteOperationFingerprint 提供 hash traits（在全局命名空间）
// 注意：必须在 WriteOperationFingerprint 定义之后
template<>
struct default_hash_traits<WriteOperationFingerprint> {
  typedef WriteOperationFingerprint value_type;
  typedef WriteOperationFingerprint key_type;
  typedef WriteOperationFingerprint compare_type;
  static bool const empty_zero_p = false;
  
  static hashval_t hash (WriteOperationFingerprint const &fp) {
    return (hashval_t)hashWriteOperationFingerprint (&fp);
  }
  
  static bool equal (WriteOperationFingerprint const &fp1, WriteOperationFingerprint const &fp2) {
    return equalWriteOperationFingerprint (&fp1, &fp2);
  }
  
  static bool equal_keys (WriteOperationFingerprint const &fp1, WriteOperationFingerprint const &fp2) {
    return equalWriteOperationFingerprint (&fp1, &fp2);
  }
  
  static void remove (WriteOperationFingerprint &fp) {
    mark_deleted (fp);
  }
  
  static void mark_deleted (WriteOperationFingerprint &fp) {
    fp.source_type = SOURCE_UNKNOWN;
    memset (&fp.data, 0, sizeof (fp.data));
  }
  
  static bool is_deleted (WriteOperationFingerprint const &fp) {
    return fp.source_type == SOURCE_UNKNOWN;
  }
  
  static void mark_empty (WriteOperationFingerprint &fp) {
    fp.source_type = SOURCE_UNKNOWN;
    memset (&fp.data, 0, sizeof (fp.data));
  }
  
  static bool is_empty (WriteOperationFingerprint const &fp) {
    return fp.source_type == SOURCE_UNKNOWN;
  }
};

// 综合 write-operation 信息
// 输入：detector - 包含 m_type_field_writes（已填充 FieldSourceInfo）
// 输出：consolidated - (type, field) -> hash_map<fingerprint, detail>
ArrayDetectErrorCode consolidateWriteOperations (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  hash_map<TypeFieldKey,
           hash_map<WriteOperationFingerprint, WriteOperationDetail*>*,
           TypeFieldHashMapTraits> &consolidated
);

} // namespace array_detector

