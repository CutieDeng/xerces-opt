#include "owned-conclusion.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"
#include "field-source-variant.hh"
#include "source-escape-collection.hh"
#include "escape-synthesizer.hh"
#include "ownership-transfer-analysis.hh"
#include "info-print.hh"
#include "string-utils.hh"

#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：获取源类型的描述（从 Wrapper 读取）
// ============================================================================

static char const* getSourceTypeDescriptionFromKind (field_analysis::FieldSourceKind source_kind) {
  switch (source_kind) {
    case field_analysis::FIELD_SRC_FUNCTION_CALL: return "function call";
    case field_analysis::FIELD_SRC_FIELD_ACCESS: return "field access";
    case field_analysis::FIELD_SRC_CONSTANT: return "constant";
    case field_analysis::FIELD_SRC_COMPUTATION: return "computation";
    case field_analysis::FIELD_SRC_PHI: return "phi node";
    case field_analysis::FIELD_SRC_UNKNOWN: return "unknown";
    default: return "unspecified";
  }
}

// 向后兼容：旧的 FieldSourceType 版本
static char const* getSourceTypeDescription (FieldSourceType source_type) {
  switch (source_type) {
    case SOURCE_FUNCTION_CALL: return "function call";
    case SOURCE_FIELD_ACCESS: return "field access";
    case SOURCE_CONSTANT: return "constant";
    case SOURCE_COMPUTATION: return "computation";
    case SOURCE_PHI: return "phi node";
    case SOURCE_UNKNOWN: return "unknown";
    default: return "unspecified";
  }
}

// ============================================================================
// 源类型分类（细化）- 从 Wrapper 读取
// ============================================================================

SourceTypeCategory categorizeSourceTypeFromWrapper (FieldWriteAnalysisWrapper* wrapper) {
  if (!wrapper) {
    return SRC_CAT_UNSUPPORTED;
  }

  switch (wrapper->source_kind) {
    case field_analysis::FIELD_SRC_FUNCTION_CALL:
      return SRC_CAT_ALLOCATION;

    case field_analysis::FIELD_SRC_FIELD_ACCESS:
      return SRC_CAT_TRANSFER;

    case field_analysis::FIELD_SRC_CONSTANT:
      // 区分 NULL 和其他常量
      {
        tree constant_value = wrapper->source_data.constant.value;
        if (constant_value && integer_zerop (constant_value)) {
          return SRC_CAT_NEUTRAL;  // NULL → 中性，不影响判定
        }
      }
      return SRC_CAT_UNSUPPORTED;

    case field_analysis::FIELD_SRC_COMPUTATION:
    case field_analysis::FIELD_SRC_PHI:
    case field_analysis::FIELD_SRC_UNKNOWN:
    default:
      return SRC_CAT_UNSUPPORTED;
  }
}

// 向后兼容：旧的 FieldSourceInfo* 版本
SourceTypeCategory categorizeSourceType (FieldSourceInfo* source_info) {
  if (!source_info) {
    return SRC_CAT_UNSUPPORTED;
  }

  switch (source_info->source_type) {
    case SOURCE_FUNCTION_CALL:
      return SRC_CAT_ALLOCATION;

    case SOURCE_FIELD_ACCESS:
      return SRC_CAT_TRANSFER;

    case SOURCE_CONSTANT:
      // 区分 NULL 和其他常量
      {
        tree constant_value = source_info->data.constant.constant_value;
        if (constant_value && integer_zerop (constant_value)) {
          return SRC_CAT_NEUTRAL;  // NULL → 中性，不影响判定
        }
      }
      return SRC_CAT_UNSUPPORTED;

    case SOURCE_COMPUTATION:
    case SOURCE_PHI:
    case SOURCE_UNKNOWN:
    default:
      return SRC_CAT_UNSUPPORTED;
  }
}

// ============================================================================
// 字符串转换函数
// ============================================================================

