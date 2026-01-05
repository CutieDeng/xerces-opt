#include "ownership-transfer-analysis.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-source-variant.hh"
#include "info-print.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：判断语句是否为销毁语句
// ============================================================================

ArrayDetectErrorCode isInvalidationStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  tree source_field,
  tree source_object,
  InvalidationKind& kind
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  kind = INVALIDATION_NONE;

  if (!stmt) {
    AD_RETURNE (OK);
  }

  // 只处理赋值语句
  if (gimple_code (stmt) != GIMPLE_ASSIGN) {
    AD_RETURNE (OK);
  }

  tree lhs = gimple_assign_lhs (stmt);
  tree rhs = gimple_assign_rhs1 (stmt);

  AD_DEBUG_PRINT ("  [isInvalidationStatement] Checking stmt, lhs=%p, rhs=%p, source_field=%p, source_object=%p",
                  (void*)lhs, (void*)rhs, (void*)source_field, (void*)source_object);

  // 检查是否为 clobber
  if (TREE_CODE (rhs) == CONSTRUCTOR && TREE_CLOBBER_P (rhs)) {
    // 检查是否是对源对象的 clobber
    if (lhs == source_object) {
      kind = INVALIDATION_CLOBBER_OBJECT;
      AD_RETURNE (OK);
    }

    // 检查是否是对源字段的 clobber
    if (TREE_CODE (lhs) == COMPONENT_REF || TREE_CODE (lhs) == MEM_REF) {
      tree field_decl = NULL_TREE;
      tree object = NULL_TREE;
      bool is_field_access = false;
      AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access));

      if (is_field_access && field_decl == source_field) {
        kind = INVALIDATION_CLOBBER_FIELD;
        AD_RETURNE (OK);
      }
    }
    AD_RETURNE (OK);
  }

  // 检查是否是对源字段的赋值
  if (TREE_CODE (lhs) == COMPONENT_REF || TREE_CODE (lhs) == MEM_REF) {
    tree field_decl = NULL_TREE;
    tree object = NULL_TREE;
    bool is_field_access = false;
    AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access));

    AD_DEBUG_PRINT ("  [isInvalidationStatement] lhs is field access: is_field_access=%d, field_decl=%p (source=%p), object=%p (source=%p)",
                    is_field_access, (void*)field_decl, (void*)source_field, (void*)object, (void*)source_object);

    if (is_field_access && field_decl == source_field) {
      // 关键修复：需要检查对象是否相同
      // 在 SSA 形式中，对象可能是 SSA_NAME，需要比较基础变量
      bool same_object = false;

      if (object == source_object) {
        same_object = true;
        AD_DEBUG_PRINT ("  [isInvalidationStatement] Direct pointer match!");
      } else if (object && source_object) {
        // 调试：输出 tree code 和 SSA 信息
        char const* obj_code_name = get_tree_code_name (TREE_CODE (object));
        char const* src_code_name = get_tree_code_name (TREE_CODE (source_object));
        AD_DEBUG_PRINT ("  [isInvalidationStatement] Tree codes: object=%s, source=%s",
                        obj_code_name, src_code_name);

        if (TREE_CODE (object) == SSA_NAME && TREE_CODE (source_object) == SSA_NAME) {
          AD_DEBUG_PRINT ("  [isInvalidationStatement] SSA versions: object=%d, source=%d",
                          SSA_NAME_VERSION (object), SSA_NAME_VERSION (source_object));

          // 比较 SSA_NAME_VAR
          tree obj_var = SSA_NAME_VAR (object);
          tree src_var = SSA_NAME_VAR (source_object);
          AD_DEBUG_PRINT ("  [isInvalidationStatement] SSA_NAME_VAR: obj_var=%p, src_var=%p",
                          (void*)obj_var, (void*)src_var);

          if (obj_var && src_var && obj_var == src_var) {
            same_object = true;
            AD_DEBUG_PRINT ("  [isInvalidationStatement] Matched via SSA_NAME_VAR!");
          }
        }

        // 尝试使用 operand_equal_p
        if (!same_object && operand_equal_p (object, source_object, 0)) {
          same_object = true;
          AD_DEBUG_PRINT ("  [isInvalidationStatement] Matched via operand_equal_p!");
        }
      }

      if (same_object) {
        AD_DEBUG_PRINT ("  [isInvalidationStatement] MATCHED: Same object and field - this IS an invalidation");

        // 检查是否赋值为 NULL
        if (integer_zerop (rhs)) {
          kind = INVALIDATION_NULL_ASSIGN;
          AD_RETURNE (OK);
        }

        // 其他赋值（覆盖）
        kind = INVALIDATION_OTHER_ASSIGN;
        AD_RETURNE (OK);
      } else {
        AD_DEBUG_PRINT ("  [isInvalidationStatement] SKIPPED: Different object (lhs.field vs source_object.field)");
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 辅助函数：递归查找销毁点（深度优先搜索）
// ============================================================================

namespace {
// 使用 visited 集合避免重复访问
ArrayDetectErrorCode findInvalidationPointsImpl (
  AD_FUNC_ARGS,
  basic_block bb,
  tree source_field,
  tree source_object,
  hash_set<basic_block>& visited,
  vec<InvalidationPoint*, va_gc>*& invalidation_points
) AD_FUNCTION_BEGIN {
  // 检查是否已访问
  if (visited.contains (bb)) {
    AD_RETURNE (OK);
  }
  visited.add (bb);

  // 遍历当前基本块的所有语句
  gimple_stmt_iterator gsi;
  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
    gimple* stmt = gsi_stmt (gsi);

    InvalidationKind kind;
    AD_TRY (isInvalidationStatement (AD_ARGS, stmt, source_field, source_object, kind));

    if (kind != INVALIDATION_NONE) {
      // 找到销毁点，记录
      InvalidationPoint* point = ggc_alloc<InvalidationPoint> ();
      if (!point) {
        AD_RETURNE (MEMORY_ERROR);
      }
      memset (point, 0, sizeof (InvalidationPoint));

      point->stmt = stmt;
      point->kind = kind;
      point->location = gimple_location (stmt);
      point->bb = bb;

      // 生成描述
      char const* kind_str = "unknown";
      switch (kind) {
        case INVALIDATION_NULL_ASSIGN:
          kind_str = "NULL assignment";
          break;
        case INVALIDATION_CLOBBER_FIELD:
          kind_str = "field clobber";
          break;
        case INVALIDATION_CLOBBER_OBJECT:
          kind_str = "object clobber";
          break;
        case INVALIDATION_OTHER_ASSIGN:
          kind_str = "overwrite assignment";
          break;
        default:
          break;
      }
      point->description = ggc_strdup (kind_str);

      vec_safe_push (invalidation_points, point);

      AD_DEBUG_PRINT ("  Found invalidation point: kind=%s, bb=%d", kind_str, bb->index);
    }
  }

  // 递归访问后继基本块
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE (e, ei, bb->succs) {
    AD_TRY (findInvalidationPointsImpl (AD_ARGS, e->dest, source_field, source_object, visited, invalidation_points));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END
}

ArrayDetectErrorCode findInvalidationPoints (
  AD_FUNC_ARGS,
  gimple* start_stmt,
  basic_block start_bb,
  tree source_field,
  tree source_object,
  vec<InvalidationPoint*, va_gc>*& invalidation_points
) AD_FUNCTION_BEGIN {
  (void)start_stmt;

  AD_DEBUG_PRINT ("Finding invalidation points from bb=%d", start_bb ? start_bb->index : -1);

  if (!start_bb) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化结果向量
  vec_alloc (invalidation_points, 0);

  // 使用 hash_set 跟踪已访问的基本块
  hash_set<basic_block> visited;

  // 从起始基本块开始搜索
  AD_TRY (findInvalidationPointsImpl (AD_ARGS, start_bb, source_field, source_object, visited, invalidation_points));

  AD_DEBUG_PRINT ("Found %u invalidation points", invalidation_points ? invalidation_points->length () : 0);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 路径分析：检查是否所有路径都经过销毁点
// ============================================================================

namespace {
// 检查从当前基本块到出口是否必然经过销毁点
ArrayDetectErrorCode checkPathsImpl (
  AD_FUNC_ARGS,
  basic_block bb,
  vec<InvalidationPoint*, va_gc>* invalidation_points,
  hash_set<basic_block>& visited,
  hash_set<basic_block>& invalidation_bbs,
  bool& has_path_without_invalidation
) AD_FUNCTION_BEGIN {
  // 如果已经找到无销毁路径，直接返回
  if (has_path_without_invalidation) {
    AD_RETURNE (OK);
  }

  // 检查是否已访问
  if (visited.contains (bb)) {
    AD_RETURNE (OK);
  }
  visited.add (bb);

  // 检查当前基本块是否包含销毁点
  bool has_invalidation = invalidation_bbs.contains (bb);

  // 如果是出口基本块
  if (EDGE_COUNT (bb->succs) == 0) {
    // 到达出口，检查是否经过销毁点
    if (!has_invalidation) {
      has_path_without_invalidation = true;
      AD_DEBUG_PRINT ("  Found path without invalidation to exit: bb=%d", bb->index);
    }
    AD_RETURNE (OK);
  }

  // 如果当前块有销毁点，不需要继续检查后续路径
  if (has_invalidation) {
    AD_RETURNE (OK);
  }

  // 递归检查所有后继
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE (e, ei, bb->succs) {
    AD_TRY (checkPathsImpl (AD_ARGS, e->dest, invalidation_points, visited, invalidation_bbs, has_path_without_invalidation));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END
}

ArrayDetectErrorCode analyzePathsToExit (
  AD_FUNC_ARGS,
  basic_block start_bb,
  vec<InvalidationPoint*, va_gc>* invalidation_points,
  unsigned int& paths_with_invalidation,
  unsigned int& paths_without_invalidation,
  unsigned int& total_paths
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing paths to exit from bb=%d", start_bb ? start_bb->index : -1);

  paths_with_invalidation = 0;
  paths_without_invalidation = 0;
  total_paths = 0;

  if (!start_bb || !invalidation_points) {
    AD_RETURNE (OK);
  }

  // 构建销毁点所在基本块的集合
  hash_set<basic_block> invalidation_bbs;
  for (unsigned int i = 0; i < invalidation_points->length (); i++) {
    InvalidationPoint* point = (*invalidation_points)[i];
    if (point && point->bb) {
      invalidation_bbs.add (point->bb);
    }
  }

  // 检查是否存在不经过销毁点的路径
  hash_set<basic_block> visited;
  bool has_path_without_invalidation = false;
  AD_TRY (checkPathsImpl (AD_ARGS, start_bb, invalidation_points, visited, invalidation_bbs, has_path_without_invalidation));

  // 简化统计：如果有销毁点但存在不经过的路径，说明是条件转移
  if (invalidation_points->length () > 0) {
    if (has_path_without_invalidation) {
      paths_with_invalidation = 1;
      paths_without_invalidation = 1;
      total_paths = 2;
    } else {
      paths_with_invalidation = 1;
      paths_without_invalidation = 0;
      total_paths = 1;
    }
  } else {
    paths_with_invalidation = 0;
    paths_without_invalidation = 1;
    total_paths = 1;
  }

  AD_DEBUG_PRINT ("Path analysis: %u with invalidation, %u without, %u total",
                  paths_with_invalidation, paths_without_invalidation, total_paths);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 核心分析函数
// ============================================================================

ArrayDetectErrorCode analyzeOwnershipTransfer (
  AD_FUNC_ARGS,
  array_detect_ns::FieldWriteCapture* write_capture,
  array_detector::FieldSourceInfo* source_info,
  OwnershipTransferAnalysisResult*& result
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing ownership transfer");

  if (!write_capture || !source_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 检查源操作数是否为字段访问
  if (source_info->source_type != SOURCE_FIELD_ACCESS) {
    // 不是字段访问，不适用
    result = ggc_alloc<OwnershipTransferAnalysisResult> ();
    if (!result) {
      AD_RETURNE (MEMORY_ERROR);
    }
    memset (result, 0, sizeof (OwnershipTransferAnalysisResult));
    result->verdict = TRANSFER_NOT_APPLICABLE;
    result->is_analyzed = true;
    result->verdict_description = ggc_strdup ("Not a field access source");
    AD_RETURNE (OK);
  }

  // 提取字段访问信息
  FieldAccessSource* field_access = &source_info->data.field_access;

  result = ggc_alloc<OwnershipTransferAnalysisResult> ();
  if (!result) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (result, 0, sizeof (OwnershipTransferAnalysisResult));

  // 填充基本信息
  result->source_field = field_access->field_decl;
  result->source_object = field_access->base_object;
  result->source_field_decl = field_access->field_decl;
  result->source_field_name = field_access->field_name;
  result->transfer_stmt = write_capture->stmt;
  result->transfer_location = write_capture->location;

  // 查找销毁点
  AD_TRY (findInvalidationPoints (
    AD_ARGS,
    write_capture->stmt,
    write_capture->bb,
    result->source_field_decl,
    result->source_object,
    result->invalidation_points
  ));

  // 分析路径
  AD_TRY (analyzePathsToExit (
    AD_ARGS,
    write_capture->bb,
    result->invalidation_points,
    result->paths_with_invalidation,
    result->paths_without_invalidation,
    result->total_exit_paths
  ));

  // 确定结论
  if (result->invalidation_points && result->invalidation_points->length () > 0) {
    if (result->paths_without_invalidation == 0) {
      result->verdict = TRANSFER_CERTAIN;
      result->verdict_description = ggc_strdup ("Certain transfer (all paths have invalidation)");
    } else {
      result->verdict = TRANSFER_CONDITIONAL;
      result->verdict_description = ggc_strdup ("Conditional transfer (some paths have invalidation)");
    }
  } else {
    result->verdict = TRANSFER_IMPOSSIBLE;
    result->verdict_description = ggc_strdup ("No transfer (no invalidation points found)");
  }

  result->is_analyzed = true;

  AD_DEBUG_PRINT ("Transfer analysis complete: verdict=%d, invalidation_points=%u",
                  result->verdict,
                  result->invalidation_points ? result->invalidation_points->length () : 0);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 集成函数：分析所有字段写入操作
// ============================================================================

ArrayDetectErrorCode analyzeAllOwnershipTransfers (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_certain_transfers
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing all ownership transfers");

  total_analyzed = 0;
  total_certain_transfers = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps* tfwo = (*iter).second;
    if (!tfwo || !tfwo->write_analysis_records) continue;

    for (unsigned int i = 0; i < tfwo->write_analysis_records->length (); i++) {
      FieldWriteAnalysisRecord* record = (*tfwo->write_analysis_records)[i];
      if (!record || !record->write_capture || !record->source_info) continue;

      // 只分析字段访问源
      if (record->source_info->source_type != SOURCE_FIELD_ACCESS) {
        continue;
      }

      total_analyzed++;

      // 执行分析
      OwnershipTransferAnalysisResult* transfer_result = NULL;
      AD_TRY (analyzeOwnershipTransfer (
        AD_ARGS,
        record->write_capture,
        record->source_info,
        transfer_result
      ));

      if (transfer_result) {
        // 存储结果到 record
        record->ownership_transfer = transfer_result;

        // 打印结果（调试信息）
        printOwnershipTransferResult (AD_ARGS, ctx.debug_file, transfer_result);

        if (transfer_result->verdict == TRANSFER_CERTAIN) {
          total_certain_transfers++;
        }
      }
    }
  }

  AD_DEBUG_PRINT ("Transfer analysis complete: %u analyzed, %u certain transfers",
                  total_analyzed, total_certain_transfers);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 调试输出
// ============================================================================

void printOwnershipTransferResult (
  AD_FUNC_ARGS,
  FILE* out,
  OwnershipTransferAnalysisResult* result
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!result || !out) {
    return;
  }

  fprintf (out, "\n=== Ownership Transfer Analysis Result ===\n");

  if (result->verdict == TRANSFER_NOT_APPLICABLE) {
    fprintf (out, "Verdict: NOT_APPLICABLE\n");
    fprintf (out, "Description: %s\n", result->verdict_description ? result->verdict_description : "N/A");
    fprintf (out, "==========================================\n");
    return;
  }

  // 源字段信息
  fprintf (out, "Source field: %s\n", result->source_field_name ? result->source_field_name : "<unknown>");

  // 转移位置
  if (result->transfer_location != UNKNOWN_LOCATION) {
    expanded_location xloc = expand_location (result->transfer_location);
    fprintf (out, "Transfer location: %s:%d:%d\n", xloc.file, xloc.line, xloc.column);
  }

  // 销毁点
  fprintf (out, "Invalidation points: %u\n",
           result->invalidation_points ? result->invalidation_points->length () : 0);
  if (result->invalidation_points) {
    for (unsigned int i = 0; i < result->invalidation_points->length (); i++) {
      InvalidationPoint* point = (*result->invalidation_points)[i];
      if (point) {
        fprintf (out, "  [%u] %s (bb=%d)\n", i,
                 point->description ? point->description : "unknown",
                 point->bb ? point->bb->index : -1);
        if (point->location != UNKNOWN_LOCATION) {
          expanded_location xloc = expand_location (point->location);
          fprintf (out, "      Location: %s:%d:%d\n", xloc.file, xloc.line, xloc.column);
        }
      }
    }
  }

  // 路径统计
  fprintf (out, "Path statistics:\n");
  fprintf (out, "  Paths with invalidation: %u\n", result->paths_with_invalidation);
  fprintf (out, "  Paths without invalidation: %u\n", result->paths_without_invalidation);
  fprintf (out, "  Total exit paths: %u\n", result->total_exit_paths);

  // 结论
  char const* verdict_str = "UNKNOWN";
  switch (result->verdict) {
    case TRANSFER_CERTAIN:
      verdict_str = "CERTAIN (ownership transferred)";
      break;
    case TRANSFER_IMPOSSIBLE:
      verdict_str = "IMPOSSIBLE (ownership shared)";
      break;
    case TRANSFER_CONDITIONAL:
      verdict_str = "CONDITIONAL (depends on control flow)";
      break;
    case TRANSFER_NOT_APPLICABLE:
      verdict_str = "NOT_APPLICABLE";
      break;
  }
  fprintf (out, "Verdict: %s\n", verdict_str);
  fprintf (out, "Description: %s\n", result->verdict_description ? result->verdict_description : "N/A");

  fprintf (out, "==========================================\n");
}

} // namespace array_detect_ns
