// ============================================================================
// ad-field-write 模块实现
// ============================================================================
// 收集所有字段写入操作
// 数据流：遍历编译单元 -> (type, field) -> (listof FieldWriteInfo)
// ============================================================================

#include "../../include/ad-field-write/field-write.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-analysis.hh"

namespace array_detect_ns {

using namespace ::array_detector;

// ============================================================================
// 辅助函数：查找或创建 type -> field 的写入操作列表
// ============================================================================

static ArrayDetectErrorCode findOrCreateTypeFieldWriteOps (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>** map_ptr,
  tree type,
  tree field_decl,
  TypeFieldWriteOps** result
) AD_FUNCTION_BEGIN {
  if (!map_ptr) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 初始化 hash_map 如果尚未初始化
  if (!*map_ptr) {
    hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* raw_ptr =
        ggc_alloc<hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>>();
    if (!raw_ptr) {
      AD_RETURNE (MEMORY_ERROR);
    }
    *map_ptr = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>();
    if (!*map_ptr) {
      AD_RETURNE (MEMORY_ERROR);
    }
    (*map_ptr)->create_ggc (0);
  }

  if (!type || !field_decl) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  // 构造查找键
  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;

  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map = *map_ptr;
  if (!map) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  TypeFieldWriteOps** existing_ptr = map->get (key);

  if (existing_ptr && *existing_ptr) {
    *result = *existing_ptr;
    AD_RETURNE (OK);
  }

  // 未找到，创建新的映射
  TypeFieldWriteOps * tfwo = ggc_alloc<TypeFieldWriteOps>();
  if (!tfwo) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (tfwo, 0, sizeof (TypeFieldWriteOps));

  tfwo->type = type;
  tfwo->field_decl = field_decl;

  // 初始化写入分析 Wrapper 列表
  tfwo->writes = ggc_alloc<vec<FieldWriteAnalysisWrapper*>>();
  if (!tfwo->writes) {
    AD_RETURNE (MEMORY_ERROR);
  }
  tfwo->writes->create (0);

  tfwo->has_rejecting_evidence = false;
  tfwo->reserved = NULL;

  map->put (key, tfwo);
  *result = tfwo;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// collectAllFieldWrites_createWriteInfo
// ============================================================================
// 创建 FieldWriteInfo 并添加到 detector 的 hash_map 中

ArrayDetectErrorCode collectAllFieldWrites_createWriteInfo (
  AD_FUNC_ARGS,
  gimple* stmt,
  tree lhs,
  tree rhs,
  tree field_decl,
  tree containing_type,
  basic_block bb,
  tree func_decl,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {

  // 创建 FieldWriteAnalysisWrapper（统一结构）
  FieldWriteAnalysisWrapper * wrapper = ggc_alloc<FieldWriteAnalysisWrapper>();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (FieldWriteAnalysisWrapper));

  // ========== FieldWrite 部分：字段写入基本信息 ==========
  wrapper->type = containing_type;
  wrapper->field = field_decl;
  wrapper->func = func_decl;
  wrapper->bb = bb;
  wrapper->stmt = stmt;
  wrapper->lhs = lhs;
  wrapper->rhs = rhs;
  wrapper->write_location = gimple_location (stmt);

  // ========== FieldWriteSource 部分：由 trace-source 模块填充 ==========
  wrapper->source_kind = field_analysis::FIELD_SRC_UNKNOWN;

  // ========== FieldUseAnalysis 部分：由 analyze-uses 模块填充 ==========
  wrapper->source_operand = NULL_TREE;
  wrapper->source_stmt = NULL;
  wrapper->all_uses = NULL;
  wrapper->total_use_count = 0;
  wrapper->max_use_depth = 0;
  wrapper->is_fully_analyzed = false;
  wrapper->escape_uses = NULL;
  wrapper->escape_count = 0;
  wrapper->has_escape = false;

  // ========== FieldEscapeConclude 部分：由 conclude-escape 模块填充 ==========
  wrapper->total_escapes = 0;
  wrapper->safe_debug_escapes = 0;
  wrapper->rejecting_escapes = 0;
  wrapper->has_rejecting_evidence = false;

  // ========== FieldMoveAnalysis 部分：由 analyze-move 模块填充 ==========
  wrapper->move = NULL;

  // 查找或创建 type -> field 的写入操作列表
  TypeFieldWriteOps * tfwo = NULL;
  AD_TRY (findOrCreateTypeFieldWriteOps (AD_ARGS, &detector.m_type_field_writes, containing_type, field_decl, &tfwo));

  // 添加 Wrapper 到列表
  tfwo->writes->safe_push (wrapper);

  // 打印字段写入捕获调试信息
  gcc_ext_util::logFieldWriteCapture (AD_ARGS, containing_type, field_decl);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// collectAllFieldWrites_checkStatement
// ============================================================================
// 检查语句是否为字段写入

ArrayDetectErrorCode collectAllFieldWrites_checkStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  // 只处理赋值语句
  if (gimple_code (stmt) != GIMPLE_ASSIGN) {
    AD_RETURNE (OK);
  }

  tree lhs = gimple_assign_lhs (stmt);
  tree rhs = gimple_assign_rhs1 (stmt);

  // 检查左值是否是字段访问
  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));

