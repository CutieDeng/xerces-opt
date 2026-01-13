// ============================================================================
// ad-field-write 模块实现
// ============================================================================
// 收集所有字段写入操作
// 数据流：whole-program -> (mapof (type, field) (listof Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo))
// ============================================================================

#include "field-write.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 前向声明
// ============================================================================

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_checkStatement(
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
);

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
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>*& map
) AD_FUNCTION_BEGIN {
  (void) ctx;
  (void) gcc_ctx;

  auto* raw_ptr = ggc_alloc<hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>>();
  if (!raw_ptr) {
    AD_RETURNE (MEMORY_ERROR);
  }

  map = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>();
  if (!map) {
    AD_RETURNE (MEMORY_ERROR);
  }
  map->create_ggc (0);

  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap
// ----------------------------------------------------------------------------
// 将 Wrapper 插入到 (type, field) 对应的写入列表中
//
// 语义：(map, type, field, wrapper) -> map[type,field].writes.push(wrapper)
// 前置条件：map 已初始化（非 NULL）

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* map,
  tree type,
  tree field_decl,
  Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo* wrapper
) AD_FUNCTION_BEGIN {
  if (!map || !type || !field_decl || !wrapper) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;

  TypeFieldAnalysisData** existing_ptr = map->get (key);
  TypeFieldAnalysisData* tfad = NULL;

  if (existing_ptr && *existing_ptr) {
    tfad = *existing_ptr;
  } else {
    tfad = ggc_alloc<TypeFieldAnalysisData>();
    if (!tfad) {
      AD_RETURNE (MEMORY_ERROR);
    }
    memset (tfad, 0, sizeof (TypeFieldAnalysisData));

    tfad->type = type;
    tfad->field_decl = field_decl;
    vec_alloc (tfad->writes, 4);
    tfad->escape_conclude = NULL;
    tfad->transfer_stats = NULL;

    map->put (key, tfad);
  }

  vec_safe_push (tfad->writes, wrapper);
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ----------------------------------------------------------------------------
// collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper
// ----------------------------------------------------------------------------
// 从字段写入信息创建 Wrapper 并插入到 map

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* map
) AD_FUNCTION_BEGIN {
  if (!write_info) {
    AD_RETURNE (INVALID_ARGUMENT);
  }

  Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo* wrapper = ggc_alloc<Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo>();
  if (!wrapper) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (wrapper, 0, sizeof (Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo));

  // 存储 FieldWriteInfo 指针
  wrapper->write_info = write_info;

  // 其他部分由后续模块填充
  wrapper->write_source = NULL;
  wrapper->escape_conclude = NULL;
  wrapper->uses = NULL;

  AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper_insertToMap (
    AD_ARGS, map, write_info->type, write_info->field_decl, wrapper));

  gcc_ext_util::logFieldWriteCapture (AD_ARGS, write_info->type, write_info->field_decl);

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
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* map,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  gimple_stmt_iterator gsi;
  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
    gimple* stmt = gsi_stmt (gsi);

    FieldWriteInfo* write_info = NULL;
    AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock_checkStatement (AD_ARGS, stmt, bb, func_decl, write_info));

    if (write_info) {
      AD_TRY (collectAllFieldWrites_scanFunction_scanBasicBlock_createWrapper (
        AD_ARGS, write_info, map));
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
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* map,
  unsigned int& write_count
) AD_FUNCTION_BEGIN {
  // 跳过内联克隆：其代码已复制到目标函数，扫描会导致重复收集。
  // 依据：gcc/cgraph.h:1443 "For inline clones this points to the function they will be inlined into."
  if (node->inlined_to) {
    AD_RETURNE (OK);
  }

  // 跳过无 GIMPLE 体的函数：外部声明、thunk 等没有 GIMPLE 表示，无语句可分析。
  // 依据：gcc/cgraph.h:1309 "Functions can also be define externally or they can be thunks with no Gimple representation."
  if (!node->has_gimple_body_p ()) {
    AD_RETURNE (OK);
  }

  // WPA 阶段函数体可能不在内存中，get_fun() 返回 NULL。
  // 依据：gcc/cgraph.h:1313 "Note that at WPA stage, the function body may not be present in memory."
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

ArrayDetectErrorCode collectAllFieldWrites_scanFunction_scanBasicBlock_checkStatement(
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
) AD_FUNCTION_BEGIN {
  result = NULL;

  // 字段写入必须是赋值语句；GIMPLE_CALL 等其他语句不直接写入字段。
  // 依据：GCC GIMPLE 规范，字段存储通过 GIMPLE_ASSIGN 的 LHS 为 COMPONENT_REF/MEM_REF 实现。
  if (gimple_code (stmt) != GIMPLE_ASSIGN) {
    AD_RETURNE (OK);
  }

  tree lhs = gimple_assign_lhs (stmt);
  tree rhs = gimple_assign_rhs1 (stmt);

  tree field_decl = NULL_TREE;
  tree object = NULL_TREE;
  bool is_field_access0;
  AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));

  // 非字段访问（如局部变量赋值）不是本模块的收集目标。
  if (!is_field_access0 || !field_decl || !object) {
    AD_RETURNE (OK);
  }

  tree object_type = TREE_TYPE (object);
  if (!object_type) {
    AD_RETURNE (OK);
  }

  // 解引用指针/引用类型：obj->field 的 object 类型是 T*，需要获取 T。
  // 依据：GCC tree 类型系统，POINTER_TYPE/REFERENCE_TYPE 是包装类型，TREE_TYPE 获取被指向类型。
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

  // 优先使用 DECL_CONTEXT 获取包含类型（更准确，特别是对于模板类）
  // DECL_CONTEXT 返回字段声明所在的类型
  tree containing_type = NULL_TREE;
  if (field_decl && DECL_CONTEXT (field_decl)) {
    tree decl_ctx = DECL_CONTEXT (field_decl);
    AD_DEBUG_PRINT ("[field-write] DECL_CONTEXT(field_decl) code=%d, TYPE_P=%d",
                    TREE_CODE (decl_ctx), TYPE_P (decl_ctx) ? 1 : 0);
    if (TYPE_P (decl_ctx)) {
      containing_type = TYPE_MAIN_VARIANT (decl_ctx);
      // 打印类型名来调试
      char const* ctx_type_name = NULL;
      gcc_ext_util::formatTypeNameWithTemplateArgs (AD_ARGS, containing_type, ctx_type_name);
      AD_DEBUG_PRINT ("[field-write] DECL_CONTEXT type name: %s", ctx_type_name ? ctx_type_name : "(null)");
    }
  }

  // 如果 DECL_CONTEXT 不可用，回退到 object_type
  if (!containing_type) {
    AD_DEBUG_PRINT ("[field-write] DECL_CONTEXT fallback to object_type");
    containing_type = TYPE_MAIN_VARIANT (object_type);
  }
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
