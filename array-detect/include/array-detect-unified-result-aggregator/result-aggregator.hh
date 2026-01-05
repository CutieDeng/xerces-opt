#pragma once

#include "prelude.hh"
#include "context.hh"
#include "array-detector.hh"
#include "owned-conclusion.hh"
#include "capacity-association.hh"
#include "array-access-collector.hh"
#include "bound-condition-analyzer.hh"
#include "type-field-hash-traits.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// 为 UnifiedFieldAnalysisResult* 定义专用的 hash_map traits
struct UnifiedFieldAnalysisResult;

typedef simple_hashmap_traits<default_hash_traits<TypeFieldKey>, UnifiedFieldAnalysisResult*>
  UnifiedResultHashMapTraits;

// ============================================================================
// 统一证据类型（位图）
// ============================================================================

enum UnifiedEvidenceType {
  EVID_NONE                    = 0,
  EVID_MALLOC_SIZE_ARG         = 1 << 0,   // malloc size 参数引用
  EVID_ARRAY_READ_BOUND        = 1 << 1,   // 数组读边界检查
  EVID_ARRAY_WRITE_BOUND       = 1 << 2,   // 数组写边界检查
  EVID_CONSISTENT_PAIRING      = 1 << 3,   // 一致的配对（同一表达式）
  EVID_SAME_OBJECT             = 1 << 4,   // 同一对象访问
};

// ============================================================================
// 容量字段关联（统一格式）
// ============================================================================

struct CapacityFieldRelation {
  // === 容量字段信息 ===
  tree capacity_field_decl;               // 容量字段声明
  char const* capacity_field_name;        // 容量字段名

  // === 证据位图 ===
  unsigned int evidence_bitmap;           // UnifiedEvidenceType 位图

  // === 证据详情 ===
  vec<BoundConditionAssociation*, va_gc>* bound_evidences;  // 边界条件证据
  vec<CapacityAssociationEvidence*, va_gc>* malloc_evidences; // malloc 证据

  // === 置信度 ===
  unsigned int confidence_score;          // 置信度评分（0-100）
};

// ============================================================================
// 统一字段分析结果
// ============================================================================

struct UnifiedFieldAnalysisResult {
  // === 标识信息 ===
  tree type;                              // 类型
  tree pointer_field_decl;                // 指针字段声明
  char const* type_name;                  // 类型名（缓存，不含模板参数）
  vec<char const*, va_gc>* template_args; // 模板参数列表（可能为 NULL）
  char const* pointer_field_name;         // 指针字段名（缓存）

  // === Owned 结论 ===
  FieldOwnedConclusion* owned_conclusion; // owned 分析结论（可能为 NULL）
  OwnedConclusionVerdict owned_verdict;   // owned 判定结果

  // === 数组访问信息 ===
  vec<ArrayAccessCapture*, va_gc>* array_accesses;  // 所有数组访问
  unsigned int total_read_accesses;       // 读访问总数
  unsigned int total_write_accesses;      // 写访问总数

  // === 边界分析信息 ===
  vec<ArrayAccessBoundAnalysis*, va_gc>* bound_analyses;  // 边界分析结果
  unsigned int accesses_with_bound;       // 有边界检查的访问数
  unsigned int accesses_without_bound;    // 无边界检查的访问数

  // === 容量关联信息 ===
  vec<CapacityFieldRelation*, va_gc>* capacity_relations; // 所有容量关联
  CapacityFieldRelation* best_capacity_match;  // 最佳容量匹配（可能为 NULL）

  // === 综合判定 ===
  bool is_array_candidate;                // 是否为数组候选
  bool has_bound_check;                   // 是否有边界检查
  bool has_capacity_association;          // 是否有容量关联
  unsigned int overall_confidence;        // 综合置信度（0-100）

  // === 描述 ===
  char const* summary_description;        // 汇总描述
};

// ============================================================================
// 函数声明
// ============================================================================

// 聚合所有分析结果
ArrayDetectErrorCode aggregateAllResults (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions,
  vec<PointerCapacityAssociation*, va_gc>* capacity_results,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses,
  vec<UnifiedFieldAnalysisResult*, va_gc>** out_results
);

// 创建统一结果条目
ArrayDetectErrorCode createUnifiedResult (
  AD_FUNC_ARGS,
  tree type,
  tree pointer_field_decl,
  UnifiedFieldAnalysisResult** out_result
);

// 合并 owned conclusion 到统一结果
ArrayDetectErrorCode mergeOwnedConclusion (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  FieldOwnedConclusion* owned_conclusion
);

// 合并 capacity association 到统一结果
ArrayDetectErrorCode mergeCapacityAssociation (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  PointerCapacityAssociation* capacity_result
);

// 合并 array access 到统一结果
ArrayDetectErrorCode mergeArrayAccesses (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  TypeFieldArrayAccesses* accesses
);

// 计算综合置信度
unsigned int calculateOverallConfidence (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result
);

// 生成汇总描述
char const* generateSummaryDescription (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result
);

// 调试输出
void printUnifiedResult (
  AD_FUNC_ARGS,
  FILE* out,
  UnifiedFieldAnalysisResult* result
);

void printAllUnifiedResults (
  AD_FUNC_ARGS,
  FILE* out,
  vec<UnifiedFieldAnalysisResult*, va_gc>* results
);

// 写入统一格式的 Racket datum
ArrayDetectErrorCode writeUnifiedResultsToRacketDatum (
  AD_FUNC_ARGS,
  vec<UnifiedFieldAnalysisResult*, va_gc>* results
);

} // namespace array_detect_ns
