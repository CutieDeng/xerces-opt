#pragma once

// ============================================================================
// Field 分析 Wrapper 数据结构
// ============================================================================
//
// 三层 Wrapper 结构：
//
// 层级2：单个使用的 Wrapper (per source-use)
//   (wrapper-source-use-info-source-escape-use-info
//     use-info            : source-use-info
//     escape-use-info     : source-escape-use-info or #f)
//
// 层级1：写入级 Wrapper (per write operation)
//   (wrapper-write-info-write-source-source-escape-conclude-ownership-move
//     write-info      : field-write-info
//     write-source    : write-original-source
//     escape-conclude : source-escape-conclude
//     ownership-move  : ownership-move or #f
//     uses            : (listof wrapper-source-use-info-source-escape-use-info*))
//
// 层级3：字段级 Wrapper (per field)
//   (wrapper-field-escape-conclude-ownership-conclude
//     type              : tree
//     field-decl        : tree
//     writes            : (listof wrapper-write-info-write-source*)
//     escape-conclude   : field-escape-conclude
//     ownership-conclude: ownership-conclude)
//
// ============================================================================

#include "gcc-common.hh"
#include "state.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "prelude.hh"

// ============================================================================
// 前置声明：各子模块结果类型
// ============================================================================

namespace array_detect_ns {
  struct FieldWriteInfo;
  struct SourceUseInfo;
  struct SourceEscapeUseInfo;
  struct SourceEscapeConclude;
  struct FieldEscapeConclude;
  struct OwnershipConclude;
  struct OwnershipMove;
  // 容量分析相关
  struct MallocCapacityEvidence;
  struct ReadCapacityEvidence;
  struct WriteCapacityEvidence;
  struct CapacityConclude;
  // 数组读取分析相关
  struct ArrayReadAccess;
  struct ReadBoundCondition;
  // 数组写入分析相关
  struct ArrayWriteAccess;
  struct WriteBoundCondition;
}

namespace array_detector {
  struct WriteOriginalSource;
}

// ============================================================================
// Wrapper 定义
// ============================================================================

