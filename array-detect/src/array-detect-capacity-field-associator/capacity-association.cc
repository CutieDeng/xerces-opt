#include "capacity-association.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"
#include "field-source-variant.hh"
#include "info-print.hh"
#include "bound-condition-analyzer.hh"
#include "string-utils.hh"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：判断类型是否为整数类型
// ============================================================================

static bool isIntegerType (tree type) {
  if (!type) return false;

  tree main_type = TYPE_MAIN_VARIANT (type);
  return INTEGRAL_TYPE_P (main_type);
}

// ============================================================================
// 收集类型中的所有整数类型字段作为候选
// ============================================================================

ArrayDetectErrorCode collectIntegerCandidates (
  AD_FUNC_ARGS,
  tree type,
  vec<tree, va_gc>** out_candidates
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  *out_candidates = NULL;

  AD_ASSERT_GCC_LOGIC (type, "type must not be NULL");

  if (TREE_CODE (type) != RECORD_TYPE) {
    AD_DEBUG_PRINT ("[collectIntegerCandidates] Skip: type is not RECORD_TYPE");
    AD_RETURNE (OK);
  }

  char const* type_name = safeGetTypeName (AD_ARGS, type);
  AD_DEBUG_PRINT ("[collectIntegerCandidates] Scanning type '%s' for integer fields", type_name);

  vec<tree, va_gc>* candidates = NULL;
  vec_alloc (candidates, 8);

  unsigned int field_count = 0;
  unsigned int integer_count = 0;

  // 遍历类型的所有字段
  for (tree field = TYPE_FIELDS (type); field; field = DECL_CHAIN (field)) {
    if (TREE_CODE (field) != FIELD_DECL) continue;

    field_count++;
    tree field_type = TREE_TYPE (field);
    char const* field_name = safeGetFieldName (AD_ARGS, field);

    if (isIntegerType (field_type)) {
      vec_safe_push (candidates, field);
      integer_count++;

      // 输出详细的字段信息到调试文件
      if (ctx.debug_file) {
        fprintf (ctx.debug_file, "    [INTEGER CANDIDATE] %s::%s (type code: %s)\n",
                 type_name, field_name,
                 get_tree_code_name (TREE_CODE (field_type)));
      }

      AD_DEBUG_PRINT ("[collectIntegerCandidates] Found integer candidate: %s::%s",
                      type_name, field_name);
    } else {
      AD_DEBUG_PRINT ("[collectIntegerCandidates] Skip non-integer field: %s::%s (type code: %s)",
                      type_name, field_name,
                      get_tree_code_name (TREE_CODE (field_type)));
    }
  }

  AD_DEBUG_PRINT ("[collectIntegerCandidates] Type '%s': %u total fields, %u integer candidates",
                  type_name, field_count, integer_count);

  *out_candidates = candidates;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 检查表达式是否引用了指定字段
// ============================================================================

bool expressionReferencesField (
  AD_FUNC_ARGS,
  tree expr,
  tree field_decl
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!expr || !field_decl) {
    return false;
  }

  // 直接检查 COMPONENT_REF
  if (TREE_CODE (expr) == COMPONENT_REF) {
    tree accessed_field = TREE_OPERAND (expr, 1);
    if (accessed_field == field_decl) {
      return true;
    }
    // 递归检查基对象
    tree base = TREE_OPERAND (expr, 0);
    if (expressionReferencesField (AD_ARGS, base, field_decl)) {
      return true;
    }
  }

  // 检查 MEM_REF
  if (TREE_CODE (expr) == MEM_REF) {
    tree base = TREE_OPERAND (expr, 0);
    if (expressionReferencesField (AD_ARGS, base, field_decl)) {
      return true;
    }
  }

  // 检查 SSA_NAME（追溯定义）
  if (TREE_CODE (expr) == SSA_NAME) {
    gimple* def_stmt = SSA_NAME_DEF_STMT (expr);
    if (def_stmt && is_gimple_assign (def_stmt)) {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (expressionReferencesField (AD_ARGS, rhs, field_decl)) {
        return true;
      }
      // 检查二元操作
      if (gimple_assign_rhs2 (def_stmt)) {
        if (expressionReferencesField (AD_ARGS, gimple_assign_rhs2 (def_stmt), field_decl)) {
          return true;
        }
      }
    }
  }

  // 检查二元操作（MULT_EXPR, PLUS_EXPR 等）
  if (BINARY_CLASS_P (expr)) {
    if (expressionReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl)) {
      return true;
    }
    if (expressionReferencesField (AD_ARGS, TREE_OPERAND (expr, 1), field_decl)) {
      return true;
    }
  }

  // 检查一元操作
  if (UNARY_CLASS_P (expr)) {
    if (expressionReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl)) {
      return true;
    }
  }

  // 检查类型转换
  if (CONVERT_EXPR_P (expr) || TREE_CODE (expr) == NOP_EXPR) {
    if (expressionReferencesField (AD_ARGS, TREE_OPERAND (expr, 0), field_decl)) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// 分析 malloc 调用的 size 参数是否引用了候选字段
// ============================================================================

ArrayDetectErrorCode analyzeMallocSizeSource (
  AD_FUNC_ARGS,
  gimple* call_stmt,
  tree candidate_field,
  bool* out_references,
  CapacityAssociationEvidence** out_evidence
) AD_FUNCTION_BEGIN {
  *out_references = false;
  *out_evidence = NULL;

  AD_ASSERT_GCC_LOGIC (call_stmt, "call_stmt must not be NULL");

  if (!is_gimple_call (call_stmt)) {
    AD_RETURNE (OK);
  }

  // 获取函数名（仅用于调试输出）
  tree fndecl = gimple_call_fndecl (call_stmt);
  char const* function_name = "<unknown>";

  if (fndecl) {
    tree id = DECL_NAME (fndecl);
    if (id) {
      function_name = IDENTIFIER_POINTER (id);
    }
  }

  // 激进假设：任何返回指针的函数调用都可能是分配函数
  // 不再检查函数名是否在 malloc family 中

  location_t loc = gimple_location (call_stmt);
  char const* candidate_name = safeGetFieldName (AD_ARGS, candidate_field);

  AD_DEBUG_PRINT ("[analyzeMallocSizeSource] Checking %s() call at %s:%d for field '%s'",
                  function_name,
                  LOCATION_FILE (loc) ? LOCATION_FILE (loc) : "<unknown>",
                  LOCATION_LINE (loc),
                  candidate_name);

  // 输出调试文件详细信息
  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "      [MALLOC CHECK] %s() at %s:%d\n",
             function_name,
             LOCATION_FILE (loc) ? LOCATION_FILE (loc) : "<unknown>",
             LOCATION_LINE (loc));
  }

  // 获取 size 参数（通常是第一个参数，calloc 是前两个参数的乘积）
  unsigned int nargs = gimple_call_num_args (call_stmt);
  if (nargs == 0) {
    AD_DEBUG_PRINT ("[analyzeMallocSizeSource] No arguments in call");
    AD_RETURNE (OK);
  }

  // 检查所有参数
  for (unsigned int i = 0; i < nargs; i++) {
    tree arg = gimple_call_arg (call_stmt, i);

    if (ctx.debug_file) {
      fprintf (ctx.debug_file, "        arg[%u]: checking for reference to '%s'...\n",
               i, candidate_name);
    }

    if (expressionReferencesField (AD_ARGS, arg, candidate_field)) {
      *out_references = true;

      // 创建证据
      CapacityAssociationEvidence* evidence = ggc_alloc<CapacityAssociationEvidence>();
      memset (evidence, 0, sizeof (CapacityAssociationEvidence));

      evidence->evidence_type = CAP_EVID_MALLOC_SIZE_ARG;
      evidence->location = loc;
      evidence->stmt = call_stmt;
      evidence->description = "Allocation size references this field";
      evidence->function_name = ggc_strdup (function_name);
      evidence->size_expr = arg;

      *out_evidence = evidence;

      AD_DEBUG_PRINT ("[analyzeMallocSizeSource] FOUND: %s() arg[%u] references field '%s'",
                      function_name, i, candidate_name);

      // 输出到调试文件
      if (ctx.debug_file) {
        fprintf (ctx.debug_file, "        -> MATCH! arg[%u] references '%s'\n",
                 i, candidate_name);
      }

      AD_RETURNE (OK);
    }
  }

  AD_DEBUG_PRINT ("[analyzeMallocSizeSource] No reference found to '%s' in %s() args",
                  candidate_name, function_name);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 从边界条件分析中提取 READ/WRITE_CONDITION 证据