char const* rejectionReasonToString (RejectionReason r) {
  switch (r) {
    case REJECTION_NONE: return "none";
    case REJECTION_INVALID_SOURCE_TYPE: return "invalid source type";
    case REJECTION_REJECTING_ESCAPE: return "rejecting escape";
    case REJECTION_SHARED_OWNERSHIP: return "shared ownership";
    case REJECTION_NO_SOURCE_INFO: return "no source info";
    default: return "unknown";
  }
}

char const* verdictToString (OwnedConclusionVerdict v) {
  switch (v) {
    case OWNED_YES: return "YES";
    case OWNED_PARTIAL_YES: return "PARTIAL_YES";
    case OWNED_NO: return "NO";
    case OWNED_UNDETERMINED: return "UNDETERMINED";
    default: return "UNKNOWN";
  }
}

char const* fieldWriteCategoryToString (FieldWriteCategory c) {
  switch (c) {
    case FIELD_WRITE_CAT_UNKNOWN: return "unknown";
    case FIELD_WRITE_CAT_INVALID: return "invalid";
    case FIELD_WRITE_CAT_NEUTRAL: return "neutral";
    case FIELD_WRITE_CAT_SUPPORTING: return "supporting";
    case FIELD_WRITE_CAT_REJECTING: return "rejecting";
    default: return "unspecified";
  }
}

char const* sourceTypeCategoryToString (SourceTypeCategory c) {
  switch (c) {
    case SRC_CAT_ALLOCATION: return "allocation";
    case SRC_CAT_TRANSFER: return "transfer";
    case SRC_CAT_NEUTRAL: return "neutral";
    case SRC_CAT_UNSUPPORTED: return "unsupported";
    default: return "unknown";
  }
}

// ============================================================================
// 证据工厂函数（使用 Wrapper）
// ============================================================================

static OwnedRejectingEvidence* createRejectingEvidenceFromWrapper (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  RejectionReason reason
) {
  (void)ctx;
  (void)gcc_ctx;

  OwnedRejectingEvidence* ev = ggc_alloc<OwnedRejectingEvidence>();
  memset (ev, 0, sizeof (OwnedRejectingEvidence));

  ev->location = wrapper->write_location;
  ev->stmt = wrapper->stmt;
  ev->rejection_reason = rejectionReasonToString (reason);

  switch (reason) {
    case REJECTION_INVALID_SOURCE_TYPE:
      ev->has_invalid_source = true;
      ev->source_description = getSourceTypeDescriptionFromKind (wrapper->source_kind);
      break;

    case REJECTION_REJECTING_ESCAPE:
      ev->has_rejecting_escape = true;
      ev->rejecting_escapes = wrapper->rejecting_escapes;
      ev->total_escapes = wrapper->total_escapes;
      ev->source_description = getSourceTypeDescriptionFromKind (wrapper->source_kind);
      break;

    case REJECTION_SHARED_OWNERSHIP:
      ev->has_transfer_issue = true;
      ev->transfer_verdict_str = "IMPOSSIBLE (shared)";
      ev->source_description = getSourceTypeDescriptionFromKind (wrapper->source_kind);
      break;

    case REJECTION_NO_SOURCE_INFO:
      ev->has_invalid_source = true;
      ev->source_description = "no source info";
      break;

    default:
      break;
  }

  return ev;
}

