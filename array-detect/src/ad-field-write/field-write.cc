// ============================================================================
// ad-field-write 模块实现
// ============================================================================
// 收集所有字段写入操作
// 数据流：whole-program -> (mapof (type, field) (listof FieldWriteAnalysisWrapper))
// ============================================================================

#include "field-write.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-analysis.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 内部实现
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// collectAllFieldWrites_initMap
// ----------------------------------------------------------------------------
// 初始化 type-field -> writes 的 hash_map

ArrayDetectErrorCode collectAllFieldWrites_initMap (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>*& map
) AD_FUNCTION_BEGIN {
  (void) ctx;
  (void) gcc_ctx;

  auto* raw_ptr = ggc_alloc<hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>>();
  if (!raw_ptr) {
    AD_RETURNE (MEMORY_ERROR);
  }

  map = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>();
  if (!map) {
    AD_RETURNE (MEMORY_ERROR);
  }
  map->create_ggc (0);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap
// ----------------------------------------------------------------------------
// 将 FieldWriteAnalysisWrapper 插入到 (type, field) 对应的写入列表中
//
// 语义：(map, type, field, wrapper) -> map[type,field].writes.push(wrapper)
// 前置条件：map 已初始化（非 NULL）

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map,
  tree type,
  tree field_decl,
  FieldWriteAnalysisWrapper* wrapper
) AD_FUNCTION_BEGIN {
  if (!map || !type || !field_decl || !wrapper) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;

  TypeFieldWriteOps** existing_ptr = map->get (key);
  TypeFieldWriteOps* tfwo = NULL;

  if (existing_ptr && *existing_ptr) {
    tfwo = *existing_ptr;
  } else {
    tfwo = ggc_alloc<TypeFieldWriteOps>();
    if (!tfwo) {
      AD_RETURNE (MEMORY_ERROR);
    }
    memset (tfwo, 0, sizeof (TypeFieldWriteOps));

    tfwo->type = type;
    tfwo->field_decl = field_decl;
    tfwo->writes = ggc_alloc<vec<FieldWriteAnalysisWrapper*>>();
    if (!tfwo->writes) {
      AD_RETURNE (MEMORY_ERROR);
    }
    tfwo->writes->create (0);
    tfwo->has_rejecting_evidence = false;
    tfwo->reserved = NULL;

    map->put (key, tfwo);
  }

  tfwo->writes->safe_push (wrapper);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper
// ----------------------------------------------------------------------------
// 从字段写入信息创建 FieldWriteAnalysisWrapper 并插入到 map

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper (
  AD_FUNC_ARGS,
  gimple* stmt,
  tree lhs,
  tree rhs,
  tree field_decl,
  tree containing_type,
  basic_block bb,
  tree func_decl,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map
) AD_FUNCTION_BEGIN {

  FieldWriteAnalysisWrapper* wrapper = ggc_alloc<FieldWriteAnalysisWrapper>();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (FieldWriteAnalysisWrapper));

  // FieldWrite 部分
  wrapper->type = containing_type;
  wrapper->field = field_decl;
  wrapper->func = func_decl;
  wrapper->bb = bb;
  wrapper->stmt = stmt;
  wrapper->lhs = lhs;
  wrapper->rhs = rhs;
  wrapper->write_location = gimple_location (stmt);

  // 其他部分由后续模块填充
  wrapper->source_kind = field_analysis::FIELD_SRC_UNKNOWN;
  wrapper->source_operand = NULL_TREE;
  wrapper->source_stmt = NULL;
  wrapper->all_uses = NULL;
  wrapper->total_use_count = 0;
  wrapper->max_use_depth = 0;
  wrapper->is_fully_analyzed = false;
  wrapper->escape_uses = NULL;
  wrapper->escape_count = 0;
  wrapper->has_escape = false;
  wrapper->total_escapes = 0;
  wrapper->safe_debug_escapes = 0;
  wrapper->rejecting_escapes = 0;
  wrapper->has_rejecting_evidence = false;
  wrapper->move = NULL;

  AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap (
    AD_ARGS, map, containing_type, field_decl, wrapper));

  gcc_ext_util::logFieldWriteCapture (AD_ARGS, containing_type, field_decl);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction_scanBasicBlock