// ============================================================================

static ArrayDetectErrorCode extractBoundConditionEvidence (
  AD_FUNC_ARGS,
  TypeFieldArrayAccesses* accesses,           // 该指针字段的所有访问（可选）
  tree candidate_field,                        // 候选容量字段
  CapacityCandidateAnalysis* analysis          // 输出：添加证据
) AD_FUNCTION_BEGIN {
  AD_ASSERT_GCC_LOGIC (candidate_field, "candidate_field must not be NULL");
  AD_ASSERT_GCC_LOGIC (analysis, "analysis must not be NULL");

  // accesses 可为空（表示没有数组访问数据）
  if (!accesses || !accesses->accesses) {
    AD_RETURNE (OK);
  }

  char const* candidate_name = safeGetFieldName (AD_ARGS, candidate_field);
  AD_DEBUG_PRINT ("[extractBoundConditionEvidence] Checking %u accesses for field '%s'",
                  vec_safe_length (accesses->accesses), candidate_name);

  for (unsigned int i = 0; i < vec_safe_length (accesses->accesses); i++) {
    ArrayAccessCapture* access = (*accesses->accesses)[i];
    if (!access || !access->bound_analysis) continue;

    ArrayAccessBoundAnalysis* ba = (ArrayAccessBoundAnalysis*)access->bound_analysis;
    if (!ba->related_fields) continue;

    // 检查边界条件是否引用了候选字段
    for (unsigned int j = 0; j < vec_safe_length (ba->related_fields); j++) {
      tree bound_field = (*ba->related_fields)[j];

      if (bound_field == candidate_field) {
        // 创建证据
        CapacityAssociationEvidence* evidence = ggc_alloc<CapacityAssociationEvidence>();
        memset (evidence, 0, sizeof (CapacityAssociationEvidence));

        evidence->location = access->location;
        evidence->stmt = access->stmt;

        if (access->direction == ACCESS_READ) {
          evidence->evidence_type = CAP_EVID_READ_CONDITION;
          evidence->description = "Read access bounded by this field";
          analysis->evidence_bitmap |= CAP_EVID_READ_CONDITION;
          AD_DEBUG_PRINT ("[extractBoundConditionEvidence] Found READ_CONDITION evidence at %s:%d",
                          LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "<unknown>",
                          LOCATION_LINE (access->location));
        } else {
          evidence->evidence_type = CAP_EVID_WRITE_CONDITION;
          evidence->description = "Write access bounded by this field";
          analysis->evidence_bitmap |= CAP_EVID_WRITE_CONDITION;
          AD_DEBUG_PRINT ("[extractBoundConditionEvidence] Found WRITE_CONDITION evidence at %s:%d",
                          LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "<unknown>",
                          LOCATION_LINE (access->location));
        }

        // 填充条件相关信息
        if (ba->primary_bound) {
          evidence->condition_expr = ba->primary_bound->condition_expr;
          evidence->comparison_code = ba->primary_bound->comparison_code;
        }
        evidence->access_stmt = (tree)access->stmt;

        vec_safe_push (analysis->evidences, evidence);

        if (ctx.debug_file) {
          fprintf (ctx.debug_file, "        -> BOUND EVIDENCE: %s access at %s:%d references '%s'\n",
                   access->direction == ACCESS_READ ? "READ" : "WRITE",
                   LOCATION_FILE (access->location) ? LOCATION_FILE (access->location) : "<unknown>",
                   LOCATION_LINE (access->location),
                   candidate_name);
        }

        // 对于同一个 access，只需要记录一次证据
        break;
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析单个候选字段与指针字段的关联
// ============================================================================

static ArrayDetectErrorCode analyzeCandidateAssociation (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  TypeFieldAnalysisData* pointer_field_data,
  TypeFieldArrayAccesses* array_accesses,      // 新增参数：数组访问数据
  tree candidate_field,
  CapacityCandidateAnalysis** out_analysis
) AD_FUNCTION_BEGIN {
  (void)detector;
  *out_analysis = NULL;

  AD_ASSERT_GCC_LOGIC (pointer_field_data, "pointer_field_data must not be NULL");
  AD_ASSERT_GCC_LOGIC (candidate_field, "candidate_field must not be NULL");

  char const* ptr_type_name = safeGetTypeName (AD_ARGS, pointer_field_data->type);
  char const* ptr_field_name = safeGetFieldName (AD_ARGS, pointer_field_data->field_decl);
  char const* candidate_name = safeGetFieldName (AD_ARGS, candidate_field);

  AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Analyzing candidate '%s' for pointer '%s::%s'",
                  candidate_name, ptr_type_name, ptr_field_name);

  // 输出到调试文件
  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "    [CANDIDATE ANALYSIS] %s::%s <- candidate: %s\n",
             ptr_type_name, ptr_field_name, candidate_name);
  }

  // 创建分析结果
  CapacityCandidateAnalysis* analysis = ggc_alloc<CapacityCandidateAnalysis>();
  memset (analysis, 0, sizeof (CapacityCandidateAnalysis));

  analysis->field_decl = candidate_field;
  analysis->field_name = candidate_name;
  analysis->field_type = TREE_TYPE (candidate_field);
  analysis->verdict = CAP_ASSOC_UNDETERMINED;
  analysis->evidence_bitmap = CAP_EVID_NONE;

  vec_alloc (analysis->evidences, 4);

  // 遍历指针字段的所有写入操作
  unsigned int write_count = 0;
  unsigned int malloc_source_count = 0;

  if (pointer_field_data->write_analysis_records) {
    write_count = pointer_field_data->write_analysis_records->length ();
    AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Checking %u write operations", write_count);

    if (ctx.debug_file) {
      fprintf (ctx.debug_file, "      Total write operations: %u\n", write_count);
    }

    for (unsigned int i = 0; i < write_count; i++) {
      FieldWriteAnalysisRecord* record = (*pointer_field_data->write_analysis_records)[i];
      if (!record) {
        AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Write[%u]: NULL record", i);
        continue;
      }

      if (!record->source_info) {
        AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Write[%u]: no source_info", i);
        continue;
      }

      AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Write[%u]: source_type=%d",
                      i, record->source_info->source_type);

      // 检查是否为函数调用源（malloc 等）
      LET_SOURCE_FUNCTION_CALL (func_call, *record->source_info) {
        malloc_source_count++;

        if (ctx.debug_file) {
          fprintf (ctx.debug_file, "      Write[%u]: function call '%s'\n",
                   i, func_call.function_name ? func_call.function_name : "<unknown>");
        }

        // 方法1: 检查 malloc 参数是否直接引用候选字段
        bool references = false;
        CapacityAssociationEvidence* evidence = NULL;

        AD_TRY (analyzeMallocSizeSource (AD_ARGS, func_call.call_stmt,
                                          candidate_field, &references, &evidence));

        if (references && evidence) {
          analysis->evidence_bitmap |= CAP_EVID_MALLOC_SIZE_ARG;
          vec_safe_push (analysis->evidences, evidence);

          if (ctx.debug_file) {
            fprintf (ctx.debug_file, "      -> MALLOC RELATION FOUND (direct ref): '%s' references '%s'\n",
                     ptr_field_name, candidate_name);
          }
        }
      } END_LET()
    }
  }

  AD_DEBUG_PRINT ("[analyzeCandidateAssociation] %u/%u writes are function calls",
                  malloc_source_count, write_count);

  // 新增：从边界条件分析中提取 READ/WRITE_CONDITION 证据
  if (array_accesses) {
    AD_TRY (extractBoundConditionEvidence (AD_ARGS, array_accesses, candidate_field, analysis));
  }

  // 判定结果
  if (analysis->evidence_bitmap != CAP_EVID_NONE) {
    analysis->verdict = CAP_ASSOC_RELATED;
    AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Result: RELATED (bitmap=0x%x)",
                    analysis->evidence_bitmap);
  } else {
    analysis->verdict = CAP_ASSOC_UNRELATED;
    AD_DEBUG_PRINT ("[analyzeCandidateAssociation] Result: UNRELATED");
  }

  if (ctx.debug_file) {
    char const* verdict_str = (analysis->verdict == CAP_ASSOC_RELATED) ? "RELATED" : "UNRELATED";
    fprintf (ctx.debug_file, "      Result: %s (evidence_bitmap=0x%x)\n",
             verdict_str, analysis->evidence_bitmap);
  }

  *out_analysis = analysis;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析单个指针字段的容量关联
