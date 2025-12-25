#include "owned-conclusion.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"
#include "field-source-variant.hh"
#include "source-escape-collection.hh"
#include "escape-synthesizer.hh"
#include "ownership-transfer-analysis.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：判断源操作数类型是否支持 owned
// ============================================================================

static bool isSourceTypeSupportingOwned (FieldSourceType source_type) {
  switch (source_type) {
    case SOURCE_FUNCTION_CALL:
      // 函数调用返回值：支持 owned（通常是分配新内存）
      return true;

    case SOURCE_FIELD_ACCESS:
      // 字段访问：需要进一步检查所有权转移
      // 这里返回 true，由所有权转移分析决定
      return true;

    case SOURCE_CONSTANT:
      // 常量（如 NULL）：支持 owned
      return true;

    case SOURCE_VARIABLE:
      // 变量：可能支持，但通常不是 owned 模式
      // 保守起见返回 false
      return false;

    case SOURCE_COMPUTATION:
    case SOURCE_PHI:
    case SOURCE_UNKNOWN:
    default:
      // 计算、PHI、未知：不支持
      return false;
  }
}

// ============================================================================
// 辅助函数：获取源类型的描述
// ============================================================================

static const char* getSourceTypeDescription (FieldSourceType source_type) {
  switch (source_type) {
    case SOURCE_FUNCTION_CALL: return "function call";
    case SOURCE_FIELD_ACCESS: return "field access";
    case SOURCE_CONSTANT: return "constant";
    case SOURCE_VARIABLE: return "variable";
    case SOURCE_COMPUTATION: return "computation";
    case SOURCE_PHI: return "phi node";
    case SOURCE_UNKNOWN: return "unknown";
    default: return "unspecified";
  }
}

// ============================================================================
// 辅助函数：检查逃逸综合结果是否拒绝 owned
// ============================================================================
//
// Owned 语义要求：指针的唯一所有者，不与其他代码共享
//
// 拒绝 owned 的逃逸类别（指针逃逸到函数外部或被共享）：
// - ESC_SYNTH_RETURN_ESCAPE: 返回给调用者（共享给调用者）
// - ESC_SYNTH_GLOBAL_ESCAPE: 存储到全局变量（全局共享）
// - ESC_SYNTH_PARAMETER_ESCAPE: 存储到参数（共享给调用者）
// - ESC_SYNTH_FIELD_ESCAPE: 存储到多个字段（内部共享）
// - ESC_SYNTH_HEAP_ESCAPE: 存储到堆内存（逃逸到其他位置）
// - ESC_SYNTH_UNKNOWN_CALL: 传给未知函数（可能被保存）
// - ESC_SYNTH_VIRTUAL_CALL: 传给虚函数（可能被保存）
// - ESC_SYNTH_INDIRECT_CALL: 传给间接调用（可能被保存）
//
// 支持或中立的逃逸类别：
// - ESC_SYNTH_NO_ESCAPE: 无逃逸（支持 owned）
// - ESC_SYNTH_ARITHMETIC_POTENTIAL: 算术运算（可能只是地址计算）
// - ESC_SYNTH_SAFE_DEBUG: 调试型逃逸（如 printf，通常不保存指针）

static bool isEscapeSynthesisRejecting (EscapeSynthesisResult* synthesis) {
  if (!synthesis) {
    return false;
  }

  unsigned int categories = synthesis->category_bitmap;

  // 检查所有拒绝性逃逸类别
  if (categories & ESC_SYNTH_RETURN_ESCAPE) return true;
  if (categories & ESC_SYNTH_GLOBAL_ESCAPE) return true;
  if (categories & ESC_SYNTH_PARAMETER_ESCAPE) return true;
  if (categories & ESC_SYNTH_FIELD_ESCAPE) return true;
  if (categories & ESC_SYNTH_HEAP_ESCAPE) return true;
  if (categories & ESC_SYNTH_UNKNOWN_CALL) return true;
  if (categories & ESC_SYNTH_VIRTUAL_CALL) return true;
  if (categories & ESC_SYNTH_INDIRECT_CALL) return true;

  return false;
}

