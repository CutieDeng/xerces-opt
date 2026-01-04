#pragma once

#include "prelude.hh"
#include "context.hh"
#include "array-detector.hh"
#include "owned-conclusion.hh"
#include "array-access-collector.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 指针-容量关联分析
// ============================================================================
// 分析疑似数组指针字段是否有关联的容量/长度字段
// 通过检测：
// 1. malloc 分配分析：ptr = malloc(n * sizeof(T)) 中 n 是否来自某个整数字段
// 2. 访存条件判断：if (i < size) { x = ptr[i]; } 读操作前的边界检查
// 3. 写存条件判断：if (i < capacity) { ptr[i] = x; } 写操作前的边界检查

// ============================================================================
// 关联判定结果
// ============================================================================

enum CapacityAssociationVerdict {
  CAP_ASSOC_RELATED,       // 相关：有证据表明该整数字段与指针字段关联
  CAP_ASSOC_UNRELATED,     // 无关：没有发现关联证据
  CAP_ASSOC_UNDETERMINED   // 未确定：无法判断
};

// ============================================================================
// 证据类型（位图）
// ============================================================================

enum CapacityEvidenceType {
  CAP_EVID_NONE             = 0,
  CAP_EVID_MALLOC_SIZE_ARG  = 1 << 0,  // malloc size 参数中引用了该字段
  CAP_EVID_READ_CONDITION   = 1 << 1,  // 读操作前的边界检查条件中引用了该字段
  CAP_EVID_WRITE_CONDITION  = 1 << 2,  // 写操作前的边界检查条件中引用了该字段
};

// ============================================================================
// 证据详情
// ============================================================================

// 单条关联证据
struct CapacityAssociationEvidence {
  // === 证据类型 ===
  CapacityEvidenceType evidence_type;

  // === 位置信息 ===
  location_t location;                  // 证据位置
  gimple* stmt;                         // 相关语句

  // === 描述信息 ===
  const char* description;              // 证据描述

  // === 类型特定信息 ===
  // 对于 MALLOC_SIZE_ARG：
  const char* function_name;            // 分配函数名（malloc/calloc/realloc）
  tree size_expr;                       // size 表达式

  // 对于 READ_CONDITION / WRITE_CONDITION：
  tree condition_expr;                  // 条件表达式
  tree_code comparison_code;            // 比较操作符（LT_EXPR, LE_EXPR, GT_EXPR, GE_EXPR）
  tree access_stmt;                     // 访存语句
};

// ============================================================================
// 单个候选字段分析结果
// ============================================================================

struct CapacityCandidateAnalysis {
  // === 候选字段信息 ===
  tree field_decl;                      // 候选字段声明
  const char* field_name;               // 候选字段名称
  tree field_type;                      // 字段类型

  // === 判定结果 ===
  CapacityAssociationVerdict verdict;   // 关联判定
  unsigned int evidence_bitmap;         // 证据类型位图

  // === 证据列表 ===
  vec<CapacityAssociationEvidence*, va_gc>* evidences;
};

// ============================================================================
// 指针字段的容量关联结果
// ============================================================================

struct PointerCapacityAssociation {
  // === 指针字段信息 ===
  tree type;                            // 所属类型
  tree pointer_field_decl;              // 指针字段声明
  const char* type_name;                // 类型名称
  const char* pointer_field_name;       // 指针字段名称

  // === 候选分析结果 ===
  vec<CapacityCandidateAnalysis*, va_gc>* candidate_analyses;

  // === 统计信息 ===
  unsigned int related_count;           // 关联的候选字段数
  unsigned int unrelated_count;         // 无关的候选字段数
  unsigned int undetermined_count;      // 未确定的候选字段数

  // === 最佳匹配 ===
  CapacityCandidateAnalysis* best_match; // 最可能关联的字段（NULL 表示无）
};

// ============================================================================
// 函数声明
// ============================================================================

// 分析所有疑似 owned 指针字段的容量关联
// 输入：owned conclusions（筛选 verdict == OWNED_YES 或 OWNED_UNDETERMINED）
//       array_accesses（数组访问收集结果，用于 READ/WRITE_CONDITION 证据提取）
// 输出：所有指针字段的容量关联分析结果
ArrayDetectErrorCode analyzeAllCapacityAssociations (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses,
  vec<PointerCapacityAssociation*, va_gc>** out_results
);

// 分析单个指针字段的容量关联
ArrayDetectErrorCode analyzePointerCapacityAssociation (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  TypeFieldAnalysisData* pointer_field_data,
  TypeFieldArrayAccesses* array_accesses,  // 该指针字段的数组访问（可为 NULL）
  PointerCapacityAssociation** out_result
);

// 收集类型中的所有整数类型字段作为候选
ArrayDetectErrorCode collectIntegerCandidates (
  AD_FUNC_ARGS,
  tree type,
  vec<tree, va_gc>** out_candidates
);

// 检查表达式是否引用了指定字段
bool expressionReferencesField (
  AD_FUNC_ARGS,
  tree expr,
  tree field_decl
);

// 分析 malloc 调用的 size 参数是否引用了候选字段
ArrayDetectErrorCode analyzeMallocSizeSource (
  AD_FUNC_ARGS,
  gimple* call_stmt,
  tree candidate_field,
  bool* out_references,
  CapacityAssociationEvidence** out_evidence
);

// 打印指针容量关联结果
void printPointerCapacityAssociation (
  AD_FUNC_ARGS,
  FILE* out,
  PointerCapacityAssociation* result
);

// 打印所有指针容量关联结果
void printAllPointerCapacityAssociations (
  AD_FUNC_ARGS,
  FILE* out,
  vec<PointerCapacityAssociation*, va_gc>* results
);

// 将容量关联结果写入 Racket datum 格式的结果文件
// 格式: ((current-file "...") (type "...") (pointer-field "...") (capacity-field "...") (relation related|unrelated|undetermined) (evidence-types (...)))
ArrayDetectErrorCode writeCapacityAssociationsToRacketDatum (
  AD_FUNC_ARGS,
  vec<PointerCapacityAssociation*, va_gc>* results
);

} // namespace array_detect_ns
