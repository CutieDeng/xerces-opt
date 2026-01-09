#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"
#include "analysis-data.hh"
#include "type-field-hash.hh"
#include "field-analysis.hh"

// ============================================================================
// 前置声明：各分析模块的结果类型
// ============================================================================
// 数据流关系（新设计）：
// - field -> (listof FieldWriteAnalysisWrapper)
// - 每个 Wrapper 包含: FieldWrite + WriteSource + UseAnalysis + EscapeConclude
// - field, (listof wrapper) -> FieldConclude
// ============================================================================

namespace array_detect_ns {
  // 使用 field_analysis 命名空间的类型
  using FieldWriteAnalysisWrapper = field_analysis::FieldWriteAnalysisWrapper;
  using FieldMoveAnalysis = field_analysis::FieldMoveAnalysis;

  // 旧类型名称的向后兼容别名
  struct FieldWriteInfo;              // 保留用于 analysis-data.hh 兼容
  struct SourceUseResult;             // 保留用于 source-escape-collection.hh 兼容
  struct EscapedUseResult;            // 保留用于 escape-synthesizer.hh 兼容
  struct SourceEscapeConclude;        // 保留用于 escape-synthesizer.hh 兼容
  struct FieldEscapeConclude;         // 保留用于 escape-synthesizer.hh 兼容
  struct OwnershipMoveResult;         // 保留用于 ownership-transfer-analysis.hh 兼容

  // 向后兼容别名
  typedef FieldWriteInfo FieldWriteCapture;
  typedef SourceUseResult SourceUseAnalysisResult;
  typedef EscapedUseResult EscapeExtractionResult;
  typedef SourceEscapeConclude EscapeEvidenceResult;
  typedef FieldEscapeConclude TypeFieldEscapeSummary;
  typedef OwnershipMoveResult OwnershipTransferAnalysisResult;
}

namespace array_detector {

  struct WriteOriginalSource;  // 写入原始来源（旧类型，保留兼容）
  typedef WriteOriginalSource FieldSourceInfo;  // 向后兼容

  using namespace ::array_detect_ns;
  using namespace ::field_analysis;

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
// TypeFieldAnalysisData: 类型字段分析数据
// ============================================================================
// 单个 (type, field) 的完整分析结果
// 使用 FieldWriteAnalysisWrapper 替代旧的分离结构
//
// (type-field-analysis-data
//   type        : tree
//   field-decl  : tree
//   writes      : (listof field-write-analysis-wrapper*)
//   conclude    : field-conclude)
// ============================================================================

struct TypeFieldAnalysisData {
  // 标识信息
  tree type;
  tree field_decl;

  // 写入操作分析列表（使用统一 Wrapper）
  vec<FieldWriteAnalysisWrapper*>* writes;

  // 是否存在拒绝证据
  bool has_rejecting_evidence;

  // 字段级结论
  FieldConclude* conclude;

  // 保留字段
  void* reserved;
};

// ============================================================================
// 向后兼容别名
// ============================================================================

typedef TypeFieldAnalysisData TypeFieldWriteOps;

// 旧的 FieldWriteAnalysisRecord 别名 -> 新的 Wrapper
typedef FieldWriteAnalysisWrapper FieldWriteAnalysisRecord;

// 类型-字段键：用于 hash_map 的复合键
struct TypeFieldKey {
  tree type;        // 类型（TYPE_MAIN_VARIANT）
  tree field_decl;  // 字段声明（FIELD_DECL）
};

// 哈希和比较函数声明
size_t hashTypeFieldKey (TypeFieldKey const *key);
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2);

} // namespace array_detector

// 在全局命名空间中特化 default_hash_traits
#include "type-field-hash-traits.hh"

// 定义 hash_map trait 类型
#include "type-field-hashmap-traits.hh"

namespace array_detector {

struct ArrayDetector {

  // 旧的数据结构（保留用于兼容，后续可移除）
  vec<FieldInfo*>* m_fields;

  // 新的数据结构：使用 hash_map 按 (type, field) 存储写入操作记录
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* m_type_field_writes;

};

}
