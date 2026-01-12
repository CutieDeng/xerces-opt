// ============================================================================
// ad-capacity-conclude 模块实现
// ============================================================================
// 汇总三种证据（malloc/read/write），生成最终容量结论
// ============================================================================

#include "capacity-conclude.hh"
#include "malloc-capacity.hh"
#include "array-read-bound.hh"
#include "array-write-bound.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 获取判定名称
// ============================================================================

char const* capacityVerdictToString (CapacityVerdict verdict) {
  switch (verdict) {
    case CAP_VERDICT_ASSOCIATED:     return "associated";
    case CAP_VERDICT_NOT_ASSOCIATED: return "not-associated";
    case CAP_VERDICT_UNKNOWN:        return "unknown";
    default:                         return "?";
  }
}

// ============================================================================
// 辅助函数：查找或创建候选得分
// ============================================================================

static CapacityConclude::CandidateScore* findOrCreateScore (
  vec<CapacityConclude::CandidateScore*, va_gc>*& scores,
  tree integer_field
) {
  if (!scores) {
    vec_alloc (scores, 4);
  }

  // 查找现有
  for (unsigned i = 0; i < scores->length (); i++) {
    if ((*scores)[i]->integer_field == integer_field) {
      return (*scores)[i];
    }
  }

  // 创建新的
  CapacityConclude::CandidateScore* score = ggc_alloc<CapacityConclude::CandidateScore>();
  memset (score, 0, sizeof (CapacityConclude::CandidateScore));
  score->integer_field = integer_field;
  vec_safe_push (scores, score);
  return score;
}

// ============================================================================
// 汇总容量结论
// ============================================================================