// ============================================================================

ArrayDetectErrorCode analyzePointerCapacityAssociation (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  TypeFieldAnalysisData* pointer_field_data,
  TypeFieldArrayAccesses* array_accesses,      // 新增参数：数组访问数据
  PointerCapacityAssociation** out_result
) AD_FUNCTION_BEGIN {
  (void)detector;

  *out_result = NULL;

  AD_ASSERT_GCC_LOGIC (pointer_field_data, "pointer_field_data must not be NULL");
  AD_ASSERT_GCC_LOGIC (pointer_field_data->type, "pointer_field_data->type must not be NULL");
  AD_ASSERT_GCC_LOGIC (pointer_field_data->field_decl, "pointer_field_data->field_decl must not be NULL");

  // 创建结果结构
  PointerCapacityAssociation* result = ggc_alloc<PointerCapacityAssociation>();
  memset (result, 0, sizeof (PointerCapacityAssociation));

  result->type = pointer_field_data->type;
  result->pointer_field_decl = pointer_field_data->field_decl;
  result->type_name = safeGetTypeName (AD_ARGS, pointer_field_data->type);
  result->pointer_field_name = safeGetFieldName (AD_ARGS, pointer_field_data->field_decl);

  vec_alloc (result->candidate_analyses, 8);

  AD_DEBUG_PRINT ("[analyzePointerCapacityAssociation] Analyzing pointer field '%s::%s'",
                  result->type_name, result->pointer_field_name);

  // 输出调试文件头
  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "\n");
    fprintf (ctx.debug_file, "=== CAPACITY ASSOCIATION ANALYSIS ===\n");
    fprintf (ctx.debug_file, "Pointer Field: %s::%s\n",
             result->type_name, result->pointer_field_name);
    fprintf (ctx.debug_file, "\n");
  }

  // 收集所有整数候选字段
  vec<tree, va_gc>* candidates = NULL;
  AD_TRY (collectIntegerCandidates (AD_ARGS, pointer_field_data->type, &candidates));

  if (!candidates || candidates->length () == 0) {
    AD_DEBUG_PRINT ("[analyzePointerCapacityAssociation] No integer candidates found for type '%s'",
                    result->type_name);
    if (ctx.debug_file) {
      fprintf (ctx.debug_file, "No integer candidate fields found in type '%s'\n",
               result->type_name);
    }
    *out_result = result;
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[analyzePointerCapacityAssociation] Found %u integer candidates",
                  candidates->length ());

  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "Integer candidates found: %u\n\n", candidates->length ());
  }

  // 分析每个候选字段
  CapacityCandidateAnalysis* best_match = NULL;
  unsigned int best_evidence_count = 0;

  for (unsigned int i = 0; i < candidates->length (); i++) {
    tree candidate = (*candidates)[i];
    char const* candidate_name = safeGetFieldName (AD_ARGS, candidate);

    AD_DEBUG_PRINT ("[analyzePointerCapacityAssociation] Processing candidate[%u]: '%s'",
                    i, candidate_name);

    CapacityCandidateAnalysis* analysis = NULL;
    AD_TRY (analyzeCandidateAssociation (AD_ARGS, detector, pointer_field_data, array_accesses, candidate, &analysis));

    if (analysis) {
      vec_safe_push (result->candidate_analyses, analysis);

      switch (analysis->verdict) {
        case CAP_ASSOC_RELATED:
          result->related_count++;
          // 选择最佳匹配（证据最多的）
          if (analysis->evidences) {
            unsigned int evidence_count = analysis->evidences->length ();
            if (evidence_count > best_evidence_count) {
              best_evidence_count = evidence_count;
              best_match = analysis;
            }
          }
          break;
        case CAP_ASSOC_UNRELATED:
          result->unrelated_count++;
          break;
        case CAP_ASSOC_UNDETERMINED:
          result->undetermined_count++;
          break;
      }
    }
  }

  result->best_match = best_match;

  AD_DEBUG_PRINT ("[analyzePointerCapacityAssociation] Result for %s::%s: %u related, %u unrelated, %u undetermined",
                  result->type_name,
                  result->pointer_field_name,
                  result->related_count,
                  result->unrelated_count,
                  result->undetermined_count);

  // 输出结果摘要到调试文件
  if (ctx.debug_file) {
    fprintf (ctx.debug_file, "\n--- SUMMARY for %s::%s ---\n",
             result->type_name, result->pointer_field_name);
    fprintf (ctx.debug_file, "  Related fields: %u\n", result->related_count);
    fprintf (ctx.debug_file, "  Unrelated fields: %u\n", result->unrelated_count);
    fprintf (ctx.debug_file, "  Undetermined fields: %u\n", result->undetermined_count);

    if (best_match) {
      fprintf (ctx.debug_file, "  Best match: %s (evidence_count=%u)\n",
               best_match->field_name, best_evidence_count);
    }
    fprintf (ctx.debug_file, "===================================\n\n");
  }

  *out_result = result;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 分析所有疑似 owned 指针字段的容量关联
