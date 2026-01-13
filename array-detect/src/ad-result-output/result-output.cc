// ============================================================================
// ad-result-output: 结果输出模块实现
// ============================================================================

#include "result-output.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "own-conclude.hh"
#include "array-detector.hh"
#include "string-utils.hh"
#include "gcc-ext-util.hh"
#include "malloc-capacity.hh"
#include "array-read-bound.hh"
#include "array-write-bound.hh"

// For flag_generate_lto
#include "options.h"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 辅助函数: 转义字符串用于 Racket datum 输出
// ============================================================================

static void writeEscapedString (FILE* out, char const* s) {
  if (!s) {
    fprintf (out, "\"\"");
    return;
  }
  fputc ('"', out);
  for (char const* p = s; *p; ++p) {
    switch (*p) {
      case '"':  fprintf (out, "\\\""); break;
      case '\\': fprintf (out, "\\\\"); break;
      case '\n': fprintf (out, "\\n");  break;
      case '\r': fprintf (out, "\\r");  break;
      case '\t': fprintf (out, "\\t");  break;
      default:   fputc (*p, out);       break;
    }
  }
  fputc ('"', out);
}

// ============================================================================
// writeOwnedFieldDatum: 输出单个 owned 字段
// ============================================================================

void writeOwnedFieldDatum (
  AD_FUNC_ARGS,
  FILE* out,
  FieldOwnedConclusion* conclusion,
  Wrapper_FieldEscapeConclude_TransferStats* tfad
) {
  if (!out || !conclusion) return;
  if (conclusion->verdict != OWNED_YES) return;

  // 提取模板参数
  vec<char const*, va_gc>* template_args = nullptr;
  if (conclusion->type) {
    char const* base_name = nullptr;
    gcc_ext_util::extractTemplateArgsFromType (AD_ARGS, conclusion->type, &base_name, &template_args);
  }

  // (owned "TypeName" (template-args "T1" ...) "field_name" (malloc-size ...) (reads ...) (writes ...))
  fprintf (out, "(owned ");
  writeEscapedString (out, conclusion->type_name);

  // (template-args "T1" "T2" ...)
  fprintf (out, " (template-args");
  if (template_args) {
    for (unsigned i = 0; i < template_args->length (); i++) {
      fprintf (out, " ");
      writeEscapedString (out, (*template_args)[i]);
    }
  }
  fprintf (out, ")");

  fprintf (out, " ");
  writeEscapedString (out, conclusion->field_name);

  // (malloc-size (total . N) ("field1" . count) ("field2" . count) ...)
  fprintf (out, " (malloc-size");
  if (tfad) {
    // total = 所有 SOURCE_FUNCTION_CALL 类型的写入数量
    unsigned total = 0;
    if (tfad->writes) {
      for (unsigned i = 0; i < tfad->writes->length (); i++) {
        auto* wrapper = (*tfad->writes)[i];
        if (!wrapper || !wrapper->write_source) continue;
        // 检查是否是函数调用来源
        if (wrapper->write_source->source_type == SOURCE_FUNCTION_CALL) {
          total++;
        }
      }
    }
    fprintf (out, " (total . %u)", total);
    // 各字段计数
    if (tfad->malloc_evidences_map) {
      for (auto iter = tfad->malloc_evidences_map->begin ();
           iter != tfad->malloc_evidences_map->end ();
           ++iter) {
        tree integer_field = (*iter).first;
        vec<MallocCapacityEvidence*, va_gc>* evidences = (*iter).second;
        unsigned count = evidences ? evidences->length () : 0;
        char const* name = safeGetFieldName (AD_ARGS, integer_field);
        fprintf (out, " (");
        writeEscapedString (out, name);
        fprintf (out, " . %u)", count);
      }
    }
  }
  fprintf (out, ")");

  // (reads (total . N) ("field1" . count) ("field2" . count) ...)
  fprintf (out, " (reads");
  if (tfad) {
    // total = 数组读取访问总数
    unsigned total = tfad->array_reads ? tfad->array_reads->length () : 0;
    fprintf (out, " (total . %u)", total);
    // 各字段计数
    if (tfad->read_evidences_map) {
      for (auto iter = tfad->read_evidences_map->begin ();
           iter != tfad->read_evidences_map->end ();
           ++iter) {
        tree integer_field = (*iter).first;
        vec<ReadCapacityEvidence*, va_gc>* evidences = (*iter).second;
        unsigned count = evidences ? evidences->length () : 0;
        char const* name = safeGetFieldName (AD_ARGS, integer_field);
        fprintf (out, " (");
        writeEscapedString (out, name);
        fprintf (out, " . %u)", count);
      }
    }
  }
  fprintf (out, ")");

  // (writes (total . N) ("field1" . count) ("field2" . count) ...)
  fprintf (out, " (writes");
  if (tfad) {
    // total = 数组写入访问总数
    unsigned total = tfad->array_writes ? tfad->array_writes->length () : 0;
    fprintf (out, " (total . %u)", total);
    // 各字段计数
    if (tfad->write_evidences_map) {
      for (auto iter = tfad->write_evidences_map->begin ();
           iter != tfad->write_evidences_map->end ();
           ++iter) {
        tree integer_field = (*iter).first;
        vec<WriteCapacityEvidence*, va_gc>* evidences = (*iter).second;
        unsigned count = evidences ? evidences->length () : 0;
        char const* name = safeGetFieldName (AD_ARGS, integer_field);
        fprintf (out, " (");
        writeEscapedString (out, name);
        fprintf (out, " . %u)", count);
      }
    }
  }
  fprintf (out, ")");

  fprintf (out, ")\n");
}

