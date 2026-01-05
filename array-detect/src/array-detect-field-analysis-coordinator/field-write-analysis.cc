#include "field-write-analysis.hh"
#include "gcc-ext-util.hh"
#include "array-detector.hh"

namespace array_detect_ns {

// 辅助函数：在平行 vec 中查找字段索引
static int findFieldIndex (vec<tree> const &field_decls, tree field_decl) {
  for (unsigned int i = 0; i < field_decls.length (); i++) {
    if (field_decls[i] == field_decl) {
      return (int)i;
    }
  }
  return -1;
}

// 分析函数中的字段写入操作
ArrayDetectErrorCode analyzeFunctionFieldWrites (
  AD_FUNC_ARGS,
  function * fn,
  char const * function_name,
  tree function_decl,
  vec<tree> &field_decls,
  vec<vec<FieldWriteCapture*>*> &field_writes
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  if (!fn || !function_name || !function_decl) {
    AD_RETURNE (INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT ("Analyzing field writes in function: %s", function_name);
  
  // 初始化输出向量
  field_decls.create (0);
  field_writes.create (0);
  
  // 遍历函数的所有基本块
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
      
      // 检查左值是否为字段访问
      tree field_decl = NULL_TREE;
      tree object = NULL_TREE;
      bool is_field_access0;
      AD_TRY (gcc_ext_util::is_field_access (AD_ARGS, lhs, &field_decl, &object, is_field_access0));
      
      if (!is_field_access0 || !field_decl) {
        continue;
      }
      
      // 查找或创建该字段的写入操作列表
      int field_index = findFieldIndex (field_decls, field_decl);
      vec<FieldWriteCapture*>* write_list = NULL;
      
      if (field_index < 0) {
        // 新字段，添加到列表
        field_decls.safe_push (field_decl);
        write_list = ggc_alloc<vec<FieldWriteCapture*>>();
        write_list->create (0);
        field_writes.safe_push (write_list);
        field_index = (int)(field_decls.length () - 1);
      } else {
        write_list = field_writes[field_index];
      }
      
      // 创建字段写入捕获记录（使用 FieldWriteCapture）
      FieldWriteCapture * capture = ggc_alloc<FieldWriteCapture>();
      if (!capture) {
        AD_RETURNE (MEMORY_ERROR);
      }
      memset (capture, 0, sizeof (FieldWriteCapture));
      
      // 获取包含类型
      tree object_type = TREE_TYPE (object);
      if (TREE_CODE (object_type) == POINTER_TYPE) {
        object_type = TREE_TYPE (object_type);
      }
      tree containing_type = TYPE_MAIN_VARIANT (object_type);
      
      // 设置基本字段
      capture->type = containing_type;
      capture->field_decl = field_decl;
      capture->function_decl = function_decl;
      capture->bb = bb;
      capture->function_name = ggc_strdup (function_name);
      capture->bb_index = bb->index;
      capture->stmt = stmt;
      capture->lhs = lhs;
      capture->rhs = rhs;
      capture->location = gimple_location (stmt);
      capture->aux = NULL;  // 供后续阶段使用
      
      // 添加到列表
      write_list->safe_push (capture);
      
      AD_DEBUG_PRINT ("  Found field write capture: type=%p, field=%p, function=%s, bb=%d", 
                      (void*)containing_type, (void*)field_decl, function_name, bb->index);
    }
  }
  
  AD_DEBUG_PRINT ("  Total fields with writes: %u", field_decls.length ());
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