// ============================================================================
// 辅助函数：获取逃逸类别的描述
// ============================================================================

static const char* getEscapeCategoryDescription (unsigned int categories) {
  if (categories == 0) return "no escape";
  if (categories & ESC_SYNTH_RETURN_ESCAPE) return "return to caller";
  if (categories & ESC_SYNTH_GLOBAL_ESCAPE) return "store to global";
  if (categories & ESC_SYNTH_PARAMETER_ESCAPE) return "store to parameter";
  if (categories & ESC_SYNTH_FIELD_ESCAPE) return "multi-field escape";
  if (categories & ESC_SYNTH_VIRTUAL_CALL) return "virtual call";
  if (categories & ESC_SYNTH_INDIRECT_CALL) return "indirect call";
  if (categories & ESC_SYNTH_UNKNOWN_CALL) return "unknown call";
  if (categories & ESC_SYNTH_HEAP_ESCAPE) return "heap escape";
  return "other escape";
}

// ============================================================================
// 辅助函数：检查所有权转移结果是否拒绝 owned
// ============================================================================

static bool isTransferResultRejecting (OwnershipTransferAnalysisResult* transfer) {
  if (!transfer) {
    // 没有所有权转移分析，保守起见不拒绝
    return false;
  }

  // TRANSFER_IMPOSSIBLE 表示共享所有权，拒绝 owned
  return transfer->verdict == TRANSFER_IMPOSSIBLE;
}

// ============================================================================
// 核心函数：分析单个写入操作是否支持 owned
// ============================================================================