// ============================================================================
// writeOwnedFieldDatumFromSummary: 从 LTO summary 输出
// 格式与 writeOwnedFieldDatum 保持一致
// ============================================================================

void writeOwnedFieldDatumFromSummary (
  AD_FUNC_ARGS,
  FILE* out,
  LtoUnifiedResultSummary* summary
) {
  (void) ctx;
  (void) gcc_ctx;

  if (!out || !summary) return;
  if (summary->owned_verdict != OWNED_YES) return;

  // (owned "TypeName" (template-args "T1" ...) "field_name" (malloc-size ...) (reads ...) (writes ...))
  fprintf (out, "(owned ");
  writeEscapedString (out, summary->type_name);

  // (template-args "T1" "T2" ...)
  fprintf (out, " (template-args");
  if (summary->template_args) {
    for (unsigned i = 0; i < summary->template_args->length (); i++) {
      fprintf (out, " ");
      writeEscapedString (out, (*summary->template_args)[i]);
    }
  }
  fprintf (out, ")");

  fprintf (out, " ");
  writeEscapedString (out, summary->ptr_field_name);

  // (malloc-size (total . N) ("field1" . count1) ...)
  fprintf (out, " (malloc-size");
  fprintf (out, " (total . %u)", summary->malloc_total);
  if (summary->malloc_field_counts) {
    for (unsigned i = 0; i < summary->malloc_field_counts->length (); i++) {
      LtoFieldCount& fc = (*summary->malloc_field_counts)[i];
      fprintf (out, " (");
      writeEscapedString (out, fc.field_name);
      fprintf (out, " . %u)", fc.count);
    }
  }
  fprintf (out, ")");

  // (reads (total . N) ("field1" . count1) ...)
  fprintf (out, " (reads");
  fprintf (out, " (total . %u)", summary->reads_total);
  if (summary->read_field_counts) {
    for (unsigned i = 0; i < summary->read_field_counts->length (); i++) {
      LtoFieldCount& fc = (*summary->read_field_counts)[i];
      fprintf (out, " (");
      writeEscapedString (out, fc.field_name);
      fprintf (out, " . %u)", fc.count);
    }
  }
  fprintf (out, ")");

  // (writes (total . N) ("field1" . count1) ...)
  fprintf (out, " (writes");
  fprintf (out, " (total . %u)", summary->writes_total);
  if (summary->write_field_counts) {
    for (unsigned i = 0; i < summary->write_field_counts->length (); i++) {
      LtoFieldCount& fc = (*summary->write_field_counts)[i];
      fprintf (out, " (");
      writeEscapedString (out, fc.field_name);
      fprintf (out, " . %u)", fc.count);
    }
  }
  fprintf (out, ")");

  fprintf (out, ")\n");
}

// ============================================================================
// writeResultsToFile: 普通编译模式输出
// ============================================================================

