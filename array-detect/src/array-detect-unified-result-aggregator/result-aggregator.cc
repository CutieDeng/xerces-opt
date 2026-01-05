#include "result-aggregator.hh"
#include "state.hh"
#include "array-detect-context-gcc.hh"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

namespace array_detect_ns {

// ============================================================================
// 辅助函数
// ============================================================================

static char const* duplicateString (char const* str) {
  if (!str) return NULL;
  size_t len = strlen(str) + 1;
  char* result = (char*)ggc_alloc_atomic(len);
  memcpy(result, str, len);
  return result;
}

static char const* getTypeName (tree type) {
  if (!type) return "<null>";
  if (TYPE_NAME(type)) {
    tree name = TYPE_NAME(type);
    if (TREE_CODE(name) == IDENTIFIER_NODE) {
      return IDENTIFIER_POINTER(name);
    } else if (TREE_CODE(name) == TYPE_DECL && DECL_NAME(name)) {
      return IDENTIFIER_POINTER(DECL_NAME(name));
    }
  }
  return "<anonymous>";
}

static char const* getFieldNameStr (tree field_decl) {
  if (!field_decl) return "<null>";
  if (DECL_NAME(field_decl)) {
    return IDENTIFIER_POINTER(DECL_NAME(field_decl));
  }
  return "<anonymous>";
}

// ============================================================================
// createUnifiedResult
// ============================================================================

ArrayDetectErrorCode createUnifiedResult (
  AD_FUNC_ARGS,
  tree type,
  tree pointer_field_decl,
  UnifiedFieldAnalysisResult** out_result
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  UnifiedFieldAnalysisResult* result =
    (UnifiedFieldAnalysisResult*)ggc_alloc_atomic(sizeof(UnifiedFieldAnalysisResult));
  memset(result, 0, sizeof(UnifiedFieldAnalysisResult));

  result->type = type;
  result->pointer_field_decl = pointer_field_decl;
  result->type_name = duplicateString(getTypeName(type));
  result->pointer_field_name = duplicateString(getFieldNameStr(pointer_field_decl));

  result->owned_verdict = OWNED_UNDETERMINED;

  vec_alloc(result->array_accesses, 8);
  vec_alloc(result->bound_analyses, 8);
  vec_alloc(result->capacity_relations, 4);

  *out_result = result;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// mergeOwnedConclusion
// ============================================================================

ArrayDetectErrorCode mergeOwnedConclusion (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  FieldOwnedConclusion* owned_conclusion
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  if (!owned_conclusion) {
    AD_RETURNE(OK);
  }

  result->owned_conclusion = owned_conclusion;
  result->owned_verdict = owned_conclusion->verdict;

  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// mergeCapacityAssociation
// ============================================================================

ArrayDetectErrorCode mergeCapacityAssociation (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  PointerCapacityAssociation* capacity_result
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  if (!capacity_result) {
    AD_RETURNE(OK);
  }

  // 遍历所有候选分析结果
  if (capacity_result->candidate_analyses) {
    unsigned int len = vec_safe_length(capacity_result->candidate_analyses);
    for (unsigned int i = 0; i < len; i++) {
      CapacityCandidateAnalysis* candidate = (*capacity_result->candidate_analyses)[i];
      if (!candidate) continue;

      // 只处理有关联证据的候选
      if (candidate->verdict != CAP_ASSOC_RELATED) continue;

      // 创建 CapacityFieldRelation
      CapacityFieldRelation* relation =
        (CapacityFieldRelation*)ggc_alloc_atomic(sizeof(CapacityFieldRelation));
      memset(relation, 0, sizeof(CapacityFieldRelation));

      relation->capacity_field_decl = candidate->field_decl;
      relation->capacity_field_name = duplicateString(candidate->field_name);
      relation->evidence_bitmap = 0;

      // 转换证据类型
      if (candidate->evidence_bitmap & CAP_EVID_MALLOC_SIZE_ARG) {
        relation->evidence_bitmap |= EVID_MALLOC_SIZE_ARG;
      }
      if (candidate->evidence_bitmap & CAP_EVID_READ_CONDITION) {
        relation->evidence_bitmap |= EVID_ARRAY_READ_BOUND;
      }
      if (candidate->evidence_bitmap & CAP_EVID_WRITE_CONDITION) {
        relation->evidence_bitmap |= EVID_ARRAY_WRITE_BOUND;
      }

      // 复制 malloc 证据
      if (candidate->evidences) {
        vec_alloc(relation->malloc_evidences, vec_safe_length(candidate->evidences));
        for (unsigned int j = 0; j < vec_safe_length(candidate->evidences); j++) {
          CapacityAssociationEvidence* ev = (*candidate->evidences)[j];
          if (ev && ev->evidence_type == CAP_EVID_MALLOC_SIZE_ARG) {
            vec_safe_push(relation->malloc_evidences, ev);
          }
        }
      }

      // 计算置信度
      relation->confidence_score = 0;
      if (relation->evidence_bitmap & EVID_MALLOC_SIZE_ARG) {
        relation->confidence_score += 40;
      }
      if (relation->evidence_bitmap & EVID_ARRAY_READ_BOUND) {
        relation->confidence_score += 30;
      }
      if (relation->evidence_bitmap & EVID_ARRAY_WRITE_BOUND) {
        relation->confidence_score += 30;
      }

      vec_safe_push(result->capacity_relations, relation);

      // 更新最佳匹配
      if (!result->best_capacity_match ||
          relation->confidence_score > result->best_capacity_match->confidence_score) {
        result->best_capacity_match = relation;
      }
    }
  }

  result->has_capacity_association = (result->best_capacity_match != NULL);
  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// mergeArrayAccesses
// ============================================================================

ArrayDetectErrorCode mergeArrayAccesses (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result,
  TypeFieldArrayAccesses* accesses
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  if (!accesses) {
    AD_RETURNE(OK);
  }

  // 复制访问记录
  if (accesses->accesses) {
    unsigned int len = vec_safe_length(accesses->accesses);
    for (unsigned int i = 0; i < len; i++) {
      ArrayAccessCapture* access = (*accesses->accesses)[i];
      if (!access) continue;

      vec_safe_push(result->array_accesses, access);

      if (access->direction == ACCESS_READ) {
        result->total_read_accesses++;
      } else {
        result->total_write_accesses++;
      }

      // 检查边界分析
      if (access->bound_analysis) {
        ArrayAccessBoundAnalysis* ba = (ArrayAccessBoundAnalysis*)access->bound_analysis;
        vec_safe_push(result->bound_analyses, ba);

        if (ba->has_valid_bound) {
          result->accesses_with_bound++;
        } else {
          result->accesses_without_bound++;
        }
      } else {
        result->accesses_without_bound++;
      }
    }
  }

  // 更新统计
  result->is_array_candidate =
    (result->total_read_accesses + result->total_write_accesses) > 0;
  result->has_bound_check = result->accesses_with_bound > 0;

  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// calculateOverallConfidence
// ============================================================================

unsigned int calculateOverallConfidence (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result
) {
  (void)ctx;
  (void)gcc_ctx;

  unsigned int score = 0;

  // Owned 结论贡献
  if (result->owned_verdict == OWNED_YES) {
    score += 30;
  } else if (result->owned_verdict == OWNED_UNDETERMINED) {
    score += 15;
  }

  // 数组访问贡献
  if (result->is_array_candidate) {
    score += 20;
  }

  // 边界检查贡献
  if (result->has_bound_check) {
    score += 25;
  }

  // 容量关联贡献
  if (result->has_capacity_association && result->best_capacity_match) {
    score += result->best_capacity_match->confidence_score / 4; // 最多25分
  }

  return (score > 100) ? 100 : score;
}

// ============================================================================
// generateSummaryDescription
// ============================================================================

char const* generateSummaryDescription (
  AD_FUNC_ARGS,
  UnifiedFieldAnalysisResult* result
) {
  (void)ctx;
  (void)gcc_ctx;

  char buffer[512];
  int pos = 0;

  pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s::%s - ",
                  result->type_name, result->pointer_field_name);

  if (result->owned_verdict == OWNED_YES) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "owned");
  } else if (result->owned_verdict == OWNED_NO) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "not owned");
  } else {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "maybe owned");
  }

  if (result->is_array_candidate) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    ", %u array accesses (%u read, %u write)",
                    result->total_read_accesses + result->total_write_accesses,
                    result->total_read_accesses, result->total_write_accesses);
  }

  if (result->has_bound_check) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    ", %u/%u with bounds",
                    result->accesses_with_bound,
                    result->accesses_with_bound + result->accesses_without_bound);
  }

  if (result->has_capacity_association && result->best_capacity_match &&
      result->best_capacity_match->capacity_field_name) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    ", capacity: %s (confidence: %u%%)",
                    result->best_capacity_match->capacity_field_name,
                    result->best_capacity_match->confidence_score);
  }

  return duplicateString(buffer);
}