// ============================================================================

ArrayDetectErrorCode analyzeAllCapacityAssociations (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses,
  vec<PointerCapacityAssociation*, va_gc>** out_results
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing capacity associations for owned pointer fields");

  *out_results = NULL;

  AD_ASSERT_GCC_LOGIC (owned_conclusions, "owned_conclusions must not be NULL");

  if (owned_conclusions->length () == 0) {
    AD_RETURNE (OK);
  }

  vec<PointerCapacityAssociation*, va_gc>* results = NULL;
  vec_alloc (results, 16);

  unsigned int analyzed = 0;
  unsigned int with_capacity = 0;

  for (unsigned int i = 0; i < owned_conclusions->length (); i++) {
    FieldOwnedConclusion* conclusion = (*owned_conclusions)[i];
    if (!conclusion) continue;

    // 只分析 OWNED_YES 或 OWNED_UNDETERMINED 的字段
    if (conclusion->verdict == OWNED_NO) {
      continue;
    }

    // 在 detector 中查找对应的分析数据
    TypeFieldKey key = { conclusion->type, conclusion->field_decl };
    TypeFieldAnalysisData** data_ptr = detector.m_type_field_writes->get (key);
    if (!data_ptr || !*data_ptr) {
      continue;
    }

    // 查找该指针字段的数组访问数据
    TypeFieldArrayAccesses* field_accesses = NULL;
    if (array_accesses) {
      TypeFieldArrayAccesses** accesses_ptr = array_accesses->get (key);
      if (accesses_ptr) {
        field_accesses = *accesses_ptr;
      }
    }

    PointerCapacityAssociation* result = NULL;
    AD_TRY (analyzePointerCapacityAssociation (AD_ARGS, detector, *data_ptr, field_accesses, &result));

    if (result) {
      vec_safe_push (results, result);
      analyzed++;

      if (result->related_count > 0) {
        with_capacity++;
      }
    }
  }

  AD_DEBUG_PRINT ("Capacity association analysis complete: %u fields analyzed, %u with capacity fields",
                  analyzed, with_capacity);

  *out_results = results;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出：打印指针容量关联结果
// ============================================================================

void printPointerCapacityAssociation (
  AD_FUNC_ARGS,
  FILE* out,
  PointerCapacityAssociation* result
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!result || !out) {
    return;
  }

  fprintf (out, "\n");
  fprintf (out, "=== Pointer-Capacity Association ===\n");
  fprintf (out, "Type: %s\n", result->type_name);
  fprintf (out, "Pointer Field: %s\n", result->pointer_field_name);
  fprintf (out, "\n");

  fprintf (out, "Statistics:\n");
  fprintf (out, "  Related candidates: %u\n", result->related_count);
  fprintf (out, "  Unrelated candidates: %u\n", result->unrelated_count);
  fprintf (out, "  Undetermined candidates: %u\n", result->undetermined_count);
  fprintf (out, "\n");

  if (result->best_match) {
    fprintf (out, "Best Match: %s\n", result->best_match->field_name);
    fprintf (out, "  Evidence bitmap: 0x%x\n", result->best_match->evidence_bitmap);
    if (result->best_match->evidences) {
      fprintf (out, "  Evidence count: %u\n", result->best_match->evidences->length ());
    }
    fprintf (out, "\n");
  }

  // 打印所有候选分析
  if (result->candidate_analyses && result->candidate_analyses->length () > 0) {
    fprintf (out, "Candidate Analyses:\n");
    for (unsigned int i = 0; i < result->candidate_analyses->length (); i++) {
      CapacityCandidateAnalysis* analysis = (*result->candidate_analyses)[i];
      if (!analysis) continue;

      char const* verdict_str = "UNDETERMINED";
      switch (analysis->verdict) {
        case CAP_ASSOC_RELATED: verdict_str = "RELATED"; break;
        case CAP_ASSOC_UNRELATED: verdict_str = "UNRELATED"; break;
        case CAP_ASSOC_UNDETERMINED: verdict_str = "UNDETERMINED"; break;
      }

      fprintf (out, "  [%u] %s: %s (evidence: 0x%x)\n",
               i, analysis->field_name, verdict_str, analysis->evidence_bitmap);

      // 打印证据详情
      if (analysis->evidences) {
        for (unsigned int j = 0; j < analysis->evidences->length (); j++) {
          CapacityAssociationEvidence* evidence = (*analysis->evidences)[j];
          if (!evidence) continue;

          fprintf (out, "        - %s at %s:%d\n",
                   evidence->description,
                   LOCATION_FILE (evidence->location),
                   LOCATION_LINE (evidence->location));
        }
      }
    }
    fprintf (out, "\n");
  }

  fprintf (out, "=====================================\n");
}