ArrayDetectErrorCode writeResultsToFile (
  AD_FUNC_ARGS,
  vec<FieldOwnedConclusion*, va_gc>* conclusions,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  // LTO 模式下跳过 (LTRANS 阶段会输出)
  if (flag_generate_lto) {
    AD_DEBUG_PRINT ("[writeResultsToFile] skip in LTO LGEN mode");
    AD_RETURNE (OK);
  }

  if (!conclusions || conclusions->is_empty ()) {
    AD_DEBUG_PRINT ("[writeResultsToFile] no conclusions to write");
    AD_RETURNE (OK);
  }

  // 获取输出文件路径
  char const* result_file = ctx.result_file_path;
  if (!result_file || !*result_file) {
    AD_DEBUG_PRINT ("[writeResultsToFile] AD_RESULT_FILE not set, skip output");
    AD_RETURNE (OK);
  }

  // Append 模式打开文件
  FILE* out = fopen (result_file, "a");
  if (!out) {
    AD_DEBUG_PRINT ("[writeResultsToFile] failed to open %s", result_file);
    AD_RETURNE (OK);  // 不是致命错误
  }

  unsigned int written = 0;

  for (unsigned i = 0; i < conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*conclusions)[i];
    if (!conclusion || conclusion->verdict != OWNED_YES) continue;

    // 查找对应的 TypeFieldAnalysisData
    Wrapper_FieldEscapeConclude_TransferStats* tfad = nullptr;
    if (detector.m_type_field_writes && conclusion->type && conclusion->field_decl) {
      TypeFieldKey key = {
        TYPE_MAIN_VARIANT (conclusion->type),
        conclusion->field_decl
      };
      Wrapper_FieldEscapeConclude_TransferStats** slot =
        detector.m_type_field_writes->get (key);
      if (slot) {
        tfad = *slot;
      }
    }

    writeOwnedFieldDatum (AD_ARGS, out, conclusion, tfad);
    written++;
  }

  fclose (out);

  AD_DEBUG_PRINT ("[writeResultsToFile] wrote %u owned fields to %s", written, result_file);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// writeLtransResultsToFile: LTRANS 阶段输出
// ============================================================================

ArrayDetectErrorCode writeLtransResultsToFile (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  // 获取输出文件路径
  char const* result_file = ctx.result_file_path;
  if (!result_file || !*result_file) {
    AD_DEBUG_PRINT ("[writeLtransResultsToFile] AD_RESULT_FILE not set, skip output");
    AD_RETURNE (OK);
  }

  // 从 LTO summaries 获取数据
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = getLtransLtoSummaries ();
  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[writeLtransResultsToFile] no LTO summaries available");
    AD_RETURNE (OK);
  }

  // Append 模式打开文件
  FILE* out = fopen (result_file, "a");
  if (!out) {
    AD_DEBUG_PRINT ("[writeLtransResultsToFile] failed to open %s", result_file);
    AD_RETURNE (OK);  // 不是致命错误
  }

  unsigned int written = 0;

  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* summary = (*summaries)[i];
    if (!summary || summary->owned_verdict != OWNED_YES) continue;

    writeOwnedFieldDatumFromSummary (AD_ARGS, out, summary);
    written++;
  }

  fclose (out);

  AD_DEBUG_PRINT ("[writeLtransResultsToFile] wrote %u owned fields to %s", written, result_file);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// writeWpaResultsToFile: WPA 阶段输出
// ============================================================================

ArrayDetectErrorCode writeWpaResultsToFile (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("[writeWpaResultsToFile] ENTRY");

  // 获取输出文件路径
  char const* result_file = ctx.result_file_path;
  AD_DEBUG_PRINT ("[writeWpaResultsToFile] result_file_path=%s",
                  result_file ? result_file : "<null>");

  if (!result_file || !*result_file) {
    AD_DEBUG_PRINT ("[writeWpaResultsToFile] AD_RESULT_FILE not set, skip output");
    AD_RETURNE (OK);
  }

  // 从 WPA summaries 获取数据
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = getWpaLtoSummaries ();
  AD_DEBUG_PRINT ("[writeWpaResultsToFile] summaries count=%u", vec_safe_length (summaries));

  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[writeWpaResultsToFile] no WPA summaries available");
    AD_RETURNE (OK);
  }

  // Append 模式打开文件
  FILE* out = fopen (result_file, "a");
  if (!out) {
    AD_DEBUG_PRINT ("[writeWpaResultsToFile] failed to open %s", result_file);
    AD_RETURNE (OK);  // 不是致命错误
  }

  unsigned int written = 0;

  for (unsigned i = 0; i < summaries->length (); i++) {
    LtoUnifiedResultSummary* summary = (*summaries)[i];
    if (!summary || summary->owned_verdict != OWNED_YES) continue;

    writeOwnedFieldDatumFromSummary (AD_ARGS, out, summary);
    written++;
  }

  fclose (out);

  AD_DEBUG_PRINT ("[writeWpaResultsToFile] wrote %u owned fields to %s", written, result_file);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
