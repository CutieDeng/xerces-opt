#pragma once

// ============================================================================
// ad-capacity-conclude 模块
// ============================================================================
// 汇总三种证据（malloc/read/write），生成最终容量结论
//
// 数据流：
//   pointer-field, (listof malloc-evidence), (listof read-evidence), (listof write-evidence)
//     -> capacity-conclude
//
// 汇总三种证据来源，选择最佳关联候选
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-wrapper.hh"

// 前置声明
namespace array_detect_ns {
  struct MallocCapacityEvidence;
  struct ReadCapacityEvidence;
  struct WriteCapacityEvidence;
}

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 容量关联判定
// ============================================================================

enum CapacityVerdict {
  CAP_VERDICT_ASSOCIATED,       // 确定关联
  CAP_VERDICT_NOT_ASSOCIATED,   // 确定不关联
  CAP_VERDICT_UNKNOWN           // 未知
};

// ============================================================================
// 容量结论
// ============================================================================
// (capacity-conclude
//   type               : tree
//   pointer_field      : tree                                ; 指针字段
//   malloc_evidences   : (listof malloc-capacity-evidence)   ; malloc 证据
//   read_evidences     : (listof read-capacity-evidence)     ; 读边界证据
//   write_evidences    : (listof write-capacity-evidence)    ; 写边界证据
//   best_candidate     : (or tree #f)                        ; 最佳关联候选
//   verdict            : (or 'associated 'not-associated 'unknown))

struct CapacityConclude {
  // === 标识 ===
  tree type;                      // 类型
  tree pointer_field;             // 指针字段 (FIELD_DECL)
  char const* type_name;          // 类型名
  char const* pointer_field_name; // 指针字段名

  // === 三种证据 ===
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences;  // malloc 证据
  vec<ReadCapacityEvidence*, va_gc>* read_evidences;      // 读证据
  vec<WriteCapacityEvidence*, va_gc>* write_evidences;    // 写证据

  // === 统计 ===
  unsigned int malloc_evidence_count;   // malloc 证据数
  unsigned int read_evidence_count;     // 读证据数
  unsigned int write_evidence_count;    // 写证据数
  unsigned int total_evidence_count;    // 总证据数

  // === 候选统计（按整数字段分组）===
  struct CandidateScore {
    tree integer_field;             // 整数字段
    unsigned int malloc_score;      // malloc 得分
    unsigned int read_score;        // 读得分
    unsigned int write_score;       // 写得分
    unsigned int total_score;       // 总得分
  };
  vec<CandidateScore*, va_gc>* candidate_scores;  // 候选得分列表

  // === 最终判定 ===
  tree best_candidate;              // 最佳关联候选 (FIELD_DECL)
  char const* best_candidate_name;  // 最佳候选名称
  CapacityVerdict verdict;          // 判定结果

  // === 描述 ===
  char const* conclusion_description;
};

// ============================================================================
// 函数声明
// ============================================================================

// 汇总容量结论
// 输入：type, pointer_field, 三种证据列表
// 输出：CapacityConclude*
ArrayDetectErrorCode summarizeCapacityConclude (
  AD_FUNC_ARGS,
  tree type,
  tree pointer_field,
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences,
  vec<ReadCapacityEvidence*, va_gc>* read_evidences,
  vec<WriteCapacityEvidence*, va_gc>* write_evidences,
  CapacityConclude** result
);

// 从 TypeFieldAnalysisData 生成容量结论
// 输入：tfad (已填充三种证据)
// 输出：tfad->capacity_conclude
ArrayDetectErrorCode generateCapacityConclude (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_TransferStats* tfad
);

// 打印容量结论
void printCapacityConclude (
  AD_FUNC_ARGS,
  FILE* out,
  CapacityConclude* conclude
);

// 打印所有容量结论
void printAllCapacityConcludes (
  AD_FUNC_ARGS,
  FILE* out,
  vec<CapacityConclude*, va_gc>* concludes
);

// 获取判定名称
char const* capacityVerdictToString (CapacityVerdict verdict);

} // namespace array_detect_ns
