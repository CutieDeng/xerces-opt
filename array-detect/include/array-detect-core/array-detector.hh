#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"
#include "analysis-data.hh"
#include "type-field-hash.hh"

// ============================================================================
// 前置声明：各分析模块的结果类型
// ============================================================================
// 数据流关系（lisp 风格描述）：
// - field -> (listof field-write-info)              [FieldWriteInfo]
// - field-write-info -> write-original-source       [WriteOriginalSource]
// - write-original-source -> (listof source-use)    [SourceUseResult]
// - (listof source-use) -> (listof escaped-use)     [EscapedUseResult]
// - source, uses, escaped -> source-escape-conclude [SourceEscapeConclude]
// - field, (listof source) -> field-escape-conclude [FieldEscapeConclude]
// - field-write-info, source -> ownership-move      [OwnershipMoveResult]
// ============================================================================

namespace array_detect_ns {
  struct FieldWriteInfo;              // 字段写入信息
  struct SourceUseResult;             // 源使用分析结果
  struct EscapedUseResult;            // 逃逸使用结果
  struct SourceEscapeConclude;        // 源逃逸结论
  struct FieldEscapeConclude;         // 字段逃逸结论
  struct OwnershipMoveResult;         // 所有权转移结果

  // 向后兼容别名
  typedef FieldWriteInfo FieldWriteCapture;
  typedef SourceUseResult SourceUseAnalysisResult;
  typedef EscapedUseResult EscapeExtractionResult;
  typedef SourceEscapeConclude EscapeEvidenceResult;
  typedef FieldEscapeConclude TypeFieldEscapeSummary;
  typedef OwnershipMoveResult OwnershipTransferAnalysisResult;
}

namespace array_detector {

  struct WriteOriginalSource;  // 写入原始来源
  typedef WriteOriginalSource FieldSourceInfo;  // 向后兼容

using namespace ::array_detect_ns;

class ArrayDetector;

// 检查字段的所有赋值是否来自同一源（用于判断数组候选）
bool checkAllAssignmentsFromSameSource (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field, char const ** out_unique_source);

// 分析字段的使用模式
ArrayDetectErrorCode analyzeFieldUsage (ArrayDetector &self, AD_FUNC_ARGS);

// 清理检测器资源
void deinit (ArrayDetector &self, AD_FUNC_ARGS);

// 添加字段到检测器
ArrayDetectErrorCode addField (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field_info);

// 获取检测器中的字段数量
ArrayDetectErrorCode getFieldCount (ArrayDetector const &self, AD_FUNC_ARGS, size_t * out_count);
// 通过索引获取字段信息
ArrayDetectErrorCode getField (ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);

// 初始化检测器（非延迟，直接分配并创建容器，不做空指针检查）
ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS);

}

namespace array_detector {

// ============================================================================
// 字段写入分析记录 (FieldWriteAnalysisRecord)
// ============================================================================
// 单个字段写入操作的完整分析链
// 数据流：field-write-info -> write-original-source -> uses -> escape-conclude
//
// (field-write-analysis-record
//   write-info        : field-write-info*
//   source            : write-original-source*
//   use-result        : source-use-result*
//   escape-conclude   : source-escape-conclude*
//   ownership-move    : ownership-move-result*)
// ============================================================================

struct FieldWriteAnalysisRecord {
  // 字段写入信息
  FieldWriteInfo* write_info;

  // 写入原始来源
  WriteOriginalSource* source;

  // 源使用分析结果
  SourceUseResult* use_result;

  // 源逃逸结论
  SourceEscapeConclude* escape_conclude;

  // 所有权转移结果
  OwnershipMoveResult* ownership_move;

  // 保留字段
  void* reserved;

  // 向后兼容字段别名（内联访问器）
  FieldWriteCapture* get_write_capture() { return write_info; }
  FieldSourceInfo* get_source_info() { return source; }
  SourceUseAnalysisResult* get_escape_analysis() { return use_result; }
  EscapeEvidenceResult* get_escape_evidence() { return escape_conclude; }
  OwnershipTransferAnalysisResult* get_ownership_transfer() { return ownership_move; }
};

// ============================================================================
// 类型字段分析数据 (TypeFieldAnalysisData)
// ============================================================================
// 单个 (type, field) 的完整分析结果
// 数据流：field -> (listof field-write-analysis-record) -> field-escape-conclude
//
// (type-field-analysis-data
//   type                   : tree
//   field-decl             : tree
//   write-records          : (listof field-write-analysis-record*)
//   escape-conclude        : field-escape-conclude*)
// ============================================================================

struct TypeFieldAnalysisData {
  // 标识信息
  tree type;
  tree field_decl;

  // 写入操作分析记录列表
  vec<FieldWriteAnalysisRecord*>* write_records;

  // 是否存在拒绝证据
  bool has_rejecting_evidence;

  // 字段逃逸结论
  FieldEscapeConclude* escape_conclude;

  // 保留字段
  void* reserved;

  // 向后兼容字段别名
  vec<FieldWriteAnalysisRecord*>* get_write_analysis_records() { return write_records; }
  FieldEscapeConclude* get_escape_summary() { return escape_conclude; }
};

// ============================================================================
// 向后兼容别名（逐步废弃）
// ============================================================================
// 为了平滑迁移，保留旧名称作为别名，后续版本将移除
typedef TypeFieldAnalysisData TypeFieldWriteOps;

// 类型-字段键：用于 hash_map 的复合键（简单结构体）
// 注意：定义也在 type-field-hash-traits.hh 中，避免循环依赖
struct TypeFieldKey {
  tree type;        // 类型（TYPE_MAIN_VARIANT，GCC 内部管理）
  tree field_decl;  // 字段声明（FIELD_DECL，GCC 内部管理）
};

// 哈希和比较函数声明（实现在 type-field-hash.cc 中）
// 使用普通函数，不使用成员函数或 C++ 特性
size_t hashTypeFieldKey (TypeFieldKey const *key);
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2);

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
  // 使用 hash_map 提供 O (1) 的查找性能，而不是 vec 的 O (n) 线性查找
  // 使用 TypeFieldHashMapTraits 提供 hash 和 equal 函数
  // hash_map 支持迭代器遍历（begin/end），可以直接遍历所有条目
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* m_type_field_writes; // 使用指针类型，延迟初始化

};

}