static OwnedSupportingEvidence* createSupportingEvidenceFromWrapper (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper
) {
  (void)ctx;
  (void)gcc_ctx;

  OwnedSupportingEvidence* ev = ggc_alloc<OwnedSupportingEvidence>();
  memset (ev, 0, sizeof (OwnedSupportingEvidence));

  ev->location = wrapper->write_location;
  ev->stmt = wrapper->stmt;
  ev->source_description = getSourceTypeDescriptionFromKind (wrapper->source_kind);

  // 从 wrapper 的 EscapeConclude 部分读取
  ev->total_escapes = wrapper->total_escapes;
  ev->safe_debug_escapes = wrapper->safe_debug_escapes;

  // 从 wrapper 的 MoveAnalysis 部分读取
  if (wrapper->move) {
    ev->has_transfer_analysis = true;
    char const* transfer_str = "UNKNOWN";
    switch (wrapper->move->verdict) {
      case field_analysis::FIELD_MOVE_CERTAIN: transfer_str = "CERTAIN"; break;
      case field_analysis::FIELD_MOVE_IMPOSSIBLE: transfer_str = "IMPOSSIBLE"; break;
      case field_analysis::FIELD_MOVE_CONDITIONAL: transfer_str = "CONDITIONAL"; break;
      default: break;
    }
    ev->transfer_verdict_str = transfer_str;
  }

  return ev;
}

// 向后兼容版本（旧的 record 接口）
static OwnedRejectingEvidence* createRejectingEvidence (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  RejectionReason reason
) {
  // FieldWriteAnalysisRecord 现在是 FieldWriteAnalysisWrapper 的别名
  return createRejectingEvidenceFromWrapper (AD_ARGS, record, reason);
}

static OwnedSupportingEvidence* createSupportingEvidence (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record
) {
  // FieldWriteAnalysisRecord 现在是 FieldWriteAnalysisWrapper 的别名
  return createSupportingEvidenceFromWrapper (AD_ARGS, record);
}

// ============================================================================
// 聚合策略：计算最终判定
// ============================================================================
//
// 判定策略:
// - supporting > 0 且 rejecting == 0 → YES（确定 owned）
// - supporting > rejecting → PARTIAL_YES（可能 owned，调试用）
// - supporting <= rejecting 且 rejecting > 0 → NO（不是 owned）
// - supporting == 0 且 rejecting == 0 → UNDETERMINED（无法判定）

static OwnedConclusionVerdict computeVerdict (
  unsigned int supporting,
  unsigned int rejecting,
  unsigned int neutral
) {
  (void)neutral;  // 中性不参与判定

  if (supporting == 0 && rejecting == 0) {
    return OWNED_UNDETERMINED;
  }

  if (rejecting == 0 && supporting > 0) {
    return OWNED_YES;
  }

  if (supporting > rejecting) {
    return OWNED_PARTIAL_YES;
  }

  return OWNED_NO;
}

// ============================================================================
// 辅助函数：检查 Wrapper 的逃逸结论是否拒绝 owned
// ============================================================================
//
// Owned 语义要求：指针的唯一所有者，不与其他代码共享
// 简化判定：存在非调试逃逸 => 拒绝 owned

static bool isWrapperEscapeRejecting (FieldWriteAnalysisWrapper* wrapper) {
  if (!wrapper) {
    return false;
  }
  return wrapper->has_rejecting_evidence;
}

// 向后兼容
static bool isEscapeEvidenceRejecting (EscapeEvidenceResult* evidence) {
  if (!evidence) {
    return false;
  }
  return evidence->has_rejecting_evidence;
}

// ============================================================================
// 辅助函数：检查 Wrapper 的所有权转移是否拒绝 owned
// ============================================================================

static bool isWrapperTransferRejecting (FieldWriteAnalysisWrapper* wrapper) {
  if (!wrapper || !wrapper->move) {
    // 没有所有权转移分析，保守起见不拒绝
    return false;
  }

  // FIELD_MOVE_IMPOSSIBLE 表示共享所有权，拒绝 owned
  return wrapper->move->verdict == field_analysis::FIELD_MOVE_IMPOSSIBLE;
}

// 向后兼容
static bool isTransferResultRejecting (OwnershipTransferAnalysisResult* transfer) {
  if (!transfer) {
    // 没有所有权转移分析，保守起见不拒绝
    return false;
  }

  // TRANSFER_IMPOSSIBLE 表示共享所有权，拒绝 owned
  return transfer->verdict == TRANSFER_IMPOSSIBLE;
}