// ============================================================================
// 调试输出：打印所有指针容量关联结果
// ============================================================================

void printAllPointerCapacityAssociations (
  AD_FUNC_ARGS,
  FILE* out,
  vec<PointerCapacityAssociation*, va_gc>* results
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!out) {
    return;
  }

  if (!results || results->length () == 0) {
    fprintf (out, "\nNo pointer-capacity associations to print.\n");
    return;
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "                 POINTER-CAPACITY ASSOCIATION ANALYSIS RESULTS\n");
  fprintf (out, "================================================================================\n");

  unsigned int with_capacity = 0;
  unsigned int without_capacity = 0;

  for (unsigned int i = 0; i < results->length (); i++) {
    PointerCapacityAssociation* result = (*results)[i];
    if (!result) continue;

    if (result->related_count > 0) {
      with_capacity++;
    } else {
      without_capacity++;
    }

    printPointerCapacityAssociation (AD_ARGS, out, result);
  }

  fprintf (out, "\n");
  fprintf (out, "================================================================================\n");
  fprintf (out, "Summary:\n");
  fprintf (out, "  Total pointer fields analyzed: %u\n", results->length ());
  fprintf (out, "  With capacity association: %u\n", with_capacity);
  fprintf (out, "  Without capacity association: %u\n", without_capacity);
  fprintf (out, "================================================================================\n");
  fprintf (out, "\n");
}