// ============================================================================
// aggregateAllResults
// ============================================================================

ArrayDetectErrorCode aggregateAllResults (
  AD_FUNC_ARGS,
  ArrayDetector& detector,
  vec<FieldOwnedConclusion*, va_gc>* owned_conclusions,
  vec<PointerCapacityAssociation*, va_gc>* capacity_results,
  hash_map<TypeFieldKey, TypeFieldArrayAccesses*, TypeFieldArrayAccessesHashMapTraits>* array_accesses,
  vec<UnifiedFieldAnalysisResult*, va_gc>** out_results
) AD_FUNCTION_BEGIN {
  (void)detector;

  // 创建结果映射：(type, field) -> UnifiedFieldAnalysisResult
  hash_map<TypeFieldKey, UnifiedFieldAnalysisResult*, UnifiedResultHashMapTraits>* result_map =
    new hash_map<TypeFieldKey, UnifiedFieldAnalysisResult*, UnifiedResultHashMapTraits>();

  // 第一步：处理 owned conclusions
  if (owned_conclusions) {
    unsigned int len = vec_safe_length(owned_conclusions);
    for (unsigned int i = 0; i < len; i++) {
      FieldOwnedConclusion* oc = (*owned_conclusions)[i];
      if (!oc) continue;

      TypeFieldKey key;
      key.type = oc->type;
      key.field_decl = oc->field_decl;
      UnifiedFieldAnalysisResult** existing = result_map->get(key);

      UnifiedFieldAnalysisResult* result;
      if (existing) {
        result = *existing;
      } else {
        AD_TRY(createUnifiedResult(AD_ARGS, oc->type, oc->field_decl, &result));
        result_map->put(key, result);
      }

      AD_TRY(mergeOwnedConclusion(AD_ARGS, result, oc));
    }
  }

  // 第二步：处理 capacity results
  if (capacity_results) {
    unsigned int len = vec_safe_length(capacity_results);
    for (unsigned int i = 0; i < len; i++) {
      PointerCapacityAssociation* cr = (*capacity_results)[i];
      if (!cr) continue;

      TypeFieldKey key;
      key.type = cr->type;
      key.field_decl = cr->pointer_field_decl;
      UnifiedFieldAnalysisResult** existing = result_map->get(key);

      UnifiedFieldAnalysisResult* result;
      if (existing) {
        result = *existing;
      } else {
        AD_TRY(createUnifiedResult(AD_ARGS, cr->type, cr->pointer_field_decl, &result));
        result_map->put(key, result);
      }

      AD_TRY(mergeCapacityAssociation(AD_ARGS, result, cr));
    }
  }

  // 第三步：处理 array accesses
  if (array_accesses) {
    for (auto iter = array_accesses->begin(); iter != array_accesses->end(); ++iter) {
      TypeFieldArrayAccesses* aa = (*iter).second;
      if (!aa) continue;

      TypeFieldKey key;
      key.type = aa->type;
      key.field_decl = aa->pointer_field_decl;
      UnifiedFieldAnalysisResult** existing = result_map->get(key);

      UnifiedFieldAnalysisResult* result;
      if (existing) {
        result = *existing;
      } else {
        AD_TRY(createUnifiedResult(AD_ARGS, aa->type, aa->pointer_field_decl, &result));
        result_map->put(key, result);
      }

      AD_TRY(mergeArrayAccesses(AD_ARGS, result, aa));
    }
  }

  // 第四步：计算综合评分和生成描述
  vec<UnifiedFieldAnalysisResult*, va_gc>* results = NULL;
  vec_alloc(results, result_map->elements());

  for (auto iter = result_map->begin(); iter != result_map->end(); ++iter) {
    UnifiedFieldAnalysisResult* result = (*iter).second;

    result->overall_confidence = calculateOverallConfidence(AD_ARGS, result);
    result->summary_description = generateSummaryDescription(AD_ARGS, result);

    vec_safe_push(results, result);
  }

  delete result_map;
  *out_results = results;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printUnifiedResult (
  AD_FUNC_ARGS,
  FILE* out,
  UnifiedFieldAnalysisResult* result
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!result || !out) return;

  fprintf(out, "\n=== Unified Analysis: %s::%s ===\n",
          result->type_name, result->pointer_field_name);

  // Owned 结论
  fprintf(out, "  Owned verdict: ");
  switch (result->owned_verdict) {
    case OWNED_YES: fprintf(out, "YES\n"); break;
    case OWNED_NO: fprintf(out, "NO\n"); break;
    case OWNED_UNDETERMINED: fprintf(out, "UNDETERMINED\n"); break;
  }

  // 数组访问
  fprintf(out, "  Array accesses: %u total (%u read, %u write)\n",
          result->total_read_accesses + result->total_write_accesses,
          result->total_read_accesses, result->total_write_accesses);

  // 边界检查
  fprintf(out, "  Bound checks: %u with bounds, %u without\n",
          result->accesses_with_bound, result->accesses_without_bound);

  // 容量关联
  fprintf(out, "  Capacity relations: %u found\n",
          (unsigned int)vec_safe_length(result->capacity_relations));
  if (result->best_capacity_match && result->best_capacity_match->capacity_field_name) {
    fprintf(out, "    Best match: %s (confidence: %u%%)\n",
            result->best_capacity_match->capacity_field_name,
            result->best_capacity_match->confidence_score);
  }

  // 综合评分
  fprintf(out, "  Overall confidence: %u%%\n", result->overall_confidence);
  fprintf(out, "  Summary: %s\n", result->summary_description ? result->summary_description : "<none>");
}