  if (!is_field_access0 || !field_decl || !object) {
    AD_RETURNE (OK);
  }

  // 找到字段访问，获取包含类型
  tree object_type = TREE_TYPE (object);
  if (!object_type) {
    AD_RETURNE (OK);
  }

  // 如果是引用类型，获取其基础类型
  if (TREE_CODE (object_type) == REFERENCE_TYPE) {
    object_type = TREE_TYPE (object_type);
    if (!object_type) {
      AD_RETURNE (OK);
    }
  }

  // 如果是指针类型，获取其指向的类型
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

  // 创建临时 FieldWriteInfo 用于返回（实际存储使用 Wrapper）
  FieldWriteInfo * write_info = ggc_alloc<FieldWriteInfo>();
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

// ============================================================================
// collectAllFieldWrites_scanBasicBlock
// ============================================================================
// 扫描基本块中的所有语句

ArrayDetectErrorCode collectAllFieldWrites_scanBasicBlock (
  AD_FUNC_ARGS,
  basic_block bb,
  tree func_decl,
  ArrayDetector& detector,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  gimple_stmt_iterator gsi;
  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
    gimple * stmt = gsi_stmt (gsi);

    FieldWriteInfo * write_info = NULL;
    AD_TRY (collectAllFieldWrites_checkStatement (AD_ARGS, stmt, bb, func_decl, write_info));

    if (write_info) {
      // 创建并添加到 detector
      AD_TRY (collectAllFieldWrites_createWriteInfo (
        AD_ARGS,
        write_info->stmt,
        write_info->lhs,
        write_info->rhs,
        write_info->field_decl,
        write_info->type,
        write_info->bb,
        write_info->function_decl,
        detector
      ));
      write_count++;
    }
  }
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// collectAllFieldWrites_scanFunction
// ============================================================================
// 扫描单个函数的所有基本块

ArrayDetectErrorCode collectAllFieldWrites_scanFunction (
  AD_FUNC_ARGS,
  struct cgraph_node* node,
  ArrayDetector& detector,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  // 防止访问已被内联释放的函数体
  if (node->inlined_to) {
    AD_RETURNE (OK);
  }
  if (!node->has_gimple_body_p ()) {
    AD_RETURNE (OK);
  }

  function * fn = node->get_fun ();
  if (!fn) {
    AD_RETURNE (OK);
  }

  tree func_decl = node->decl;
  char const * func_name = node->name ();
  if (func_decl && DECL_NAME (func_decl)) {
    func_name = IDENTIFIER_POINTER (DECL_NAME (func_decl));
  }
  AD_DEBUG_PRINT ("Scanning function: %s", func_name);

  // 遍历函数中的基本块
  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    AD_TRY (collectAllFieldWrites_scanBasicBlock (AD_ARGS, bb, func_decl, detector, write_count));
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// collectAllFieldWrites
// ============================================================================
// 主入口：收集编译单元中所有字段写入操作

ArrayDetectErrorCode collectAllFieldWrites (
  AD_FUNC_ARGS,
  ArrayDetector& detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Collecting all field writes (new module)");

  struct cgraph_node * node;
  unsigned int func_count = 0;
  unsigned int total_write_count = 0;

  AD_DEBUG_PRINT ("Starting function traversal...");
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    unsigned int write_count = 0;
    AD_TRY (collectAllFieldWrites_scanFunction (AD_ARGS, node, detector, write_count));
    total_write_count += write_count;
    func_count++;
  }

  AD_DEBUG_PRINT ("Collection complete: %u functions processed, %u write operations found",
                  func_count, total_write_count);

  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