// ============================================================================
// 核心函数：分析单个字段写入 Wrapper 是否支持 owned
// ============================================================================
//
// 重构后的简化版本：
// 1. 使用 FieldWriteOwnedAnalysisResult 返回结构化结果
// 2. 使用 categorizeSourceTypeFromWrapper() 细化源类型分类
// 3. 中性源 (NULL 赋值) 不产生证据，不影响判定
// 4. 使用工厂函数创建证据对象

static ArrayDetectErrorCode analyzeFieldWriteWrapperForOwned (
  AD_FUNC_ARGS,
  FieldWriteAnalysisWrapper* wrapper,
  FieldWriteOwnedAnalysisResult& result
) AD_FUNCTION_BEGIN {
  result.category = FIELD_WRITE_CAT_UNKNOWN;
  result.rejection_reason = REJECTION_NONE;
  result.evidence = NULL;

  // Step 0: 基本有效性检查
  if (!wrapper) {
    result.category = FIELD_WRITE_CAT_INVALID;
    AD_RETURNE (OK);
  }

  // Step 1: 检查源信息（通过 source_kind 判断）
  if (wrapper->source_kind == field_analysis::FIELD_SRC_UNKNOWN) {
    result.category = FIELD_WRITE_CAT_REJECTING;
    result.rejection_reason = REJECTION_NO_SOURCE_INFO;
    result.evidence = createRejectingEvidenceFromWrapper (AD_ARGS, wrapper, REJECTION_NO_SOURCE_INFO);
    AD_RETURNE (OK);
  }

  // Step 2: 分类源类型
  SourceTypeCategory src_cat = categorizeSourceTypeFromWrapper (wrapper);

  // Step 3: 中性源直接跳过（不产生证据）
  if (src_cat == SRC_CAT_NEUTRAL) {
    result.category = FIELD_WRITE_CAT_NEUTRAL;
    AD_RETURNE (OK);
  }

  // Step 4: 不支持的源类型 → 拒绝
  if (src_cat == SRC_CAT_UNSUPPORTED) {
    result.category = FIELD_WRITE_CAT_REJECTING;
    result.rejection_reason = REJECTION_INVALID_SOURCE_TYPE;
    result.evidence = createRejectingEvidenceFromWrapper (AD_ARGS, wrapper, REJECTION_INVALID_SOURCE_TYPE);
    AD_RETURNE (OK);
  }

  // Step 5: 检查逃逸证据
  bool escape_ok = !isWrapperEscapeRejecting (wrapper);
  if (!escape_ok) {
    result.category = FIELD_WRITE_CAT_REJECTING;
    result.rejection_reason = REJECTION_REJECTING_ESCAPE;
    result.evidence = createRejectingEvidenceFromWrapper (AD_ARGS, wrapper, REJECTION_REJECTING_ESCAPE);
    AD_RETURNE (OK);
  }

  // Step 6: 检查所有权转移（仅对 TRANSFER 类）
  if (src_cat == SRC_CAT_TRANSFER) {
    bool transfer_ok = !isWrapperTransferRejecting (wrapper);
    if (!transfer_ok) {
      result.category = FIELD_WRITE_CAT_REJECTING;
      result.rejection_reason = REJECTION_SHARED_OWNERSHIP;
      result.evidence = createRejectingEvidenceFromWrapper (AD_ARGS, wrapper, REJECTION_SHARED_OWNERSHIP);
      AD_RETURNE (OK);
    }
  }

  // Step 7: 全部通过 → 支持
  result.category = FIELD_WRITE_CAT_SUPPORTING;
  result.evidence = createSupportingEvidenceFromWrapper (AD_ARGS, wrapper);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 向后兼容（FieldWriteAnalysisRecord 现在是 FieldWriteAnalysisWrapper 的别名）
static ArrayDetectErrorCode analyzeFieldWriteForOwned (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  FieldWriteOwnedAnalysisResult& result
) AD_FUNCTION_BEGIN {
  AD_TRY (analyzeFieldWriteWrapperForOwned (AD_ARGS, record, result));
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 核心函数：分析单个字段的 owned 结论
// ============================================================================
//
// 重构后版本：
// 1. 使用 FieldWriteOwnedAnalysisResult 结构化结果
// 2. 区分中性/支持/拒绝三类证据
// 3. 使用 computeVerdict() 计算最终判定（容错一票否决策略）

ArrayDetectErrorCode analyzeFieldOwnedConclusion (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData* field_data,
  FieldOwnedConclusion*& result
) AD_FUNCTION_BEGIN {
  if (!field_data) {
    AD_DEBUG_PRINT ("Warning: field_data is NULL in analyzeFieldOwnedConclusion");
    result = NULL;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  result = NULL;

  // 验证必要的字段 - 这些字段为 NULL 是数据完整性错误
  if (!field_data->type) {
    AD_DEBUG_PRINT ("Error: field_data->type is NULL (data integrity violation)");
    AD_RETURNE (INVALID_ARGUMENT);
  }

  if (!field_data->field_decl) {
    AD_DEBUG_PRINT ("Error: field_data->field_decl is NULL (data integrity violation)");
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 创建结论结构
  FieldOwnedConclusion* conclusion = ggc_alloc<FieldOwnedConclusion>();
  memset (conclusion, 0, sizeof (FieldOwnedConclusion));

  conclusion->type = field_data->type;
  conclusion->field_decl = field_data->field_decl;
  conclusion->type_name = safeGetTypeName (AD_ARGS, field_data->type);
  conclusion->field_name = safeGetFieldName (AD_ARGS, field_data->field_decl);

  // 初始化证据列表
  vec_alloc (conclusion->supporting_evidences, 4);
  vec_alloc (conclusion->rejecting_evidences, 4);

  // 遍历所有字段写入 Wrapper
  if (!field_data->writes) {
    conclusion->verdict = OWNED_UNDETERMINED;
    conclusion->conclusion_description = "No field write operations found";
    AD_RETURNO (conclusion);
  }

  unsigned int total_field_writes = field_data->writes->length ();
  conclusion->total_field_writes = total_field_writes;

  unsigned int supporting_count = 0;
  unsigned int rejecting_count = 0;
  unsigned int neutral_count = 0;

  for (unsigned int i = 0; i < total_field_writes; i++) {
    FieldWriteAnalysisWrapper* wrapper = (*field_data->writes)[i];
    if (!wrapper) continue;

    FieldWriteOwnedAnalysisResult field_write_result;
    AD_TRY (analyzeFieldWriteWrapperForOwned (AD_ARGS, wrapper, field_write_result));

    switch (field_write_result.category) {
      case FIELD_WRITE_CAT_SUPPORTING:
        if (field_write_result.evidence) {
          vec_safe_push (conclusion->supporting_evidences,
                         static_cast<OwnedSupportingEvidence*>(field_write_result.evidence));
        }
        supporting_count++;
        break;

      case FIELD_WRITE_CAT_REJECTING:
        if (field_write_result.evidence) {
          vec_safe_push (conclusion->rejecting_evidences,
                         static_cast<OwnedRejectingEvidence*>(field_write_result.evidence));
        }
        rejecting_count++;
        break;

      case FIELD_WRITE_CAT_NEUTRAL:
        // 中性不产生证据，仅计数
        neutral_count++;
        break;

      case FIELD_WRITE_CAT_INVALID:
      case FIELD_WRITE_CAT_UNKNOWN:
      default:
        // 无效/未知记录不计入
        break;
    }
  }

  conclusion->supporting_field_writes_count = supporting_count;
  conclusion->rejecting_field_writes_count = rejecting_count;

  // 使用新的聚合策略确定最终判定
  conclusion->verdict = computeVerdict (supporting_count, rejecting_count, neutral_count);

  // 生成结论描述
  switch (conclusion->verdict) {
    case OWNED_YES:
      conclusion->conclusion_description = "Field is owned pointer (all evidence supports, no rejection)";
      break;

    case OWNED_PARTIAL_YES:
      conclusion->conclusion_description = "Field may be owned pointer (supporting > rejecting, for debugging)";
      break;

    case OWNED_NO:
      conclusion->conclusion_description = "Field cannot be owned pointer (rejecting >= supporting)";
      break;

    case OWNED_UNDETERMINED:
    default:
      if (neutral_count > 0) {
        conclusion->conclusion_description = "Cannot determine (only neutral operations, e.g., NULL assignments)";
      } else {
        conclusion->conclusion_description = "Cannot determine (no valid evidence)";
      }
      break;
  }

  AD_RETURNO (conclusion);
} AD_FUNCTION_END

// ============================================================================
// 集成函数：分析所有字段的 owned 结论
// ============================================================================

ArrayDetectErrorCode analyzeAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<FieldOwnedConclusion*, va_gc>*& result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing all field owned conclusions");

  result = NULL;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  vec<FieldOwnedConclusion*, va_gc>* conclusions = NULL;
  vec_alloc (conclusions, 16);

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  unsigned int total_fields = 0;
  unsigned int owned_yes = 0;
  unsigned int owned_partial_yes = 0;
  unsigned int owned_no = 0;
  unsigned int undetermined = 0;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps* tfwo = (*iter).second;
    if (!tfwo) continue;

    total_fields++;

    FieldOwnedConclusion* conclusion = NULL;
    AD_TRY (analyzeFieldOwnedConclusion (AD_ARGS, tfwo, conclusion));

    if (conclusion) {
      vec_safe_push (conclusions, conclusion);

      switch (conclusion->verdict) {
        case OWNED_YES:
          owned_yes++;
          break;
        case OWNED_PARTIAL_YES:
          owned_partial_yes++;
          break;
        case OWNED_NO:
          owned_no++;
          break;
        case OWNED_UNDETERMINED:
          undetermined++;
          break;
      }
    }
  }

  AD_DEBUG_PRINT ("Field owned conclusion analysis complete: %u fields, %u YES, %u PARTIAL_YES, %u NO, %u undetermined",
                  total_fields, owned_yes, owned_partial_yes, owned_no, undetermined);

  AD_RETURNO (conclusions);
} AD_FUNCTION_END

// ============================================================================
// 调试输出：打印单个字段的 owned 结论
// ============================================================================

void printFieldOwnedConclusion (
  AD_FUNC_ARGS,
  FILE* out,
  FieldOwnedConclusion* conclusion
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!conclusion || !out) {
    return;
  }

  fprintf (out, "\n");
  fprintf (out, "=== Field Owned Conclusion ===\n");
  fprintf (out, "Type: %s\n", conclusion->type_name);
  fprintf (out, "Field: %s\n", conclusion->field_name);
  fprintf (out, "\n");

  char const* verdict_str = "UNDETERMINED";
  switch (conclusion->verdict) {
    case OWNED_YES: verdict_str = "YES (owned)"; break;
    case OWNED_PARTIAL_YES: verdict_str = "PARTIAL_YES (maybe owned, for debugging)"; break;
    case OWNED_NO: verdict_str = "NO (not owned)"; break;
    case OWNED_UNDETERMINED: verdict_str = "UNDETERMINED"; break;
  }

  fprintf (out, "Verdict: %s\n", verdict_str);
  fprintf (out, "Description: %s\n", conclusion->conclusion_description);
  fprintf (out, "\n");

  fprintf (out, "Statistics:\n");
  fprintf (out, "  Total field writes: %u\n", conclusion->total_field_writes);
  fprintf (out, "  Supporting field writes: %u\n", conclusion->supporting_field_writes_count);
  fprintf (out, "  Rejecting field writes: %u\n", conclusion->rejecting_field_writes_count);
  fprintf (out, "\n");

  // 打印支持证据
  if (conclusion->supporting_evidences && conclusion->supporting_evidences->length () > 0) {
    fprintf (out, "Supporting Evidence:\n");
    for (unsigned int i = 0; i < conclusion->supporting_evidences->length (); i++) {
      OwnedSupportingEvidence* evidence = (*conclusion->supporting_evidences)[i];
      if (!evidence) continue;

      fprintf (out, "  [%u] %s:%d:%d\n",
               i,
               LOCATION_FILE (evidence->location),
               LOCATION_LINE (evidence->location),
               LOCATION_COLUMN (evidence->location));
      fprintf (out, "      Source: %s\n", evidence->source_description);
      fprintf (out, "      Escapes: %u (safe debug: %u)\n",
               evidence->total_escapes,
               evidence->safe_debug_escapes);
      if (evidence->has_transfer_analysis) {
        fprintf (out, "      Ownership transfer: %s\n", evidence->transfer_verdict_str);
      }
    }
    fprintf (out, "\n");
  }

  // 打印拒绝证据
  if (conclusion->rejecting_evidences && conclusion->rejecting_evidences->length () > 0) {
    fprintf (out, "Rejecting Evidence:\n");
    for (unsigned int i = 0; i < conclusion->rejecting_evidences->length (); i++) {
      OwnedRejectingEvidence* evidence = (*conclusion->rejecting_evidences)[i];
      if (!evidence) continue;

      fprintf (out, "  [%u] %s:%d:%d\n",
               i,
               LOCATION_FILE (evidence->location),
               LOCATION_LINE (evidence->location),
               LOCATION_COLUMN (evidence->location));
      fprintf (out, "      Reason: %s\n", evidence->rejection_reason);

      if (evidence->has_invalid_source) {
        fprintf (out, "      Source type: %s\n", evidence->source_description);
      }

      if (evidence->has_rejecting_escape) {
        fprintf (out, "      Rejecting escapes: %u / %u total\n",
                 evidence->rejecting_escapes,
                 evidence->total_escapes);
      }

      if (evidence->has_transfer_issue) {
        fprintf (out, "      Ownership transfer: %s\n", evidence->transfer_verdict_str);
      }
    }
    fprintf (out, "\n");
  }

  fprintf (out, "==============================\n");
}

// ============================================================================
// 调试输出：打印所有字段的 owned 结论
// ============================================================================

void printAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  FILE* out,
  vec<FieldOwnedConclusion*, va_gc>* conclusions
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out) {
    return;
  }

  if (!conclusions || conclusions->length () == 0) {
    fprintf (out, "\nNo field owned conclusions to print.\n");
    return;
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "                    FIELD OWNED CONCLUSION ANALYSIS RESULTS\n");
  fprintf (out, "================================================================================\n");

  unsigned int yes_count = 0;
  unsigned int partial_yes_count = 0;
  unsigned int no_count = 0;
  unsigned int undetermined_count = 0;

  for (unsigned int i = 0; i < conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*conclusions)[i];
    if (!conclusion) continue;

    switch (conclusion->verdict) {
      case OWNED_YES: yes_count++; break;
      case OWNED_PARTIAL_YES: partial_yes_count++; break;
      case OWNED_NO: no_count++; break;
      case OWNED_UNDETERMINED: undetermined_count++; break;
    }

    printFieldOwnedConclusion (AD_ARGS, out, conclusion);
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "Summary:\n");
  fprintf (out, "  Total fields analyzed: %u\n", conclusions->length ());
  fprintf (out, "  YES (owned): %u\n", yes_count);
  fprintf (out, "  PARTIAL_YES (maybe owned): %u\n", partial_yes_count);
  fprintf (out, "  NO (not owned): %u\n", no_count);
  fprintf (out, "  UNDETERMINED: %u\n", undetermined_count);
  fprintf (out, "================================================================================\n");
  fprintf (out, "\n");
}

} // namespace array_detect_ns