void printAllUnifiedResults (
  AD_FUNC_ARGS,
  FILE* out,
  vec<UnifiedFieldAnalysisResult*, va_gc>* results
) {
  if (!out) return;

  if (!results) {
    fprintf(out, "\n=== No unified results ===\n");
    return;
  }

  fprintf(out, "\n========================================\n");
  fprintf(out, "UNIFIED ANALYSIS RESULTS (%u entries)\n",
          (unsigned int)vec_safe_length(results));
  fprintf(out, "========================================\n");

  unsigned int len = vec_safe_length(results);
  for (unsigned int i = 0; i < len; i++) {
    printUnifiedResult(AD_ARGS, out, (*results)[i]);
  }
}

// ============================================================================
// writeUnifiedResultsToRacketDatum
// ============================================================================

ArrayDetectErrorCode writeUnifiedResultsToRacketDatum (
  AD_FUNC_ARGS,
  vec<UnifiedFieldAnalysisResult*, va_gc>* results
) AD_FUNCTION_BEGIN {
  (void)gcc_ctx;

  // 如果未设置结果文件路径，直接返回
  if (!ctx.result_file_path) {
    AD_RETURNE(OK);
  }

  // 如果没有结果，直接返回
  if (!results || results->length() == 0) {
    AD_RETURNE(OK);
  }

  AD_DEBUG_PRINT("Writing unified results to Racket datum file: %s", ctx.result_file_path);

  // 重置缓冲区使用量
  ctx.result_datum_buffer_size = 0;

  // 先转义 current_input_file
  char const* current_file = ctx.current_input_file ? ctx.current_input_file : main_input_filename;
  size_t escaped_file_len = 0;
  if (current_file) {
    // 预留空间存储转义后的文件名
    for (char const* p = current_file; *p; p++) {
      if (*p == '"' || *p == '\\') escaped_file_len++;
      escaped_file_len++;
    }
  }

  // 确保缓冲区足够大
  size_t estimated_size = escaped_file_len + 1 + results->length() * 1024;
  if (estimated_size > ctx.result_datum_buffer_capacity) {
    size_t new_capacity = estimated_size * 2;
    char* new_buffer = (char*)ggc_realloc(ctx.result_datum_buffer, new_capacity);
    ctx.result_datum_buffer = new_buffer;
    ctx.result_datum_buffer_capacity = new_capacity;
  }

  // 为转义后的文件名分配独立的缓冲区（避免被 xrealloc 影响）
  char* escaped_file_buffer = NULL;
  if (current_file) {
    escaped_file_buffer = (char*)xmalloc(escaped_file_len + 1);
    char* dest = escaped_file_buffer;
    for (char const* p = current_file; *p; p++) {
      if (*p == '"' || *p == '\\') *dest++ = '\\';
      *dest++ = *p;
    }
    *dest = '\0';
  } else {
    escaped_file_buffer = (char*)xmalloc(1);
    escaped_file_buffer[0] = '\0';
  }

  // 开始写入数据
  ctx.result_datum_buffer_size = 0;

  #define APPEND_STR(s) do { \
    size_t len = strlen(s); \
    if (ctx.result_datum_buffer_size + len >= ctx.result_datum_buffer_capacity) { \
      size_t new_cap = (ctx.result_datum_buffer_size + len + 1) * 2; \
      ctx.result_datum_buffer = (char*)ggc_realloc(ctx.result_datum_buffer, new_cap); \
      ctx.result_datum_buffer_capacity = new_cap; \
    } \
    memcpy(ctx.result_datum_buffer + ctx.result_datum_buffer_size, s, len); \
    ctx.result_datum_buffer_size += len; \
  } while (0)

  #define APPEND_FMT(...) do { \
    char tmp[256]; \
    snprintf(tmp, sizeof(tmp), __VA_ARGS__); \
    APPEND_STR(tmp); \
  } while (0)

  unsigned int len = vec_safe_length(results);
  for (unsigned int i = 0; i < len; i++) {
    UnifiedFieldAnalysisResult* result = (*results)[i];
    if (!result) continue;

    APPEND_STR("(");

    // 基本信息
    APPEND_STR("(file \"");
    APPEND_STR(escaped_file_buffer);
    APPEND_STR("\")");
    APPEND_FMT("(type \"%s\")", result->type_name ? result->type_name : "<unknown>");
    APPEND_FMT("(field \"%s\")", result->pointer_field_name ? result->pointer_field_name : "<unknown>");

    // Owned 结论
    APPEND_STR("(owned ");
    switch (result->owned_verdict) {
      case OWNED_YES: APPEND_STR("yes"); break;
      case OWNED_NO: APPEND_STR("no"); break;
      case OWNED_UNDETERMINED: APPEND_STR("maybe"); break;
    }
    APPEND_STR(")");

    // malloc-size: 每次 malloc 调用关联的字段列表
    // 格式：listof listof string，与 reads/writes 保持一致
    // 每次 malloc 调用作为一个事件，输出该调用依赖的所有字段
    APPEND_STR("(malloc-size (");
    {
      // 收集所有 malloc 证据，按 malloc 调用语句分组
      // 使用简单数组存储，因为 malloc 调用数量通常很少
      struct MallocCallGroup {
        gimple* stmt;
        location_t loc;
        vec<char const*, va_gc>* fields;
      };
      vec<MallocCallGroup, va_gc>* malloc_groups = NULL;
      vec_alloc(malloc_groups, 4);

      if (result->capacity_relations) {
        unsigned int rel_len = vec_safe_length(result->capacity_relations);
        for (unsigned int j = 0; j < rel_len; j++) {
          CapacityFieldRelation* rel = (*result->capacity_relations)[j];
          if (!rel || !rel->capacity_field_name) continue;
          if (!(rel->evidence_bitmap & EVID_MALLOC_SIZE_ARG)) continue;
          if (!rel->malloc_evidences) continue;

          // 遍历该字段的所有 malloc 证据
          unsigned int ev_len = vec_safe_length(rel->malloc_evidences);
          for (unsigned int k = 0; k < ev_len; k++) {
            CapacityAssociationEvidence* ev = (*rel->malloc_evidences)[k];
            if (!ev) continue;

            // 查找或创建该 malloc 调用的分组
            MallocCallGroup* group = NULL;
            unsigned int grp_len = vec_safe_length(malloc_groups);
            for (unsigned int g = 0; g < grp_len; g++) {
              MallocCallGroup& grp = (*malloc_groups)[g];
              // 使用 stmt 或 location 匹配
              if ((ev->stmt && grp.stmt == ev->stmt) ||
                  (!ev->stmt && grp.loc == ev->location)) {
                group = &grp;
                break;
              }
            }

            if (!group) {
              // 创建新分组
              MallocCallGroup new_grp;
              new_grp.stmt = ev->stmt;
              new_grp.loc = ev->location;
              new_grp.fields = NULL;
              vec_alloc(new_grp.fields, 4);
              vec_safe_push(malloc_groups, new_grp);
              group = &(*malloc_groups)[vec_safe_length(malloc_groups) - 1];
            }

            // 添加字段名（检查去重）
            bool already_exists = false;
            unsigned int fld_len = vec_safe_length(group->fields);
            for (unsigned int f = 0; f < fld_len; f++) {
              if (strcmp((*group->fields)[f], rel->capacity_field_name) == 0) {
                already_exists = true;
                break;
              }
            }
            if (!already_exists) {
              vec_safe_push(group->fields, rel->capacity_field_name);
            }
          }
        }
      }

      // 输出所有 malloc 调用分组
      bool first_group = true;
      unsigned int grp_len = vec_safe_length(malloc_groups);
      for (unsigned int g = 0; g < grp_len; g++) {
        MallocCallGroup& grp = (*malloc_groups)[g];
        if (!first_group) APPEND_STR(" ");
        first_group = false;

        APPEND_STR("(");
        bool first_field = true;
        unsigned int fld_len = vec_safe_length(grp.fields);
        for (unsigned int f = 0; f < fld_len; f++) {
          if (!first_field) APPEND_STR(" ");
          APPEND_FMT("\"%s\"", (*grp.fields)[f]);
          first_field = false;
        }
        APPEND_STR(")");
      }
    }
    APPEND_STR("))");

    // reads: 每次读访问关联的边界字段列表
    APPEND_STR("(reads (");
    if (result->array_accesses) {
      bool first_access = true;
      unsigned int acc_len = vec_safe_length(result->array_accesses);
      for (unsigned int j = 0; j < acc_len; j++) {
        ArrayAccessCapture* access = (*result->array_accesses)[j];
        if (!access || access->direction != ACCESS_READ) continue;

        if (!first_access) APPEND_STR(" ");
        first_access = false;

        APPEND_STR("(");
        // 获取该访问关联的边界字段
        ArrayAccessBoundAnalysis* ba = (ArrayAccessBoundAnalysis*)access->bound_analysis;
        if (ba && ba->related_fields) {
          bool first_field = true;
          unsigned int field_len = vec_safe_length(ba->related_fields);
          for (unsigned int k = 0; k < field_len; k++) {
            tree field_decl = (*ba->related_fields)[k];
            if (!field_decl) continue;
            char const* field_name = DECL_NAME(field_decl) ?
              IDENTIFIER_POINTER(DECL_NAME(field_decl)) : "<anon>";
            if (!first_field) APPEND_STR(" ");
            APPEND_FMT("\"%s\"", field_name);
            first_field = false;
          }
        }
        APPEND_STR(")");
      }
    }
    APPEND_STR("))");

    // writes: 每次写访问关联的边界字段列表
    APPEND_STR("(writes (");
    if (result->array_accesses) {
      bool first_access = true;
      unsigned int acc_len = vec_safe_length(result->array_accesses);
      for (unsigned int j = 0; j < acc_len; j++) {
        ArrayAccessCapture* access = (*result->array_accesses)[j];
        if (!access || access->direction != ACCESS_WRITE) continue;

        if (!first_access) APPEND_STR(" ");
        first_access = false;

        APPEND_STR("(");
        // 获取该访问关联的边界字段
        ArrayAccessBoundAnalysis* ba = (ArrayAccessBoundAnalysis*)access->bound_analysis;
        if (ba && ba->related_fields) {
          bool first_field = true;
          unsigned int field_len = vec_safe_length(ba->related_fields);
          for (unsigned int k = 0; k < field_len; k++) {
            tree field_decl = (*ba->related_fields)[k];
            if (!field_decl) continue;
            char const* field_name = DECL_NAME(field_decl) ?
              IDENTIFIER_POINTER(DECL_NAME(field_decl)) : "<anon>";
            if (!first_field) APPEND_STR(" ");
            APPEND_FMT("\"%s\"", field_name);
            first_field = false;
          }
        }
        APPEND_STR(")");
      }
    }
    APPEND_STR("))");

    APPEND_STR(")\n");
  }

  #undef APPEND_STR
  #undef APPEND_FMT

  // 释放转义文件名缓冲区
  free(escaped_file_buffer);
  escaped_file_buffer = NULL;

  // 使用 O_APPEND 模式，单次 write() 调用保证原子性（POSIX）
  size_t data_size = ctx.result_datum_buffer_size;

  int fd = open(ctx.result_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd < 0) {
    AD_DEBUG_PRINT("Failed to open result file: %s", ctx.result_file_path);
    AD_RETURNE(RESOURCE_ERROR);
  }

  ssize_t bytes_written = write(fd, ctx.result_datum_buffer, data_size);
  if (bytes_written < 0 || (size_t)bytes_written != data_size) {
    close(fd);
    AD_DEBUG_PRINT("Failed to write result file");
    AD_RETURNE(SYSTEM_ERROR);
  }

  close(fd);
  AD_DEBUG_PRINT("Successfully wrote %zu bytes to result file", data_size);

  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