// ============================================================================
// 辅助函数：收集按证据类型分类的字段列表
// ============================================================================

static void collectFieldsByEvidenceType (
  PointerCapacityAssociation* result,
  vec<char const*, va_gc>** out_malloc_fields,
  vec<char const*, va_gc>** out_read_fields,
  vec<char const*, va_gc>** out_write_fields
) {
  vec<char const*, va_gc>* malloc_fields = NULL;
  vec<char const*, va_gc>* read_fields = NULL;
  vec<char const*, va_gc>* write_fields = NULL;

  vec_alloc (malloc_fields, 4);
  vec_alloc (read_fields, 4);
  vec_alloc (write_fields, 4);

  if (result && result->candidate_analyses) {
    for (unsigned int i = 0; i < result->candidate_analyses->length (); i++) {
      CapacityCandidateAnalysis* analysis = (*result->candidate_analyses)[i];
      if (!analysis || analysis->verdict != CAP_ASSOC_RELATED) continue;

      char const* field_name = analysis->field_name ? analysis->field_name : "<unknown>";

      if (analysis->evidence_bitmap & CAP_EVID_MALLOC_SIZE_ARG) {
        vec_safe_push (malloc_fields, field_name);
      }
      if (analysis->evidence_bitmap & CAP_EVID_READ_CONDITION) {
        vec_safe_push (read_fields, field_name);
      }
      if (analysis->evidence_bitmap & CAP_EVID_WRITE_CONDITION) {
        vec_safe_push (write_fields, field_name);
      }
    }
  }

  *out_malloc_fields = malloc_fields;
  *out_read_fields = read_fields;
  *out_write_fields = write_fields;
}