ArrayDetectErrorCode summarizeCapacityConclude (
  AD_FUNC_ARGS,
  tree type,
  tree pointer_field,
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences,
  vec<ReadCapacityEvidence*, va_gc>* read_evidences,
  vec<WriteCapacityEvidence*, va_gc>* write_evidences,
  CapacityConclude** result
) AD_FUNCTION_BEGIN {
  *result = NULL;

  CapacityConclude* conclude = ggc_alloc<CapacityConclude>();
  memset (conclude, 0, sizeof (CapacityConclude));

  // 基本信息
  conclude->type = type;
  conclude->pointer_field = pointer_field;
  conclude->type_name = safeGetTypeName (AD_ARGS, type);
  conclude->pointer_field_name = safeGetFieldName (AD_ARGS, pointer_field);

  // 证据引用
  conclude->malloc_evidences = malloc_evidences;
  conclude->read_evidences = read_evidences;
  conclude->write_evidences = write_evidences;

  // 统计证据数量
  conclude->malloc_evidence_count = malloc_evidences ? malloc_evidences->length () : 0;
  conclude->read_evidence_count = read_evidences ? read_evidences->length () : 0;
  conclude->write_evidence_count = write_evidences ? write_evidences->length () : 0;
  conclude->total_evidence_count =
    conclude->malloc_evidence_count +
    conclude->read_evidence_count +
    conclude->write_evidence_count;

  // 收集候选得分
  vec<CapacityConclude::CandidateScore*, va_gc>* scores = NULL;

  // 处理 malloc 证据
  if (malloc_evidences) {
    for (unsigned i = 0; i < malloc_evidences->length (); i++) {
      MallocCapacityEvidence* ev = (*malloc_evidences)[i];
      if (!ev || !ev->integer_field) continue;

      CapacityConclude::CandidateScore* score = findOrCreateScore (scores, ev->integer_field);
      score->malloc_score += 3;  // malloc 证据权重较高
      score->total_score += 3;
    }
  }

  // 处理 read 证据
  if (read_evidences) {
    for (unsigned i = 0; i < read_evidences->length (); i++) {
      ReadCapacityEvidence* ev = (*read_evidences)[i];
      if (!ev || !ev->integer_field) continue;

      CapacityConclude::CandidateScore* score = findOrCreateScore (scores, ev->integer_field);
      score->read_score += 2;
      score->total_score += 2;
    }
  }

  // 处理 write 证据
  if (write_evidences) {
    for (unsigned i = 0; i < write_evidences->length (); i++) {
      WriteCapacityEvidence* ev = (*write_evidences)[i];
      if (!ev || !ev->integer_field) continue;

      CapacityConclude::CandidateScore* score = findOrCreateScore (scores, ev->integer_field);
      score->write_score += 2;
      score->total_score += 2;
    }
  }

  conclude->candidate_scores = scores;

  // 选择最佳候选
  tree best = NULL_TREE;
  unsigned int best_score = 0;

  if (scores) {
    for (unsigned i = 0; i < scores->length (); i++) {
      CapacityConclude::CandidateScore* s = (*scores)[i];
      if (s->total_score > best_score) {
        best_score = s->total_score;
        best = s->integer_field;
      }
    }
  }

  conclude->best_candidate = best;
  conclude->best_candidate_name = best ? safeGetFieldName (AD_ARGS, best) : NULL;

  // 判定结果
  if (best && best_score >= 2) {
    conclude->verdict = CAP_VERDICT_ASSOCIATED;
    conclude->conclusion_description = "Capacity field found with strong evidence";
  } else if (conclude->total_evidence_count == 0) {
    conclude->verdict = CAP_VERDICT_UNKNOWN;
    conclude->conclusion_description = "No evidence found";
  } else {
    conclude->verdict = CAP_VERDICT_NOT_ASSOCIATED;
    conclude->conclusion_description = "Insufficient evidence for association";
  }

  AD_DEBUG_PRINT ("[capacity-conclude] %s.%s -> %s (%s)",
                  conclude->type_name ? conclude->type_name : "?",
                  conclude->pointer_field_name ? conclude->pointer_field_name : "?",
                  conclude->best_candidate_name ? conclude->best_candidate_name : "(none)",
                  capacityVerdictToString (conclude->verdict));

  *result = conclude;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 从 TypeFieldAnalysisData 生成容量结论
// ============================================================================

ArrayDetectErrorCode generateCapacityConclude (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
) AD_FUNCTION_BEGIN {
  if (!tfad) {
    AD_RETURNE (OK);
  }

  // 从 hashmap 提取 flat list 用于 summarize
  // Malloc evidences
  vec<MallocCapacityEvidence*, va_gc>* malloc_evidences_flat = NULL;
  if (tfad->malloc_evidences_map) {
    for (auto iter = tfad->malloc_evidences_map->begin ();
         iter != tfad->malloc_evidences_map->end ();
         ++iter) {
      vec<MallocCapacityEvidence*, va_gc>* evidences = (*iter).second;
      if (evidences) {
        for (unsigned i = 0; i < evidences->length (); i++) {
          vec_safe_push (malloc_evidences_flat, (*evidences)[i]);
        }
      }
    }
  }

  // Read evidences
  vec<ReadCapacityEvidence*, va_gc>* read_evidences_flat = NULL;
  if (tfad->read_evidences_map) {
    for (auto iter = tfad->read_evidences_map->begin ();
         iter != tfad->read_evidences_map->end ();
         ++iter) {
      vec<ReadCapacityEvidence*, va_gc>* evidences = (*iter).second;
      if (evidences) {
        for (unsigned i = 0; i < evidences->length (); i++) {
          vec_safe_push (read_evidences_flat, (*evidences)[i]);
        }
      }
    }
  }

  // Write evidences
  vec<WriteCapacityEvidence*, va_gc>* write_evidences_flat = NULL;
  if (tfad->write_evidences_map) {
    for (auto iter = tfad->write_evidences_map->begin ();
         iter != tfad->write_evidences_map->end ();
         ++iter) {
      vec<WriteCapacityEvidence*, va_gc>* evidences = (*iter).second;
      if (evidences) {
        for (unsigned i = 0; i < evidences->length (); i++) {
          vec_safe_push (write_evidences_flat, (*evidences)[i]);
        }
      }
    }
  }

  AD_TRY (summarizeCapacityConclude (
    AD_ARGS,
    tfad->type,
    tfad->field_decl,
    malloc_evidences_flat,
    read_evidences_flat,
    write_evidences_flat,
    &tfad->capacity_conclude
  ));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 打印容量结论
// ============================================================================

void printCapacityConclude (
  AD_FUNC_ARGS,
  FILE* out,
  CapacityConclude* conclude
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out || !conclude) return;

  fprintf (out, "=== Capacity Conclude: %s.%s ===\n",
           conclude->type_name ? conclude->type_name : "?",
           conclude->pointer_field_name ? conclude->pointer_field_name : "?");

  fprintf (out, "  Evidence counts: malloc=%u, read=%u, write=%u, total=%u\n",
           conclude->malloc_evidence_count,
           conclude->read_evidence_count,
           conclude->write_evidence_count,
           conclude->total_evidence_count);

  if (conclude->candidate_scores) {
    fprintf (out, "  Candidate scores:\n");
    for (unsigned i = 0; i < conclude->candidate_scores->length (); i++) {
      CapacityConclude::CandidateScore* s = (*conclude->candidate_scores)[i];
      char const* name = safeGetFieldName (AD_ARGS, s->integer_field);
      fprintf (out, "    %s: malloc=%u, read=%u, write=%u, total=%u\n",
               name ? name : "?",
               s->malloc_score, s->read_score, s->write_score, s->total_score);
    }
  }

  fprintf (out, "  Best candidate: %s\n",
           conclude->best_candidate_name ? conclude->best_candidate_name : "(none)");
  fprintf (out, "  Verdict: %s\n",
           capacityVerdictToString (conclude->verdict));
  fprintf (out, "  Conclusion: %s\n",
           conclude->conclusion_description ? conclude->conclusion_description : "");
}

// ============================================================================
// 打印所有容量结论
// ============================================================================

void printAllCapacityConcludes (
  AD_FUNC_ARGS,
  FILE* out,
  vec<CapacityConclude*, va_gc>* concludes
) {
  if (!out || !concludes) return;

  fprintf (out, "\n====== All Capacity Conclusions (%u) ======\n",
           concludes->length ());

  for (unsigned i = 0; i < concludes->length (); i++) {
    printCapacityConclude (AD_ARGS, out, (*concludes)[i]);
    fprintf (out, "\n");
  }
}

} // namespace array_detect_ns