static ArrayDetectErrorCode analyzeWriteForOwned (
  AD_FUNC_ARGS,
  FieldWriteAnalysisRecord* record,
  bool& supports_owned,
  OwnedSupportingEvidence** out_supporting,
  OwnedRejectingEvidence** out_rejecting
) AD_FUNCTION_BEGIN {
  supports_owned = true;
  *out_supporting = NULL;
  *out_rejecting = NULL;

  if (!record || !record->write_capture) {
    supports_owned = false;
    AD_RETURNE (OK);
  }

  FieldWriteCapture* capture = record->write_capture;
  FieldSourceInfo* source_info = record->source_info;
  EscapeSynthesisResult* escape_synthesis = record->escape_synthesis;
  OwnershipTransferAnalysisResult* transfer = record->ownership_transfer;

  location_t loc = gimple_location (capture->stmt);

  // === 检查 1: 源操作数类型 ===
  if (!source_info) {
    // 没有源信息，保守拒绝
    supports_owned = false;

    OwnedRejectingEvidence* evidence = ggc_alloc<OwnedRejectingEvidence>();
    memset (evidence, 0, sizeof (OwnedRejectingEvidence));
    evidence->location = loc;
    evidence->stmt = capture->stmt;
    evidence->rejection_reason = "No source operand analysis available";
    evidence->has_invalid_source = true;
    evidence->source_description = "no source info";
    *out_rejecting = evidence;
    AD_RETURNE (OK);
  }

  FieldSourceType source_type = source_info->source_type;
  bool source_supports = isSourceTypeSupportingOwned (source_type);

  // === 检查 2: 逃逸综合结果 ===
  bool has_rejecting_escape = isEscapeSynthesisRejecting (escape_synthesis);

  // === 检查 3: 所有权转移（仅对字段访问源）===
  bool has_transfer_issue = false;
  if (source_type == SOURCE_FIELD_ACCESS) {
    has_transfer_issue = isTransferResultRejecting (transfer);
  }

  // === 综合判断 ===
  if (!source_supports || has_rejecting_escape || has_transfer_issue) {
    // 拒绝 owned
    supports_owned = false;

    OwnedRejectingEvidence* evidence = ggc_alloc<OwnedRejectingEvidence>();
    memset (evidence, 0, sizeof (OwnedRejectingEvidence));
    evidence->location = loc;
    evidence->stmt = capture->stmt;

    // 确定拒绝原因
    if (!source_supports) {
      evidence->rejection_reason = "Source operand type does not support owned pointer";
      evidence->has_invalid_source = true;
      evidence->source_description = getSourceTypeDescription (source_type);
    } else if (has_rejecting_escape) {
      evidence->rejection_reason = "Source operand has rejecting escape";
      evidence->has_rejecting_escape = true;
      evidence->category_bitmap = escape_synthesis ? escape_synthesis->category_bitmap : 0;
      evidence->total_escapes = escape_synthesis ? escape_synthesis->total_escapes : 0;
      evidence->source_description = getSourceTypeDescription (source_type);
    } else if (has_transfer_issue) {
      evidence->rejection_reason = "Ownership transfer indicates shared ownership";
      evidence->has_transfer_issue = true;
      const char* transfer_str = "UNKNOWN";
      switch (transfer->verdict) {
        case TRANSFER_CERTAIN: transfer_str = "CERTAIN"; break;
        case TRANSFER_IMPOSSIBLE: transfer_str = "IMPOSSIBLE (shared)"; break;
        case TRANSFER_CONDITIONAL: transfer_str = "CONDITIONAL"; break;
        default: break;
      }
      evidence->transfer_verdict_str = transfer_str;
      evidence->source_description = getSourceTypeDescription (source_type);
    }

    *out_rejecting = evidence;
  } else {
    // 支持 owned
    supports_owned = true;

    OwnedSupportingEvidence* evidence = ggc_alloc<OwnedSupportingEvidence>();
    memset (evidence, 0, sizeof (OwnedSupportingEvidence));
    evidence->location = loc;
    evidence->stmt = capture->stmt;

    evidence->source_description = getSourceTypeDescription (source_type);

    if (escape_synthesis) {
      evidence->total_uses = escape_synthesis->total_uses;
      evidence->total_escapes = escape_synthesis->total_escapes;
      evidence->category_bitmap = escape_synthesis->category_bitmap;
    }

    if (transfer) {
      evidence->has_transfer_analysis = true;
      const char* transfer_str = "UNKNOWN";
      switch (transfer->verdict) {
        case TRANSFER_CERTAIN: transfer_str = "CERTAIN"; break;
        case TRANSFER_IMPOSSIBLE: transfer_str = "IMPOSSIBLE"; break;
        case TRANSFER_CONDITIONAL: transfer_str = "CONDITIONAL"; break;
        default: break;
      }
      evidence->transfer_verdict_str = transfer_str;
    }

    *out_supporting = evidence;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 核心函数：分析单个字段的 owned 结论
// ============================================================================

ArrayDetectErrorCode analyzeFieldOwnedConclusion (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData* field_data,
  FieldOwnedConclusion** out_conclusion
) AD_FUNCTION_BEGIN {
  if (!field_data) {
    *out_conclusion = NULL;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_conclusion = NULL;

  // 创建结论结构
  FieldOwnedConclusion* conclusion = ggc_alloc<FieldOwnedConclusion>();
  memset (conclusion, 0, sizeof (FieldOwnedConclusion));

  conclusion->type = field_data->type;
  conclusion->field_decl = field_data->field_decl;
  conclusion->type_name = identifier_to_locale (IDENTIFIER_POINTER (TYPE_IDENTIFIER (field_data->type)));
  conclusion->field_name = identifier_to_locale (IDENTIFIER_POINTER (DECL_NAME (field_data->field_decl)));

  // 初始化证据列表
  vec_alloc (conclusion->supporting_evidences, 4);
  vec_alloc (conclusion->rejecting_evidences, 4);

  // 遍历所有写入操作
  if (!field_data->write_analysis_records) {
    conclusion->verdict = OWNED_UNDETERMINED;
    conclusion->conclusion_description = "No write operations found";
    *out_conclusion = conclusion;
    AD_RETURNE (OK);
  }

  unsigned int total_writes = field_data->write_analysis_records->length ();
  conclusion->total_writes = total_writes;

  unsigned int supporting_count = 0;
  unsigned int rejecting_count = 0;

  for (unsigned int i = 0; i < total_writes; i++) {
    FieldWriteAnalysisRecord* record = (*field_data->write_analysis_records)[i];
    if (!record) continue;

    bool supports_owned = false;
    OwnedSupportingEvidence* supporting = NULL;
    OwnedRejectingEvidence* rejecting = NULL;

    AD_TRY (analyzeWriteForOwned (AD_ARGS, record, supports_owned, &supporting, &rejecting));

    if (supports_owned && supporting) {
      vec_safe_push (conclusion->supporting_evidences, supporting);
      supporting_count++;
    } else if (!supports_owned && rejecting) {
      vec_safe_push (conclusion->rejecting_evidences, rejecting);
      rejecting_count++;
    }
  }

  conclusion->supporting_writes_count = supporting_count;
  conclusion->rejecting_writes_count = rejecting_count;

  // 确定最终判定
  if (rejecting_count > 0) {
    // 有任何拒绝证据，判定为 NO
    conclusion->verdict = OWNED_NO;
    conclusion->conclusion_description = "Field cannot be owned pointer (has rejecting evidence)";
  } else if (supporting_count > 0) {
    // 所有证据都支持，判定为 YES
    conclusion->verdict = OWNED_YES;
    conclusion->conclusion_description = "Field may be owned pointer (all evidence supports)";
  } else {
    // 没有有效证据
    conclusion->verdict = OWNED_UNDETERMINED;
    conclusion->conclusion_description = "Cannot determine (no valid evidence)";
  }

  *out_conclusion = conclusion;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 集成函数：分析所有字段的 owned 结论
// ============================================================================

ArrayDetectErrorCode analyzeAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<FieldOwnedConclusion*, va_gc>** out_conclusions
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing all field owned conclusions");

  *out_conclusions = NULL;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  vec<FieldOwnedConclusion*, va_gc>* conclusions = NULL;
  vec_alloc (conclusions, 16);

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  unsigned int total_fields = 0;
  unsigned int owned_yes = 0;
  unsigned int owned_no = 0;
  unsigned int undetermined = 0;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps* tfwo = (*iter).second;
    if (!tfwo) continue;

    total_fields++;

    FieldOwnedConclusion* conclusion = NULL;
    AD_TRY (analyzeFieldOwnedConclusion (AD_ARGS, tfwo, &conclusion));

    if (conclusion) {
      vec_safe_push (conclusions, conclusion);

      switch (conclusion->verdict) {
        case OWNED_YES:
          owned_yes++;
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

  AD_DEBUG_PRINT ("Field owned conclusion analysis complete: %u fields, %u YES, %u NO, %u undetermined",
                  total_fields, owned_yes, owned_no, undetermined);

  *out_conclusions = conclusions;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出：打印单个字段的 owned 结论
// ============================================================================

void printFieldOwnedConclusion (
  AD_FUNC_ARGS,
  FieldOwnedConclusion* conclusion
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!conclusion) {
    return;
  }

  fprintf (stderr, "\n");
  fprintf (stderr, "=== Field Owned Conclusion ===\n");
  fprintf (stderr, "Type: %s\n", conclusion->type_name);
  fprintf (stderr, "Field: %s\n", conclusion->field_name);
  fprintf (stderr, "\n");

  const char* verdict_str = "UNDETERMINED";
  switch (conclusion->verdict) {
    case OWNED_YES: verdict_str = "YES (may be owned)"; break;
    case OWNED_NO: verdict_str = "NO (cannot be owned)"; break;
    case OWNED_UNDETERMINED: verdict_str = "UNDETERMINED"; break;
  }

  fprintf (stderr, "Verdict: %s\n", verdict_str);
  fprintf (stderr, "Description: %s\n", conclusion->conclusion_description);
  fprintf (stderr, "\n");

  fprintf (stderr, "Statistics:\n");
  fprintf (stderr, "  Total writes: %u\n", conclusion->total_writes);
  fprintf (stderr, "  Supporting writes: %u\n", conclusion->supporting_writes_count);
  fprintf (stderr, "  Rejecting writes: %u\n", conclusion->rejecting_writes_count);
  fprintf (stderr, "\n");

  // 打印支持证据
  if (conclusion->supporting_evidences && conclusion->supporting_evidences->length () > 0) {
    fprintf (stderr, "Supporting Evidence:\n");
    for (unsigned int i = 0; i < conclusion->supporting_evidences->length (); i++) {
      OwnedSupportingEvidence* evidence = (*conclusion->supporting_evidences)[i];
      if (!evidence) continue;

      fprintf (stderr, "  [%u] %s:%d:%d\n",
               i,
               LOCATION_FILE (evidence->location),
               LOCATION_LINE (evidence->location),
               LOCATION_COLUMN (evidence->location));
      fprintf (stderr, "      Source: %s\n", evidence->source_description);
      fprintf (stderr, "      Uses: %u, Escapes: %u, Categories: 0x%x\n",
               evidence->total_uses,
               evidence->total_escapes,
               evidence->category_bitmap);
      if (evidence->has_transfer_analysis) {
        fprintf (stderr, "      Ownership transfer: %s\n", evidence->transfer_verdict_str);
      }
    }
    fprintf (stderr, "\n");
  }

  // 打印拒绝证据
  if (conclusion->rejecting_evidences && conclusion->rejecting_evidences->length () > 0) {
    fprintf (stderr, "Rejecting Evidence:\n");
    for (unsigned int i = 0; i < conclusion->rejecting_evidences->length (); i++) {
      OwnedRejectingEvidence* evidence = (*conclusion->rejecting_evidences)[i];
      if (!evidence) continue;

      fprintf (stderr, "  [%u] %s:%d:%d\n",
               i,
               LOCATION_FILE (evidence->location),
               LOCATION_LINE (evidence->location),
               LOCATION_COLUMN (evidence->location));
      fprintf (stderr, "      Reason: %s\n", evidence->rejection_reason);

      if (evidence->has_invalid_source) {
        fprintf (stderr, "      Source type: %s\n", evidence->source_description);
      }

      if (evidence->has_rejecting_escape) {
        fprintf (stderr, "      Escape categories: 0x%x (%s)\n",
                 evidence->category_bitmap,
                 getEscapeCategoryDescription (evidence->category_bitmap));
        fprintf (stderr, "      Total escapes: %u\n", evidence->total_escapes);
      }

      if (evidence->has_transfer_issue) {
        fprintf (stderr, "      Ownership transfer: %s\n", evidence->transfer_verdict_str);
      }
    }
    fprintf (stderr, "\n");
  }

  fprintf (stderr, "==============================\n");
}

// ============================================================================
// 调试输出：打印所有字段的 owned 结论
// ============================================================================

void printAllFieldOwnedConclusions (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!conclusions || conclusions->length () == 0) {
    fprintf (stderr, "\nNo field owned conclusions to print.\n");
    return;
  }

  fprintf (stderr, "\n");
  fprintf (stderr, "================================================================================\n");
  fprintf (stderr, "                    FIELD OWNED CONCLUSION ANALYSIS RESULTS\n");
  fprintf (stderr, "================================================================================\n");

  unsigned int yes_count = 0;
  unsigned int no_count = 0;
  unsigned int undetermined_count = 0;

  for (unsigned int i = 0; i < conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*conclusions)[i];
    if (!conclusion) continue;

    switch (conclusion->verdict) {
      case OWNED_YES: yes_count++; break;
      case OWNED_NO: no_count++; break;
      case OWNED_UNDETERMINED: undetermined_count++; break;
    }

    printFieldOwnedConclusion (AD_ARGS, conclusion);
  }

  fprintf (stderr, "\n");
  fprintf (stderr, "================================================================================\n");
  fprintf (stderr, "Summary:\n");
  fprintf (stderr, "  Total fields analyzed: %u\n", conclusions->length ());
  fprintf (stderr, "  YES (may be owned): %u\n", yes_count);
  fprintf (stderr, "  NO (cannot be owned): %u\n", no_count);
  fprintf (stderr, "  UNDETERMINED: %u\n", undetermined_count);
  fprintf (stderr, "================================================================================\n");
  fprintf (stderr, "\n");
}

} // namespace array_detect_ns