// ============================================================================
// 辅助函数：构建字段列表字符串 (field1 field2 ...)
// ============================================================================

static size_t buildFieldListString (
  ArrayDetectContext& ctx,
  vec<char const*, va_gc>* fields,
  char* buffer,
  size_t buffer_size
) {
  size_t pos = 0;

  if (pos < buffer_size) buffer[pos++] = '(';

  if (fields && fields->length () > 0) {
    for (unsigned int i = 0; i < fields->length (); i++) {
      char const* field_name = (*fields)[i];
      if (!field_name) continue;

      // 转义字段名
      escapeRacketString (ctx, field_name, 0);
      char const* escaped = getEscapedString (ctx, 0);

      // 添加空格分隔（除了第一个）
      if (i > 0 && pos < buffer_size) {
        buffer[pos++] = ' ';
      }

      // 添加转义后的字段名（带引号）
      if (pos < buffer_size) buffer[pos++] = '"';
      size_t escaped_len = strlen (escaped);
      for (size_t j = 0; j < escaped_len && pos < buffer_size - 1; j++) {
        buffer[pos++] = escaped[j];
      }
      if (pos < buffer_size) buffer[pos++] = '"';
    }
  }

  if (pos < buffer_size) buffer[pos++] = ')';
  if (pos < buffer_size) buffer[pos] = '\0';

  return pos;
}

// ============================================================================
// 将容量关联结果写入 Racket datum 格式的结果文件
// ============================================================================
// 输出格式：
// ((current-file "...")
//  (type "...")
//  (pointer-field "...")
//  (malloc-relation ("field1" "field2" ...))
//  (read-relation ("field1" ...))
//  (write-relation ("field1" ...)))

