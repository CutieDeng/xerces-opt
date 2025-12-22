#include "prelude.hh"
#include "state.hh"
#include "field-write-collector.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "analysis-data.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// 辅助函数：查找或创建 type -> field 的写入操作列表
// 使用 hash_map 提供 O(1) 查找性能
// 输入/输出：map_ptr - hash_map 的指针的指针，如果为 NULL 则自动初始化
static ArrayDetectErrorCode findOrCreateTypeFieldWriteOps(
  AD_FUNC_ARGS,
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>** map_ptr,
  tree type,
  tree field_decl,
  TypeFieldWriteOps** out_tfwo
) AD_FUNCTION_BEGIN {
  // 检查输入参数
  if (!map_ptr) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 初始化 hash_map 如果尚未初始化
  if (!*map_ptr) {
    // 使用 ggc_alloc 分配内存（GCC 垃圾回收）
    // 注意：ggc_alloc<T>() 只分配内存，不调用构造函数
    hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* raw_ptr = 
        ggc_alloc<hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>>();
    if (!raw_ptr) {
      AD_RETURNE(MEMORY_ERROR);
    }
    // 使用 placement new 在已分配的内存上构造 hash_map 对象
    // 这是关键：ggc_alloc 只分配内存，必须使用 placement new 调用构造函数
    *map_ptr = new (raw_ptr) hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>();
    if (!*map_ptr) {
      AD_RETURNE(MEMORY_ERROR);
    }
    // hash_map 构造后需要调用 create_ggc() 来初始化内部哈希表
    (*map_ptr)->create_ggc(0);
  }
  
  // 检查输入参数
  if (!type || !field_decl) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 构造查找键
  TypeFieldKey key;
  key.type = type;
  key.field_decl = field_decl;
  
  // 使用 hash_map 进行 O(1) 查找
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* map = *map_ptr;
  
  // 检查 map 是否有效（双重检查，防御性编程）
  if (!map) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 验证 key 的有效性（防御性编程）
  if (!key.type || !key.field_decl) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 调用 get() 查找键
  TypeFieldWriteOps** existing_ptr = map->get(key);
  
  // 如果 existing_ptr 不为 NULL，说明键存在（即使值为 NULL）
  if (existing_ptr) {
    // 键存在，检查值是否为 NULL
    if (*existing_ptr) {
      // 找到现有的映射
      *out_tfwo = *existing_ptr;
      AD_RETURNE(OK);
    }
    // 键存在但值为 NULL，这种情况不应该发生（因为我们总是创建非 NULL 的值）
    // 但为了安全，我们继续创建新值并更新
  }
  
  // 未找到，创建新的映射
  TypeFieldWriteOps* tfwo = ggc_alloc<TypeFieldWriteOps>();
  if (!tfwo) {
    AD_RETURNE(MEMORY_ERROR);
  }
  memset(tfwo, 0, sizeof(TypeFieldWriteOps));
  
  tfwo->type = type;
  tfwo->field_decl = field_decl;
  tfwo->write_ops = ggc_alloc<vec<FieldWriteCapture*>>();
  if (!tfwo->write_ops) {
    AD_RETURNE(MEMORY_ERROR);
  }
  tfwo->write_ops->create(0);
  
  // 插入到 hash_map 中
  map->put(key, tfwo);
  *out_tfwo = tfwo;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 收集所有类型和字段：遍历编译单元提取字段写入操作
// 语义：扫描所有函数，对于每处 field access 的写入进行处理
// 输出：填充 detector.m_type_field_writes 容器，存储 (type, field, source-info, origin-src) 记录
ArrayDetectErrorCode collectTypesAndFields(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Collecting all types and fields (new implementation)");
  
  struct cgraph_node* node;
  size_t func_count = 0;
  size_t write_op_count = 0;
  
  AD_DEBUG_PRINT ("Starting function traversal...");
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun ();
    if (!fn) continue;
    
    func_count++;
    // 获取函数名称
    const char* func_name = node->name ();
    tree func_decl = node->decl;
    if (func_decl && DECL_NAME(func_decl)) {
      func_name = IDENTIFIER_POINTER (DECL_NAME (func_decl));
    }
    AD_DEBUG_PRINT ("Processing function %zu: %s", func_count, func_name);
  
  // 遍历函数中的语句，查找字段写入操作
  basic_block bb;
  FOR_EACH_BB_FN(bb, fn) {
    gimple_stmt_iterator gsi;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
      gimple* stmt = gsi_stmt (gsi);
      
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
      
      // Pipeline 第一步：只捕获字段写入信息，不向下解析
      // 创建字段写入捕获记录
      FieldWriteCapture* capture = ggc_alloc<FieldWriteCapture>();
      if (!capture) {
        AD_RETURNE(MEMORY_ERROR);
      }
      memset(capture, 0, sizeof(FieldWriteCapture));
      
      // 核心信息：类型和字段
      capture->type = containing_type;
      capture->field_decl = field_decl;
      
      // 上下文信息：函数和基本块
      capture->function_decl = func_decl;
      capture->bb = bb;
      capture->function_name = func_name ? ggc_strdup(func_name) : NULL;
      capture->bb_index = bb ? bb->index : -1;
      
      // GIMPLE 语句信息
      capture->stmt = stmt;
      capture->lhs = lhs;  // MEM
      capture->rhs = rhs;  // SSA_NAME
      
      // 源码位置
      capture->location = gimple_location(stmt);
      
      // 通用用途功能指针：初始化为 NULL
      capture->aux = NULL;
      
      // 查找或创建 type -> field 的写入操作列表
      TypeFieldWriteOps* tfwo = NULL;
      AD_TRY(findOrCreateTypeFieldWriteOps(AD_ARGS, &detector.m_type_field_writes, containing_type, field_decl, &tfwo));
      
      // 添加到列表
      tfwo->write_ops->safe_push(capture);
      write_op_count++;
      
      // 打印字段写入捕获调试信息（类型名、字段名、字段类型名）
      gcc_ext_util::logFieldWriteCapture(AD_ARGS, containing_type, field_decl);
    }
  }

  AD_DEBUG_PRINT ("Function process end, %zu: %s", func_count, func_name);
}
  
AD_DEBUG_PRINT ("Collection complete: %zu functions processed, %zu write operations found", func_count, write_op_count);

AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