namespace field_analysis {

// ----------------------------------------------------------------------------
// 层级2：单个使用的 Wrapper (per source-use)
// ----------------------------------------------------------------------------
// (wrapper-source-use-info-source-escape-use-info
//   use-info         : source-use-info           ; source-use-info 模块产出
//   escape-use-info  : source-escape-use-info or #f)  ; source-escape-use-info 模块产出

struct Wrapper_SourceUseInfo_SourceEscapeUseInfo {
  ::array_detect_ns::SourceUseInfo* use_info;
  ::array_detect_ns::SourceEscapeUseInfo* escape_use_info;  // 非逃逸时为 NULL
};

// ----------------------------------------------------------------------------
// 层级1.5：数组读取的 Wrapper (per array-read)
// ----------------------------------------------------------------------------
// (wrapper-array-read-access-read-bound-conditions
//   read-access       : array-read-access              ; 数组读取访问
//   bound-conditions  : (listof read-bound-condition)) ; 边界条件列表 (一对多)

struct Wrapper_ArrayReadAccess_ReadBoundConditions {
  ::array_detect_ns::ArrayReadAccess* read_access;
  vec<::array_detect_ns::ReadBoundCondition*, va_gc>* bound_conditions;  // 一对多
};

// ----------------------------------------------------------------------------
// 层级1.5b：数组写入的 Wrapper (per array-write)
// ----------------------------------------------------------------------------
// (wrapper-array-write-access-write-bound-conditions
//   write-access       : array-write-access              ; 数组写入访问
//   bound-conditions   : (listof write-bound-condition)) ; 边界条件列表 (一对多)

struct Wrapper_ArrayWriteAccess_WriteBoundConditions {
  ::array_detect_ns::ArrayWriteAccess* write_access;
  vec<::array_detect_ns::WriteBoundCondition*, va_gc>* bound_conditions;  // 一对多
};

// ============================================================================
// Malloc 证据 Hashmap 类型定义
// ============================================================================
// (mapof integer-field (listof malloc-capacity-evidence))
// Key: tree (FIELD_DECL) - 整数字段
// Value: vec<MallocCapacityEvidence*> - 该字段关联的证据列表
// 使用 GCC 内置的 ggc_ptr_hash<tree_node> 作为 tree 的 hash traits

typedef simple_hashmap_traits<ggc_ptr_hash<tree_node>, vec<::array_detect_ns::MallocCapacityEvidence*, va_gc>*>
  TreeToMallocEvidencesTraits;

typedef hash_map<tree, vec<::array_detect_ns::MallocCapacityEvidence*, va_gc>*, TreeToMallocEvidencesTraits>
  MallocEvidencesByIntegerFieldMap;

// ============================================================================
// Read 证据 Hashmap 类型定义
// ============================================================================
// (mapof integer-field (listof read-capacity-evidence))

typedef simple_hashmap_traits<ggc_ptr_hash<tree_node>, vec<::array_detect_ns::ReadCapacityEvidence*, va_gc>*>
  TreeToReadEvidencesTraits;

typedef hash_map<tree, vec<::array_detect_ns::ReadCapacityEvidence*, va_gc>*, TreeToReadEvidencesTraits>
  ReadEvidencesByIntegerFieldMap;

// ============================================================================
// Write 证据 Hashmap 类型定义
// ============================================================================
// (mapof integer-field (listof write-capacity-evidence))

typedef simple_hashmap_traits<ggc_ptr_hash<tree_node>, vec<::array_detect_ns::WriteCapacityEvidence*, va_gc>*>
  TreeToWriteEvidencesTraits;

typedef hash_map<tree, vec<::array_detect_ns::WriteCapacityEvidence*, va_gc>*, TreeToWriteEvidencesTraits>
  WriteEvidencesByIntegerFieldMap;

// ----------------------------------------------------------------------------
// 层级1：写入级 Wrapper
// ----------------------------------------------------------------------------
// (wrapper-write-info-write-source-source-escape-conclude-ownership-move
//   write-info       : field-write-info
//   write-source     : write-original-source
//   escape-conclude  : source-escape-conclude
//   ownership-move   : ownership-move or #f
//   uses             : (listof wrapper-source-use-info-source-escape-use-info*)
//   malloc-evidences : (listof malloc-capacity-evidence))

struct Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove {
  ::array_detect_ns::FieldWriteInfo* write_info;
  ::array_detector::WriteOriginalSource* write_source;
  ::array_detect_ns::SourceEscapeConclude* escape_conclude;
  ::array_detect_ns::OwnershipMove* ownership_move;
  vec<Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>* uses;

  // === malloc 容量证据（per write）===
  vec<::array_detect_ns::MallocCapacityEvidence*, va_gc>* malloc_evidences;
};

// ----------------------------------------------------------------------------
// 层级3：字段级 Wrapper
// ----------------------------------------------------------------------------
// 扩展：添加容量分析相关字段
//   malloc_evidences_map : (mapof integer-field (listof malloc-capacity-evidence))
//   read_evidences       : (listof read-capacity-evidence)
//   write_evidences      : (listof write-capacity-evidence)
//   capacity_conclude    : capacity-conclude

struct Wrapper_FieldEscapeConclude_OwnershipConclude {
  tree type;
  tree field_decl;
  vec<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_OwnershipMove*, va_gc>* writes;

  // === 逃逸分析结论 ===
  ::array_detect_ns::FieldEscapeConclude* escape_conclude;

  // === 所有权分析结论 ===
  ::array_detect_ns::OwnershipConclude* ownership_conclude;

  // === 数组读取分析 (per-read wrappers, 一对多) ===
  vec<Wrapper_ArrayReadAccess_ReadBoundConditions*, va_gc>* array_reads;

  // === 数组写入分析 (per-write wrappers, 一对多) ===
  vec<Wrapper_ArrayWriteAccess_WriteBoundConditions*, va_gc>* array_writes;

  // === 容量关联分析 ===
  // malloc 证据按 integer_field 分组 (hashmap)
  MallocEvidencesByIntegerFieldMap* malloc_evidences_map;
  // read 证据按 integer_field 分组 (hashmap)
  ReadEvidencesByIntegerFieldMap* read_evidences_map;
  // write 证据按 integer_field 分组 (hashmap)
  WriteEvidencesByIntegerFieldMap* write_evidences_map;

  // === 容量关联结论 ===
  ::array_detect_ns::CapacityConclude* capacity_conclude;
};

} // namespace field_analysis