ArrayDetectErrorCode writeCapacityAssociationsToRacketDatum (
  AD_FUNC_ARGS,
  vec<PointerCapacityAssociation*, va_gc>* results
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  // 如果未设置结果文件路径，直接返回
  if (!ctx.result_file_path) {
    AD_RETURNE (OK);
  }

  // 如果没有结果，直接返回
  if (!results || results->length () == 0) {
    AD_RETURNE (OK);
  }

  AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] Writing to: %s", ctx.result_file_path);

  // 重置缓冲区使用量
  ctx.result_datum_buffer_size = 0;

  // 转义 current_input_file
  char const* current_file = ctx.current_input_file ? ctx.current_input_file : "";
  escapeRacketString (ctx, current_file, 0);
  char const* escaped_file_ptr = getEscapedString (ctx, 0);
  size_t escaped_file_len = strlen (escaped_file_ptr);

  // 保存转义后的文件名到缓冲区开头（临时存储）
  if (!ensureResultBufferCapacity (ctx, escaped_file_len + 1)) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memcpy (ctx.result_datum_buffer, escaped_file_ptr, escaped_file_len + 1);
  char const* escaped_current_file = ctx.result_datum_buffer;
  ctx.result_datum_buffer_size = escaped_file_len + 1;

  // 临时缓冲区用于字段列表
  char malloc_list[1024];
  char read_list[1024];
  char write_list[1024];

  // 构建所有 datum（每个指针字段一条记录）
  for (unsigned int i = 0; i < results->length (); i++) {
    PointerCapacityAssociation* result = (*results)[i];
    if (!result) continue;

    // 收集按证据类型分类的字段
    vec<char const*, va_gc>* malloc_fields = NULL;
    vec<char const*, va_gc>* read_fields = NULL;
    vec<char const*, va_gc>* write_fields = NULL;
    collectFieldsByEvidenceType (result, &malloc_fields, &read_fields, &write_fields);

    // 统计各列表数量（用于调试输出）
    unsigned int malloc_count = malloc_fields ? malloc_fields->length () : 0;
    unsigned int read_count = read_fields ? read_fields->length () : 0;
    unsigned int write_count = write_fields ? write_fields->length () : 0;

    // 注意：即使所有列表为空，也要输出记录（用户要求）

    // 构建字段列表字符串
    buildFieldListString (ctx, malloc_fields, malloc_list, sizeof (malloc_list));
    buildFieldListString (ctx, read_fields, read_list, sizeof (read_list));
    buildFieldListString (ctx, write_fields, write_list, sizeof (write_list));

    // 转义类型名和字段名（使用缓冲区后半部分）
    escapeRacketString (ctx, result->type_name ? result->type_name : "", 1);
    char const* escaped_type = getEscapedString (ctx, 1);

    // 需要单独转义 pointer_field_name
    char escaped_ptr_field[256];
    {
      char const* ptr_name = result->pointer_field_name ? result->pointer_field_name : "";
      size_t k = 0;
      for (size_t m = 0; ptr_name[m] != '\0' && k < 254; m++) {
        if (ptr_name[m] == '"' || ptr_name[m] == '\\') {
          escaped_ptr_field[k++] = '\\';
        }
        escaped_ptr_field[k++] = ptr_name[m];
      }
      escaped_ptr_field[k] = '\0';
    }

    // 计算需要的空间
    size_t needed = escaped_file_len + strlen (escaped_type) + strlen (escaped_ptr_field) +
                    strlen (malloc_list) + strlen (read_list) + strlen (write_list) + 512;
    size_t required_capacity = ctx.result_datum_buffer_size + needed;

    if (!ensureResultBufferCapacity (ctx, required_capacity)) {
      AD_RETURNE (MEMORY_ERROR);
    }

    // 格式化条目
    // 格式: ((current-file "...")(type "...")(pointer-field "...")(malloc-relation (...))(read-relation (...))(write-relation (...)))
    int written = snprintf (
      ctx.result_datum_buffer + ctx.result_datum_buffer_size,
      ctx.result_datum_buffer_capacity - ctx.result_datum_buffer_size,
      "((current-file \"%s\")(type \"%s\")(pointer-field \"%s\")(malloc-relation %s)(read-relation %s)(write-relation %s))\n",
      escaped_current_file, escaped_type, escaped_ptr_field,
      malloc_list, read_list, write_list
    );

    if (written > 0) {
      ctx.result_datum_buffer_size += (size_t)written;
    }

    AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] Added entry for %s::%s: malloc=%u, read=%u, write=%u",
                    result->type_name, result->pointer_field_name,
                    malloc_count, read_count, write_count);
  }

  // 检查是否有数据要写入
  size_t data_offset = escaped_file_len + 1;
  size_t data_size = ctx.result_datum_buffer_size - data_offset;

  if (data_size == 0) {
    AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] No capacity associations to write");
    AD_RETURNE (OK);
  }

  // 原子性写入文件
  int fd = open (ctx.result_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd < 0) {
    AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] Failed to open result file: %s", ctx.result_file_path);
    AD_RETURNE (RESOURCE_ERROR);
  }

  ssize_t bytes_written = write (fd, ctx.result_datum_buffer + data_offset, data_size);
  if (bytes_written < 0 || (size_t)bytes_written != data_size) {
    AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] Failed to write to result file (written %zd of %zu bytes)",
                    bytes_written, data_size);
    close (fd);
    AD_RETURNE (RESOURCE_ERROR);
  }

  close (fd);

  AD_DEBUG_PRINT ("[writeCapacityAssociationsToRacketDatum] Successfully wrote %zu bytes to result file", data_size);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
