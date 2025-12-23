#include "field-analysis-main.hh"
#include "field-analysis-integration.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// 执行完整的字段分析流程
ArrayDetectErrorCode performFieldAnalysis (
  AD_FUNC_ARGS,
  ArrayDetector const &detector,
  vec<tree> &field_decls,
  vec<FieldAnalysisResult*> &field_results
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT ("=== Starting field analysis ===");
  
  // 检查 detector 是否已初始化
  if (!detector.m_fields) {
    AD_RETURNE (NOT_INITIALIZED);
  }
  
  // 存储所有函数的分析结果
  vec<FunctionAnalysisResult*> all_function_results;
  all_function_results.create (0);
  
  // 逐个函数分析
  struct cgraph_node * node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node) {
    function * fn = node->get_fun ();
    if (!fn) {
      continue;
    }
    
    // 获取函数名称
    char const * func_name = node->name ();
    tree decl = node->decl;
    if (decl && DECL_NAME (decl)) {
      func_name = IDENTIFIER_POINTER (DECL_NAME (decl));
    }
    if (!func_name || strlen (func_name) == 0) {
      func_name = node->name ();
    }
    
    AD_DEBUG_PRINT ("Analyzing function: %s", func_name);
    
    // 分析该函数的所有字段
    FunctionAnalysisResult * func_result = ggc_alloc<FunctionAnalysisResult>();
    memset (func_result, 0, sizeof (FunctionAnalysisResult));
    
    AD_TRY (analyzeFunctionFields (AD_ARGS, fn, func_name, decl, *func_result));
    
    // 只添加有结果的函数
    if (func_result->field_decls && func_result->field_decls->length () > 0) {
      all_function_results.safe_push (func_result);
    }
  }
  
  AD_DEBUG_PRINT ("Analyzed %u functions", all_function_results.length ());
  
  // 整合所有函数的分析结果
  AD_TRY (integrateFunctionResults (AD_ARGS, all_function_results, field_decls, field_results));
  
  AD_DEBUG_PRINT ("=== Field analysis completed ===");
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// 更新 FieldInfo 的 is_array_candidate 字段
ArrayDetectErrorCode updateFieldInfoFromResults (
  AD_FUNC_ARGS,
  vec<tree> const &field_decls,
  vec<FieldAnalysisResult*> const &field_results,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  if (field_decls.length () != field_results.length ()) {
    AD_RETURNE (LOGICAL_ERROR);
  }
  
  if (!detector.m_fields) {
    AD_RETURNE (NOT_INITIALIZED);
  }
  
  AD_DEBUG_PRINT ("Updating FieldInfo from analysis results");
  
  // 遍历所有字段结果
  for (unsigned int i = 0; i < field_decls.length (); i++) {
    tree field_decl = field_decls[i];
    FieldAnalysisResult * result = field_results[i];
    
    if (!result) {
      continue;
    }
    
    // 查找对应的 FieldInfo
    size_t field_count = 0;
    AD_TRY (getFieldCount (detector, AD_ARGS, &field_count));
    
    for (size_t j = 0; j < field_count; j++) {
      FieldInfo * field_info = NULL;
      AD_TRY (getField (detector, AD_ARGS, j, &field_info));
      
      if (field_info && field_info->field_decl == field_decl) {
        // 更新 is_array_candidate
        field_info->is_array_candidate = result->is_memory_owner;
        
        AD_DEBUG_PRINT ("Field %s::%s: is_memory_owner = %s, reason = %s",
                      field_info->containing_type, field_info->field_name,
                      result->is_memory_owner ? "true" : "false",
                      result->reason ? result->reason : "<no reason>");
        break;
      }
    }
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
