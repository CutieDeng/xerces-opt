#include "prelude.hh"
#include "state.hh"
#include "field-write-collector.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// 辅助函数：查找或创建 type -> field 的写入操作列表
// 使用 hash_map 提供 O (1) 查找性能
// 输入/输出：map_ptr - hash_map 的指针的指针，如果为 NULL 则自动初始化
static ArrayDetectErrorCode findOrCreateTypeFieldWriteOps (
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>** map_ptr,
  tree type,
  tree field_decl,
  TypeFieldWriteOps** result
) AD_FUNCTION_BEGIN {
  // 检查输入参数
  if (!map_ptr) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 初始化 hash_map 如果尚未初始化
  if (!*map_ptr) {
    // 使用 ggc_alloc 分配内存（GCC 垃圾回收）
    // 注意：ggc_alloc<T>() 只分配内存，不调用构造函数
    hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* raw_ptr = 
        ggc_alloc<hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>>();
    if (!raw_ptr) {
      AD_RETURNE (MEMORY_ERROR);
    }
    // 使用 placement new 在已分配的内存上构造 hash_map 对象
    // 这是关键：ggc_alloc 只分配内存，必须使用 placement new 调用构造函数
    *map_ptr = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>();
    if (!*map_ptr) {
      AD_RETURNE (MEMORY_ERROR);
    }
    // hash_map 构造后需要调用 create_ggc () 来初始化内部哈希表
    (*map_ptr)->create_ggc (0);
  }
  
  // 检查输入参数
  if (!type || !field_decl) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 构造查找键
  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;
  
  // 使用 hash_map 进行 O (1) 查找
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map = *map_ptr;
  
  // 检查 map 是否有效（双重检查，防御性编程）
  if (!map) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 验证 key 的有效性（防御性编程）
  if (!key.type || !key.field_decl) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  // 调用 get () 查找键
  TypeFieldWriteOps** existing_ptr = map->get (key);
  
  // 如果 existing_ptr 不为 NULL，说明键存在（即使值为 NULL）
  if (existing_ptr) {
    // 键存在，检查值是否为 NULL
    if (*existing_ptr) {
      // 找到现有的映射
      *result = *existing_ptr;
      AD_RETURNE (OK);
    }
    // 键存在但值为 NULL，这种情况不应该发生（因为我们总是创建非 NULL 的值）
    // 但为了安全，我们继续创建新值并更新
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

  // 初始化字段级别分析结果
  tfwo->has_rejecting_evidence = false;
  tfwo->reserved = NULL;
  
  // 插入到 hash_map 中
  map->put (key, tfwo);
  *result = tfwo;
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 收集所有类型和字段：遍历编译单元提取字段写入操作
// 语义：扫描所有函数，对于每处 field access 的写入进行处理
// 输出：填充 detector.m_type_field_writes 容器，存储 (type, field, source-info, origin-src) 记录
ArrayDetectErrorCode collectTypesAndFields (ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Collecting all types and fields (new implementation)");
  
  struct cgraph_node * node;
  size_t func_count = 0;
  size_t write_op_count = 0;
  
  AD_DEBUG_PRINT ("Starting function traversal...");
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    // 防止访问已被内联释放的函数体
    if (node->inlined_to)
      continue;
    if (!node->has_gimple_body_p ())
      continue;

    function * fn = node->get_fun ();
    if (!fn) continue;
    
    func_count++;
    // 获取函数名称
    char const * func_name = node->name ();
    tree func_decl = node->decl;
    if (func_decl && DECL_NAME (func_decl)) {
      func_name = IDENTIFIER_POINTER (DECL_NAME (func_decl));
    }
    AD_DEBUG_PRINT ("Processing function %zu: %s", func_count, func_name);
  
  // 遍历函数中的语句，查找字段写入操作
  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    gimple_stmt_iterator gsi;
    for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
      gimple * stmt = gsi_stmt (gsi);
      
      // 只处理赋值语句
      if (gimple_code (stmt) != GIMPLE_ASSIGN) {
        continue;
      }
      
      tree lhs = gimple_assign_lhs (stmt);
      tree rhs = gimple_assign_rhs1 (stmt);
      
      // 检查左值是否是字段访问
      tree field_decl = NULL_TREE;
      tree object = NULL_TREE;
      bool is_field_access0;
      AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));
      
      if (!is_field_access0 || !field_decl || !object) {
        continue;
      }
      
      // 找到字段访问，获取包含类型
      tree object_type = TREE_TYPE (object);
      if (!object_type) continue;
      
      // 如果是引用类型，获取其基础类型
      if (TREE_CODE (object_type) == REFERENCE_TYPE) {
        object_type = TREE_TYPE (object_type);
        if (!object_type) continue;
      }
      
      // 如果是指针类型，获取其指向的类型
      if (TREE_CODE (object_type) == POINTER_TYPE) {
        object_type = TREE_TYPE (object_type);
        if (!object_type) continue;
      }
      
      tree containing_type = TYPE_MAIN_VARIANT (object_type);
      if (!containing_type) continue;
      
      // Pipeline 第一步：创建 FieldWriteAnalysisWrapper，填充 FieldWrite 部分
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
      wrapper->lhs = lhs;  // MEM_REF/COMPONENT_REF
      wrapper->rhs = rhs;  // SSA_NAME
      wrapper->write_location = gimple_location (stmt);

      // ========== FieldWriteSource 部分：由 trace-source 模块填充 ==========
      wrapper->source_kind = field_analysis::FIELD_SRC_UNKNOWN;
      // source_data union 已由 memset 清零

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

      // ========== FieldMoveAnalysis 部分：由 analyze-move 模块填充（可选）==========
      wrapper->move = NULL;

      // 查找或创建 type -> field 的写入操作列表
      TypeFieldWriteOps * tfwo = NULL;
      AD_TRY (findOrCreateTypeFieldWriteOps (AD_ARGS, &detector.m_type_field_writes, containing_type, field_decl, &tfwo));

      // 添加 Wrapper 到列表
      tfwo->writes->safe_push (wrapper);
      write_op_count++;
      
      // 打印字段写入捕获调试信息（类型名、字段名、字段类型名）
      gcc_ext_util::logFieldWriteCapture (AD_ARGS, containing_type, field_decl);
    }
  }

  AD_DEBUG_PRINT ("Function process end, %zu: %s", func_count, func_name);
}
  
AD_DEBUG_PRINT ("Collection complete: %zu functions processed, %zu write operations found", func_count, write_op_count);

AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
