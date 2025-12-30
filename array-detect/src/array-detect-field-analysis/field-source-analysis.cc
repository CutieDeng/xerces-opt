#include "field-source-analysis.hh"

namespace array_detect_ns {

// 从写入操作中提取源变量
ArrayDetectErrorCode extractSourceVariables (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<vec<FieldWriteCapture*>*> const &field_writes,
  vec<vec<tree>*> &field_sources
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT ("Extracting source variables from write operations");
  
  if (field_decls.length () != field_writes.length ()) {
    AD_RETURNE (LOGICAL_ERROR);
  }
  
  // 初始化输出向量
  field_sources.create (0);
  
  // 遍历所有字段的写入操作
  for (unsigned int i = 0; i < field_decls.length (); i++) {
    (void) field_decls[i];  // field_decl available for future use
    vec<FieldWriteCapture*>* write_ops = field_writes[i];
    
    if (!write_ops) {
      vec<tree>* empty_list = ggc_alloc<vec<tree>>();
      empty_list->create (0);
      field_sources.safe_push (empty_list);
      continue;
    }
    
    // 创建源变量列表
    vec<tree>* source_list = ggc_alloc<vec<tree>>();
    source_list->create (0);
    
    // 从写入操作中提取源变量（SSA_NAME）
    for (unsigned int j = 0; j < write_ops->length (); j++) {
      FieldWriteCapture * write_op = (*write_ops)[j];
      if (!write_op) {
        continue;
      }
      
      tree rhs_value = write_op->rhs;
      
      // 如果右值是 SSA_NAME，直接添加
      if (TREE_CODE (rhs_value) == SSA_NAME) {
        // 检查是否已存在
        bool exists = false;
        for (unsigned int k = 0; k < source_list->length (); k++) {
          if ((*source_list)[k] == rhs_value) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          source_list->safe_push (rhs_value);
        }
      }
    }
    
    // 添加到列表
    field_sources.safe_push (source_list);
    AD_DEBUG_PRINT ("  Field %u has %u source variables", i, source_list->length ());
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