// ----------------------------------------------------------------------------
// 扫描基本块，收集字段写入并插入到 map

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock (
  AD_FUNC_ARGS,
  basic_block bb,
  tree func_decl,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  gimple_stmt_iterator gsi;
  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
    gimple* stmt = gsi_stmt (gsi);

    FieldWriteInfo* write_info = NULL;
    AD_TRY (collectAllFieldWrites_checkStatement (AD_ARGS, stmt, bb, func_decl, write_info));

    if (write_info) {
      AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper (
        AD_ARGS,
        write_info->stmt,
        write_info->lhs,
        write_info->rhs,
        write_info->field_decl,
        write_info->type,
        write_info->bb,
        write_info->function_decl,
        map
      ));
      write_count++;
    }
  }
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction
// ----------------------------------------------------------------------------
// 扫描单个函数，收集字段写入并插入到 map

ArrayDetectErrorCode collectAllFieldWrites_scanFunction (
  AD_FUNC_ARGS,
  struct cgraph_node* node,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  if (node->inlined_to) {
    AD_RETURNE (OK);
  }
  if (!node->has_gimple_body_p ()) {
    AD_RETURNE (OK);
  }

  function* fn = node->get_fun ();
  if (!fn) {
    AD_RETURNE (OK);
  }

  tree func_decl = node->decl;
  char const* func_name = node->name ();
  if (func_decl && DECL_NAME (func_decl)) {
    func_name = IDENTIFIER_POINTER (DECL_NAME (func_decl));
  }
  AD_DEBUG_PRINT ("Scanning function: %s", func_name);

  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock (AD_ARGS, bb, func_decl, map, write_count));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // anonymous namespace

// ============================================================================
// 公开接口实现
// ============================================================================

// ----------------------------------------------------------------------------
// collectAllFieldWrites_checkStatement
// ----------------------------------------------------------------------------

ArrayDetectErrorCode collectAllFieldWrites_checkStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  if (gimple_code (stmt) != GIMPLE_ASSIGN) {
    AD_RETURNE (OK);
  }

  tree lhs = gimple_assign_lhs (stmt);
  tree rhs = gimple_assign_rhs1 (stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));

  if (!is_field_access0 || !field_decl || !object) {
    AD_RETURNE (OK);
  }

  tree object_type = TREE_TYPE (object);
  if (!object_type) {
    AD_RETURNE (OK);
  }

  if (TREE_CODE (object_type) == REFERENCE_TYPE) {
    object_type = TREE_TYPE (object_type);
    if (!object_type) {
      AD_RETURNE (OK);
    }
  }
  if (TREE_CODE (object_type) == POINTER_TYPE) {
    object_type = TREE_TYPE (object_type);
    if (!object_type) {
      AD_RETURNE (OK);
    }
  }

  tree containing_type = TYPE_MAIN_VARIANT (object_type);
  if (!containing_type) {
    AD_RETURNE (OK);
  }

  FieldWriteInfo* write_info = ggc_alloc<FieldWriteInfo>();
  if (!write_info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (write_info, 0, sizeof (FieldWriteInfo));

  write_info->type = containing_type;
  write_info->field_decl = field_decl;
  write_info->function_decl = func_decl;
  write_info->bb = bb;
  write_info->stmt = stmt;
  write_info->lhs = lhs;
  write_info->rhs = rhs;
  write_info->location = gimple_location (stmt);
  write_info->bb_index = bb->index;

  if (func_decl && DECL_NAME (func_decl)) {
    write_info->function_name = IDENTIFIER_POINTER (DECL_NAME (func_decl));
  } else {
    write_info->function_name = "<unknown>";
  }

  result = write_info;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites
// ----------------------------------------------------------------------------

ArrayDetectErrorCode collectAllFieldWrites (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Collecting all field writes");

  if (!detector.m_type_field_writes) {
    AD_TRY (collectAllFieldWrites_initMap (AD_ARGS, detector.m_type_field_writes));
  }

  struct cgraph_node* node;
  unsigned int func_count = 0;
  unsigned int total_write_count = 0;

  AD_DEBUG_PRINT ("Starting function traversal...");
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    unsigned int write_count = 0;
    AD_TRY (collectAllFieldWrites_scanFunction (AD_ARGS, node, detector.m_type_field_writes, write_count));
    total_write_count += write_count;
    func_count++;
  }

  AD_DEBUG_PRINT ("Collection complete: %u functions, %u writes",
                  func_count, total_write_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
