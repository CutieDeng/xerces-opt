#include "owned-conclusion.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"
#include "field-source-variant.hh"
#include "source-escape-collection.hh"
#include "escape-synthesizer.hh"
#include "ownership-transfer-analysis.hh"
#include "info-print.hh"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>

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
    case SOURCE_COMPUTATION: return "computation";
    case SOURCE_PHI: return "phi node";
    case SOURCE_UNKNOWN: return "unknown";
    default: return "unspecified";
  }
}

// ============================================================================
// 辅助函数：检查逃逸证据是否拒绝 owned
// ============================================================================
//
// Owned 语义要求：指针的唯一所有者，不与其他代码共享
// 简化判定：存在非调试逃逸 => 拒绝 owned

static bool isEscapeEvidenceRejecting (EscapeEvidenceResult* evidence) {
  if (!evidence) {
    return false;
  }
  return evidence->has_rejecting_evidence;
}

// ============================================================================
// 辅助函数：获取逃逸证据的描述
// ============================================================================

static const char* getEscapeEvidenceDescription (EscapeEvidenceResult* evidence) {
  if (!evidence) return "no evidence";
  if (!evidence->has_rejecting_evidence) return "no rejecting escape";
  return "has rejecting escape";
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
  EscapeEvidenceResult* escape_evidence = record->escape_evidence;
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

  // === 检查 2: 逃逸证据结果 ===
  bool has_rejecting_escape = isEscapeEvidenceRejecting (escape_evidence);

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
      evidence->rejecting_escapes = escape_evidence ? escape_evidence->rejecting_escapes : 0;
      evidence->total_escapes = escape_evidence ? escape_evidence->total_escapes : 0;
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

    if (escape_evidence) {
      evidence->total_escapes = escape_evidence->total_escapes;
      evidence->safe_debug_escapes = escape_evidence->safe_debug_escapes;
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
// 辅助函数：安全获取类型名
// ============================================================================

static const char* safeGetTypeName (AD_FUNC_ARGS, tree type) {
  (void)gcc_ctx;

  if (!type) {
    AD_DEBUG_PRINT ("Warning: type is NULL when getting type name");
    return "<null-type>";
  }

  tree type_id = TYPE_IDENTIFIER (type);
  if (!type_id) {
    AD_DEBUG_PRINT ("Warning: TYPE_IDENTIFIER returned NULL for type %p", (void*)type);
    return "<anonymous-type>";
  }

  const char* id_ptr = IDENTIFIER_POINTER (type_id);
  if (!id_ptr) {
    AD_DEBUG_PRINT ("Warning: IDENTIFIER_POINTER returned NULL for type identifier");
    return "<unnamed-type>";
  }

  return identifier_to_locale (id_ptr);
}

// ============================================================================
// 辅助函数：安全获取字段名
// ============================================================================

static const char* safeGetFieldName (AD_FUNC_ARGS, tree field_decl) {
  (void)gcc_ctx;

  if (!field_decl) {
    AD_DEBUG_PRINT ("Warning: field_decl is NULL when getting field name");
    return "<null-field>";
  }

  tree decl_name = DECL_NAME (field_decl);
  if (!decl_name) {
    AD_DEBUG_PRINT ("Warning: DECL_NAME returned NULL for field_decl %p", (void*)field_decl);
    return "<anonymous-field>";
  }

  const char* id_ptr = IDENTIFIER_POINTER (decl_name);
  if (!id_ptr) {
    AD_DEBUG_PRINT ("Warning: IDENTIFIER_POINTER returned NULL for field name");
    return "<unnamed-field>";
  }

  return identifier_to_locale (id_ptr);
}

// ============================================================================
// 核心函数：分析单个字段的 owned 结论
// ============================================================================

ArrayDetectErrorCode analyzeFieldOwnedConclusion (
  AD_FUNC_ARGS,
  TypeFieldAnalysisData* field_data,
  FieldOwnedConclusion** out_conclusion
) AD_FUNCTION_BEGIN {
  if (!field_data) {
    AD_DEBUG_PRINT ("Warning: field_data is NULL in analyzeFieldOwnedConclusion");
    *out_conclusion = NULL;
    AD_RETURNE (INVALID_ARGUMENT);
  }

  *out_conclusion = NULL;

  // 验证必要的字段
  if (!field_data->type) {
    AD_DEBUG_PRINT ("Warning: field_data->type is NULL, skipping this field");
    AD_RETURNE (OK);  // 跳过无效字段，不返回错误
  }

  if (!field_data->field_decl) {
    AD_DEBUG_PRINT ("Warning: field_data->field_decl is NULL, skipping this field");
    AD_RETURNE (OK);  // 跳过无效字段，不返回错误
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

  const char* verdict_str = "UNDETERMINED";
  switch (conclusion->verdict) {
    case OWNED_YES: verdict_str = "YES (may be owned)"; break;
    case OWNED_NO: verdict_str = "NO (cannot be owned)"; break;
    case OWNED_UNDETERMINED: verdict_str = "UNDETERMINED"; break;
  }

  fprintf (out, "Verdict: %s\n", verdict_str);
  fprintf (out, "Description: %s\n", conclusion->conclusion_description);
  fprintf (out, "\n");

  fprintf (out, "Statistics:\n");
  fprintf (out, "  Total writes: %u\n", conclusion->total_writes);
  fprintf (out, "  Supporting writes: %u\n", conclusion->supporting_writes_count);
  fprintf (out, "  Rejecting writes: %u\n", conclusion->rejecting_writes_count);
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

    printFieldOwnedConclusion (AD_ARGS, out, conclusion);
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "Summary:\n");
  fprintf (out, "  Total fields analyzed: %u\n", conclusions->length ());
  fprintf (out, "  YES (may be owned): %u\n", yes_count);
  fprintf (out, "  NO (cannot be owned): %u\n", no_count);
  fprintf (out, "  UNDETERMINED: %u\n", undetermined_count);
  fprintf (out, "================================================================================\n");
  fprintf (out, "\n");
}

// ============================================================================
// 辅助函数：转义 Racket 字符串中的特殊字符（使用 context 缓冲区）
// ============================================================================

static void escapeRacketString (
  ArrayDetectContext& ctx,
  const char* input,
  size_t half_offset  // 0 = 使用前半部分，1 = 使用后半部分
) {
  // 使用 escaped_string_buffer 的前半或后半部分
  size_t half_size = ctx.escaped_string_buffer_size / 2;
  char* output = ctx.escaped_string_buffer + (half_offset * half_size);
  size_t output_size = half_size;

  size_t j = 0;
  for (size_t i = 0; input[i] != '\0' && j < output_size - 1; i++) {
    char c = input[i];
    if (c == '"' || c == '\\') {
      if (j + 2 >= output_size) break;
      output[j++] = '\\';
      output[j++] = c;
    } else {
      output[j++] = c;
    }
  }
  output[j] = '\0';
}

// 获取转义后的字符串指针
static inline char* getEscapedString (ArrayDetectContext& ctx, size_t half_offset) {
  size_t half_size = ctx.escaped_string_buffer_size / 2;
  return ctx.escaped_string_buffer + (half_offset * half_size);
}

// ============================================================================
// 辅助函数：确保 result_datum_buffer 有足够容量
// ============================================================================

static bool ensureResultBufferCapacity (ArrayDetectContext& ctx, size_t required) {
  if (ctx.result_datum_buffer_capacity >= required) {
    return true;
  }

  // 扩展容量（至少翻倍，或满足需求）
  size_t new_capacity = ctx.result_datum_buffer_capacity * 2;
  while (new_capacity < required) {
    new_capacity *= 2;
  }

  char* new_buffer = (char*) ggc_realloc (ctx.result_datum_buffer, new_capacity);
  if (!new_buffer) {
    return false;
  }

  ctx.result_datum_buffer = new_buffer;
  ctx.result_datum_buffer_capacity = new_capacity;
  return true;
}

// ============================================================================
// 将结论写入 Racket datum 格式的结果文件
// ============================================================================

ArrayDetectErrorCode writeResultsToRacketDatum (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  // 如果未设置结果文件路径，直接返回
  if (!ctx.result_file_path) {
    AD_RETURNE (OK);
  }

  // 如果没有结论，直接返回
  if (!conclusions || conclusions->length () == 0) {
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("Writing results to Racket datum file: %s", ctx.result_file_path);

  // 重置缓冲区使用量
  ctx.result_datum_buffer_size = 0;

  // 先转义 current_input_file（对所有结论相同）
  // 使用 escaped_string_buffer 的前半部分，然后复制到 result_datum_buffer 开头临时保存
  const char* current_file = ctx.current_input_file ? ctx.current_input_file : "";
  escapeRacketString (ctx, current_file, 0);
  const char* escaped_file_ptr = getEscapedString (ctx, 0);
  size_t escaped_file_len = strlen (escaped_file_ptr);

  // 将转义后的文件名保存到 result_datum_buffer 开头（临时存储）
  if (!ensureResultBufferCapacity (ctx, escaped_file_len + 1)) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memcpy (ctx.result_datum_buffer, escaped_file_ptr, escaped_file_len + 1);
  const char* escaped_current_file = ctx.result_datum_buffer;

  // 重置写入位置到文件名之后
  ctx.result_datum_buffer_size = escaped_file_len + 1;

  // === 构建所有 datum 到缓冲区 ===
  for (unsigned int i = 0; i < conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*conclusions)[i];
    if (!conclusion) continue;

    // 转义类型名和字段名（分别使用缓冲区的前半和后半部分）
    escapeRacketString (ctx, conclusion->type_name ? conclusion->type_name : "", 0);
    escapeRacketString (ctx, conclusion->field_name ? conclusion->field_name : "", 1);

    const char* escaped_type = getEscapedString (ctx, 0);
    const char* escaped_field = getEscapedString (ctx, 1);

    // 获取结果字符串
    const char* result_str;
    switch (conclusion->verdict) {
      case OWNED_YES: result_str = "yes"; break;
      case OWNED_NO: result_str = "no"; break;
      case OWNED_UNDETERMINED:
      default: result_str = "maybe"; break;
    }

    // 计算需要的空间
    size_t needed = escaped_file_len + strlen (escaped_type) + strlen (escaped_field) + 128;
    size_t required_capacity = ctx.result_datum_buffer_size + needed;

    // 确保缓冲区容量足够
    if (!ensureResultBufferCapacity (ctx, required_capacity)) {
      AD_RETURNE (MEMORY_ERROR);
    }

    // 格式化当前条目（包含 current-file 字段）
    int written = snprintf (
      ctx.result_datum_buffer + ctx.result_datum_buffer_size,
      ctx.result_datum_buffer_capacity - ctx.result_datum_buffer_size,
      "((current-file \"%s\")(type \"%s\")(field \"%s\")(result %s))\n",
      escaped_current_file, escaped_type, escaped_field, result_str
    );

    if (written > 0) {
      ctx.result_datum_buffer_size += (size_t)written;
    }
  }

  // === 原子性写入文件 ===
  // 使用 O_APPEND 模式，单次 write() 调用保证原子性（POSIX）
  // 注意：result_datum_buffer 开头存储了转义后的文件名，实际数据从 escaped_file_len + 1 开始
  size_t data_offset = escaped_file_len + 1;
  size_t data_size = ctx.result_datum_buffer_size - data_offset;

  int fd = open (ctx.result_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd < 0) {
    AD_DEBUG_PRINT ("Failed to open result file: %s", ctx.result_file_path);
    AD_RETURNE (RESOURCE_ERROR);
  }

  ssize_t bytes_written = write (fd, ctx.result_datum_buffer + data_offset, data_size);
  if (bytes_written < 0 || (size_t)bytes_written != data_size) {
    AD_DEBUG_PRINT ("Failed to write to result file (written %zd of %zu bytes)",
                    bytes_written, data_size);
    close (fd);
    AD_RETURNE (RESOURCE_ERROR);
  }

  close (fd);

  AD_DEBUG_PRINT ("Successfully wrote %zu bytes (%u conclusions) to result file",
                  data_size, conclusions->length ());

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
