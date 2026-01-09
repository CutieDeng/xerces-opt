// ============================================================================
// ad-ownership-move 模块实现
// ============================================================================
// 分析字段复制中源字段是否被销毁
// 数据流：field-write-info, write-original-source -> ownership-move-result
// ============================================================================

#include "ownership-move.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "info-print.hh"
#include "field-analysis.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_findInvalidationPoints_searchBlock_isInvalidationStatement
// ----------------------------------------------------------------------------
// 判断语句是否为销毁语句

ArrayDetectErrorCode analyzeOwnershipMove_findInvalidationPoints_searchBlock_isInvalidationStatement (
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

    if (is_field_access && field_decl == source_field) {
      // Check if same object (SSA form may use different SSA_NAME versions)
      bool same_object = false;

      if (object == source_object) {
        same_object = true;
      } else if (object && source_object) {
        if (TREE_CODE (object) == SSA_NAME && TREE_CODE (source_object) == SSA_NAME) {
          tree obj_var = SSA_NAME_VAR (object);
          tree src_var = SSA_NAME_VAR (source_object);
          if (obj_var && src_var && obj_var == src_var) {
            same_object = true;
          }
        }
        if (!same_object && operand_equal_p (object, source_object, 0)) {
          same_object = true;
        }
      }

      if (same_object) {
        if (integer_zerop (rhs)) {
          kind = INVALIDATION_NULL_ASSIGN;
          AD_RETURNE (OK);
        }
        kind = INVALIDATION_OTHER_ASSIGN;
        AD_RETURNE (OK);
      }
    }
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_findInvalidationPoints_searchBlock
// ----------------------------------------------------------------------------
// 递归搜索基本块中的销毁点

ArrayDetectErrorCode analyzeOwnershipMove_findInvalidationPoints_searchBlock (
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
    AD_TRY (analyzeOwnershipMove_findInvalidationPoints_searchBlock_isInvalidationStatement (AD_ARGS, stmt, source_field, source_object, kind));

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
      (void)kind_str;
    }
  }

  // 递归访问后继基本块
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE (e, ei, bb->succs) {
    AD_TRY (analyzeOwnershipMove_findInvalidationPoints_searchBlock (
      AD_ARGS, e->dest, source_field, source_object, visited, invalidation_points
    ));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_findInvalidationPoints
// ----------------------------------------------------------------------------
// 查找从当前语句到函数出口的所有销毁点

ArrayDetectErrorCode analyzeOwnershipMove_findInvalidationPoints (
  AD_FUNC_ARGS,
  gimple* start_stmt,
  basic_block start_bb,
  tree source_field,
  tree source_object,
  vec<InvalidationPoint*, va_gc>*& invalidation_points
) AD_FUNCTION_BEGIN {
  (void)start_stmt;

  if (!start_bb) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化结果向量
  vec_alloc (invalidation_points, 0);

  // 使用 hash_set 跟踪已访问的基本块
  hash_set<basic_block> visited;

  // 从起始基本块开始搜索
  AD_TRY (analyzeOwnershipMove_findInvalidationPoints_searchBlock (
    AD_ARGS, start_bb, source_field, source_object, visited, invalidation_points
  ));

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_analyzePaths_checkPathsImpl
// ----------------------------------------------------------------------------
// 检查从当前基本块到出口是否必然经过销毁点

ArrayDetectErrorCode analyzeOwnershipMove_analyzePaths_checkPathsImpl (
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
    AD_TRY (analyzeOwnershipMove_analyzePaths_checkPathsImpl (
      AD_ARGS, e->dest, invalidation_points, visited, invalidation_bbs, has_path_without_invalidation
    ));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_analyzePaths
// ----------------------------------------------------------------------------
// 分析从当前基本块到函数出口的路径

ArrayDetectErrorCode analyzeOwnershipMove_analyzePaths (
  AD_FUNC_ARGS,
  basic_block start_bb,
  vec<InvalidationPoint*, va_gc>* invalidation_points,
  unsigned int& paths_with,
  unsigned int& paths_without,
  unsigned int& total_paths
) AD_FUNCTION_BEGIN {
  paths_with = 0;
  paths_without = 0;
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
  AD_TRY (analyzeOwnershipMove_analyzePaths_checkPathsImpl (
    AD_ARGS, start_bb, invalidation_points, visited, invalidation_bbs, has_path_without_invalidation
  ));

  // 简化统计：如果有销毁点但存在不经过的路径，说明是条件转移
  if (invalidation_points->length () > 0) {
    if (has_path_without_invalidation) {
      paths_with = 1;
      paths_without = 1;
      total_paths = 2;
    } else {
      paths_with = 1;
      paths_without = 0;
      total_paths = 1;
    }
  } else {
    paths_with = 0;
    paths_without = 1;
    total_paths = 1;
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeOwnershipMove_determineVerdict
// ----------------------------------------------------------------------------
// 确定所有权转移结论

ArrayDetectErrorCode analyzeOwnershipMove_determineVerdict (
  AD_FUNC_ARGS,
  OwnershipMoveResult* result
) AD_FUNCTION_BEGIN {
  (void)ctx;
  (void)gcc_ctx;

  if (!result) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 根据销毁点和路径确定结论
  if (result->invalidation_points && result->invalidation_points->length () > 0) {
    if (result->paths_without_invalidation == 0) {
      result->verdict = MOVE_CERTAIN;
      result->verdict_description = ggc_strdup ("Certain transfer (all paths have invalidation)");
    } else {
      result->verdict = MOVE_CONDITIONAL;
      result->verdict_description = ggc_strdup ("Conditional transfer (some paths have invalidation)");
    }
  } else {
    result->verdict = MOVE_IMPOSSIBLE;
    result->verdict_description = ggc_strdup ("No transfer (no invalidation points found)");
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeAllOwnershipMoves_convertVerdict
// ----------------------------------------------------------------------------
// 转换判定类型

field_analysis::FieldMoveVerdict analyzeAllOwnershipMoves_convertVerdict (OwnershipMoveVerdict verdict) {
  switch (verdict) {
    case MOVE_CERTAIN:
      return field_analysis::FIELD_MOVE_CERTAIN;
    case MOVE_IMPOSSIBLE:
      return field_analysis::FIELD_MOVE_IMPOSSIBLE;
    case MOVE_CONDITIONAL:
      return field_analysis::FIELD_MOVE_CONDITIONAL;
    default:
      return field_analysis::FIELD_MOVE_NOT_APPLICABLE;
  }
}

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// analyzeOwnershipMove
// ----------------------------------------------------------------------------
// 主入口：分析单个字段写入的所有权转移

ArrayDetectErrorCode analyzeOwnershipMove (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  WriteOriginalSource* source,
  OwnershipMoveResult*& result
) AD_FUNCTION_BEGIN {
  if (!write_info || !source) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 检查源操作数是否为字段访问
  if (source->source_type != SOURCE_FIELD_ACCESS) {
    // 不是字段访问，不适用
    result = ggc_alloc<OwnershipMoveResult> ();
    if (!result) {
      AD_RETURNE (MEMORY_ERROR);
    }
    memset (result, 0, sizeof (OwnershipMoveResult));
    result->verdict = MOVE_NOT_APPLICABLE;
    result->is_analyzed = true;
    result->verdict_description = ggc_strdup ("Not a field access source");
    AD_RETURNE (OK);
  }

  // 提取字段访问信息
  FieldAccessSource* field_access = &source->data.field_access;

  result = ggc_alloc<OwnershipMoveResult> ();
  if (!result) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (result, 0, sizeof (OwnershipMoveResult));

  // 填充基本信息
  result->source_field = field_access->field_decl;
  result->source_object = field_access->base_object;
  result->source_field_decl = field_access->field_decl;
  result->source_field_name = field_access->field_name;
  result->transfer_stmt = write_info->stmt;
  result->transfer_location = write_info->location;

  // 查找销毁点
  AD_TRY (analyzeOwnershipMove_findInvalidationPoints (
    AD_ARGS,
    write_info->stmt,
    write_info->bb,
    result->source_field_decl,
    result->source_object,
    result->invalidation_points
  ));

  // 分析路径
  AD_TRY (analyzeOwnershipMove_analyzePaths (
    AD_ARGS,
    write_info->bb,
    result->invalidation_points,
    result->paths_with_invalidation,
    result->paths_without_invalidation,
    result->total_exit_paths
  ));

  // 确定结论
  AD_TRY (analyzeOwnershipMove_determineVerdict (AD_ARGS, result));

  result->is_analyzed = true;

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// analyzeAllOwnershipMoves
// ----------------------------------------------------------------------------
// Pipeline 接口：分析所有字段写入操作的所有权转移

ArrayDetectErrorCode analyzeAllOwnershipMoves (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_certain_moves
) AD_FUNCTION_BEGIN {
  total_analyzed = 0;
  total_certain_moves = 0;

  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }

  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;

  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldWriteOps* tfwo = (*iter).second;
    if (!tfwo || !tfwo->writes) continue;

    for (unsigned int i = 0; i < tfwo->writes->length (); i++) {
      field_analysis::FieldWriteAnalysisWrapper* wrapper = (*tfwo->writes)[i];
      if (!wrapper) continue;

      // 只分析字段访问源
      if (wrapper->source_kind != field_analysis::FIELD_SRC_FIELD_ACCESS) {
        continue;
      }

      total_analyzed++;

      // 从 wrapper 创建临时 FieldWriteInfo
      FieldWriteInfo temp_write_info;
      memset (&temp_write_info, 0, sizeof (FieldWriteInfo));
      temp_write_info.type = wrapper->type;
      temp_write_info.field_decl = wrapper->field;
      temp_write_info.function_decl = wrapper->func;
      temp_write_info.bb = wrapper->bb;
      temp_write_info.stmt = wrapper->stmt;
      temp_write_info.lhs = wrapper->lhs;
      temp_write_info.rhs = wrapper->rhs;
      temp_write_info.location = wrapper->write_location;

      // 从 wrapper 创建临时 WriteOriginalSource
      WriteOriginalSource temp_source;
      memset (&temp_source, 0, sizeof (WriteOriginalSource));
      temp_source.source_type = SOURCE_FIELD_ACCESS;
      temp_source.data.field_access.access_stmt = wrapper->source_data.field_access.stmt;
      temp_source.data.field_access.access_expr = NULL_TREE;
      temp_source.data.field_access.field_decl = wrapper->source_data.field_access.field;
      temp_source.data.field_access.field_name = wrapper->source_data.field_access.field_name;
      temp_source.data.field_access.object_type = wrapper->source_data.field_access.object_type;
      temp_source.data.field_access.type_name = wrapper->source_data.field_access.type_name;
      temp_source.data.field_access.base_object = wrapper->source_data.field_access.object;
      temp_source.data.field_access.location = wrapper->source_data.field_access.location;

      // 执行分析
      OwnershipMoveResult* move_result = NULL;
      AD_TRY (analyzeOwnershipMove (
        AD_ARGS,
        &temp_write_info,
        &temp_source,
        move_result
      ));

      if (move_result) {
        // 存储结果到 wrapper->move (转换到 FieldMoveAnalysis)
        field_analysis::FieldMoveAnalysis* field_move = ggc_alloc<field_analysis::FieldMoveAnalysis> ();
        if (field_move) {
          memset (field_move, 0, sizeof (field_analysis::FieldMoveAnalysis));
          field_move->source_field = move_result->source_field;
          field_move->source_object = move_result->source_object;
          field_move->transfer_stmt = move_result->transfer_stmt;
          field_move->transfer_location = move_result->transfer_location;
          field_move->invalidations = NULL;
          field_move->verdict = analyzeAllOwnershipMoves_convertVerdict (move_result->verdict);
          field_move->paths_with = move_result->paths_with_invalidation;
          field_move->paths_without = move_result->paths_without_invalidation;
          field_move->total_paths = move_result->total_exit_paths;
          field_move->verdict_desc = move_result->verdict_description;
          wrapper->move = field_move;
        }

        // 打印结果（调试信息）
        printOwnershipMoveResult (AD_ARGS, ctx.debug_file, move_result);

        if (move_result->verdict == MOVE_CERTAIN) {
          total_certain_moves++;
        }
      }
    }
  }

  AD_DEBUG_PRINT ("ownershipMove: %u analyzed, %u certain",
                  total_analyzed, total_certain_moves);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// printOwnershipMoveResult
// ----------------------------------------------------------------------------
// 打印所有权转移结果

void printOwnershipMoveResult (
  AD_FUNC_ARGS,
  FILE* out,
  OwnershipMoveResult* result
) {
  (void)ctx;
  (void)gcc_ctx;

  if (!result || !out) {
    return;
  }

  fprintf (out, "\n=== Ownership Transfer Analysis Result ===\n");

  if (result->verdict == MOVE_NOT_APPLICABLE) {
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
    case MOVE_CERTAIN:
      verdict_str = "CERTAIN (ownership transferred)";
      break;
    case MOVE_IMPOSSIBLE:
      verdict_str = "IMPOSSIBLE (ownership shared)";
      break;
    case MOVE_CONDITIONAL:
      verdict_str = "CONDITIONAL (depends on control flow)";
      break;
    case MOVE_NOT_APPLICABLE:
      verdict_str = "NOT_APPLICABLE";
      break;
  }
  fprintf (out, "Verdict: %s\n", verdict_str);
  fprintf (out, "Description: %s\n", result->verdict_description ? result->verdict_description : "N/A");

  fprintf (out, "==========================================\n");
}

} // namespace array_detect_ns
