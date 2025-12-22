  #include "prelude.hh"
  #include "state.hh"
  #include "array-detector-driver.hh"
  #include "array-detector.hh"
  #include "context-init.hh"
  #include "gcc-ext-util.hh"
  #include "field-analysis-main.hh"
  #include "analysis-data.hh"

  namespace array_detect_ns {}

  namespace array_detector {

  using namespace ::array_detect_ns;

  // 字段赋值追踪：分析字段赋值来源
  // 语义：执行两步分析 - 赋值分析 -> 候选判断
  // 前置条件：字段已通过 collectTypesAndFields 收集
  ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
    AD_DEBUG_PRINT ("Tracing field assignments");
    
    // 使用新的分析流程
    vec<tree> field_decls;
    vec<FieldAnalysisResult*> field_results;
    
    // 执行完整的字段分析
    AD_TRY (performFieldAnalysis (AD_ARGS, detector, field_decls, field_results));
    
    // 更新 FieldInfo 的 is_array_candidate 字段
    AD_TRY (updateFieldInfoFromResults (AD_ARGS, field_decls, field_results, detector));
    
    AD_RETURNE (OK);
  } AD_FUNCTION_END


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
    // 根据 GCC hash_map 的实现，get() 返回 Value*，对于 Value=TypeFieldWriteOps*，返回 TypeFieldWriteOps**
    // 如果键不存在，get() 返回 NULL
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
    // 注意：根据 GCC 源码，get() 的实现是：
    //   Value* get(const Key& k) {
    //     hash_entry& e = m_table.find_with_hash(k, Traits::hash(k));
    //     return Traits::is_empty(e) ? NULL : &e.m_value;
    //   }
    // 如果 is_empty() 返回 true，get() 返回 NULL
    // 如果 is_empty() 返回 false，get() 返回 &e.m_value（即指向 Value 的指针）
    // 对于 Value=TypeFieldWriteOps*，返回 TypeFieldWriteOps**
    TypeFieldWriteOps** existing_ptr = map->get(key);
    
    // 如果 existing_ptr 不为 NULL，说明键存在（即使值为 NULL）
    if (existing_ptr) {
      // 键存在，检查值是否为 NULL
      // 注意：existing_ptr 是指向 TypeFieldWriteOps* 的指针
      // *existing_ptr 是 TypeFieldWriteOps* 类型的值
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
        
        // Pipeline 链接：供下一轮解析管线使用（初始化为 NULL）
        capture->next = NULL;
        
        // 查找或创建 type -> field 的写入操作列表
        TypeFieldWriteOps* tfwo = NULL;
        AD_TRY(findOrCreateTypeFieldWriteOps(AD_ARGS, &detector.m_type_field_writes, containing_type, field_decl, &tfwo));
        
        // 添加到列表
        tfwo->write_ops->safe_push(capture);
        write_op_count++;
        
        AD_DEBUG_PRINT ("  Captured field write: type=%p, field=%p, function=%s, bb=%d", 
                        (void*)containing_type, (void*)field_decl, func_name, bb->index);
      }
    }

    AD_DEBUG_PRINT ("Function process end, %zu: %s", func_count, func_name);
  }
  
  AD_DEBUG_PRINT ("Collection complete: %zu functions processed, %zu write operations found", func_count, write_op_count);
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

ArrayDetectErrorCode analyzeFieldAssignmentsInFunctions(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Analyzing field assignments in functions");
  
  // 遍历所有函数
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun ();
    if (!fn) continue;
    
    // 获取函数名称（尝试获取可读的名称）
    const char* func_name = node->name ();
    tree decl = node->decl;
    if (decl && DECL_NAME(decl)) {
      func_name = IDENTIFIER_POINTER (DECL_NAME (decl));
    }
    // 如果还是空，使用mangled name
    if (!func_name || strlen (func_name) == 0) {
      func_name = node->name();
    }
    
    // 获取函数所属的类型（对于成员函数）
    const char* containing_type_name = NULL;
    if (decl) {
      tree context = DECL_CONTEXT (decl);
      if (context) {
        if (TREE_CODE (context) == RECORD_TYPE || TREE_CODE (context) == UNION_TYPE) {
          AD_TRY (gcc_ext_util::get_type_name (AD_ARGS, context, containing_type_name));
        } else if (TREE_CODE (context) == NAMESPACE_DECL) {
          // 命名空间中的函数
          if (DECL_NAME(context)) {
            containing_type_name = IDENTIFIER_POINTER (DECL_NAME (context));
          }
        }
      }
    }
    
    // 输出调试信息，包含函数所属类型
    if (containing_type_name) {
      AD_DEBUG_PRINT ("Analyzing function: %s::%s", containing_type_name, func_name);
    } else {
      AD_DEBUG_PRINT ("Analyzing function: %s", func_name);
    }
    
    // 添加函数基本信息调试
    AD_DEBUG_PRINT ("Function has %d basic blocks", n_basic_blocks_for_fn (fn));
    
    // 遍历函数中的所有基本块
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      AD_DEBUG_PRINT ("Processing basic block %d", bb->index);
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt (gsi);
        AD_DEBUG_PRINT ("  Statement type: %s", gimple_code_name[gimple_code (stmt)]);
        
        // 检查是否是赋值语句
        if (gimple_code (stmt) == GIMPLE_ASSIGN) {
          AD_DEBUG_PRINT ("  Found assignment statement, analyzing...");
          // 打印具体的赋值语句代码和位置信息
          AD_TRY (gcc_ext_util::get_source_location_string (AD_ARGS, gimple_location (stmt), ctx.source_location_buffer, ctx.source_location_buffer_size));
          fprintf (ctx.debug_file, "  Assignment statement at %s:\n", ctx.source_location_buffer);
          print_gimple_stmt (ctx.debug_file, stmt, 4, TDF_DETAILS);
          gcc_ext_util::analyze_gimple_assignment (AD_ARGS, stmt, detector, func_name, decl);
        }
        // 检查是否是GIMPLE_CALL语句（可能是通过调用赋值）
        else if (gimple_code (stmt) == GIMPLE_CALL) {
          AD_DEBUG_PRINT ("  Found call statement, skipping for now");
          // 这里可以处理通过函数调用返回值的赋值
          // 简化处理：暂时跳过
        }
        else {
          AD_DEBUG_PRINT ("  Skipping statement type: %s", gimple_code_name[gimple_code (stmt)]);
        }
      }
    }
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
