#include "field-analysis-integration.hh"
#include "field-write-analysis.hh"
#include "field-source-analysis.hh"
#include "field-escape-analysis.hh"
#include "field-owner-analysis.hh"
#include "field-dataflow-analysis.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// 分析单个函数的所有字段
ArrayDetectErrorCode analyzeFunctionFields(
  AD_FUNC_ARGS,
  function* fn,
  const char* function_name,
  tree function_decl,
  FunctionAnalysisResult &function_result
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  if (!fn || !function_name || !function_decl) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("=== Analyzing function: %s ===", function_name);
  
  // 初始化函数结果
  memset(&function_result, 0, sizeof(FunctionAnalysisResult));
  function_result.function_name = ggc_strdup(function_name);
  function_result.function_decl = function_decl;
  function_result.field_decls = ggc_alloc<vec<tree>>();
  function_result.field_decls->create(0);
  function_result.field_results = ggc_alloc<vec<FieldAnalysisResult*>>();
  function_result.field_results->create(0);
  
  // 阶段1：分析字段写入操作
  // map(field_decl -> list of FieldWriteCapture)
  vec<tree> field_decls;
  vec<vec<FieldWriteCapture*>*> field_writes;
  AD_TRY(analyzeFunctionFieldWrites(AD_ARGS, fn, function_name, function_decl, 
                                     field_decls, field_writes));
  
  AD_DEBUG_PRINT("  Found writes for %u fields", field_decls.length());
  
  if (field_decls.length() == 0) {
    AD_RETURNE(OK);
  }
  
  // 阶段2：提取源变量
  // map(field_decl -> list of source var)
  vec<vec<tree>*> field_sources;
  AD_TRY(extractSourceVariables(AD_ARGS, field_decls, field_writes, field_sources));
  
  AD_DEBUG_PRINT("  Extracted sources for %u fields", field_sources.length());
  
  // 阶段3：分析每个字段
  for (unsigned int i = 0; i < field_decls.length(); i++) {
    tree field_decl = field_decls[i];
    vec<FieldWriteCapture*>* write_ops = field_writes[i];
    vec<tree>* source_vars = i < field_sources.length() ? field_sources[i] : NULL;
    
    if (!write_ops) {
      continue;
    }
    
    // 创建字段分析结果
    FieldAnalysisResult* field_result = ggc_alloc<FieldAnalysisResult>();
    memset(field_result, 0, sizeof(FieldAnalysisResult));
    field_result->field_decl = field_decl;
    field_result->write_ops = write_ops;
    
    // 设置源变量列表
    if (source_vars) {
      field_result->source_vars = source_vars;
    } else {
      field_result->source_vars = ggc_alloc<vec<tree>>();
      field_result->source_vars->create(0);
    }
    
    // 阶段3a：分析逃逸
    vec<EscapeSite*>* escape_sites = ggc_alloc<vec<EscapeSite*>>();
    escape_sites->create(0);
    AD_TRY(analyzeFieldEscapes(AD_ARGS, field_decls, field_sources, i, *escape_sites));
    field_result->escape_sites = escape_sites;
    
    AD_DEBUG_PRINT("  Field %u has %u escape sites", i, escape_sites->length());
    
    // 阶段3b：提取源操作（如果不逃逸）
    if (escape_sites->length() == 0 && source_vars) {
      vec<SourceOperation*>* source_ops = ggc_alloc<vec<SourceOperation*>>();
      source_ops->create(0);
      AD_TRY(extractSourceOperations(AD_ARGS, *source_vars, *source_ops));
      field_result->sources = source_ops;
      
      AD_DEBUG_PRINT("  Field %u has %u source operations", i, source_ops->length());
      
      // 阶段3c：数据流分析
    DataFlowGraph dataflow_graph;
    AD_TRY(buildDataFlowGraph(AD_ARGS, field_decl, fn, *write_ops, dataflow_graph));
    
    // 追踪值来源
    if (write_ops->length() > 0) {
      FieldWriteCapture* first_write = (*write_ops)[0];
      vec<ValueSource*> value_sources;
      AD_TRY(traceValueSource(AD_ARGS, field_decl, first_write, value_sources));
      
      AD_DEBUG_PRINT("  Field %u: traced %u value sources", i, value_sources.length());
      for (unsigned int j = 0; j < value_sources.length(); j++) {
        ValueSource* source = value_sources[j];
        if (source && source->description) {
          AD_DEBUG_PRINT("    Source %u: %s", j, source->description);
        }
      }
    }
    
    // 阶段3d：判定是否为内存持有者
    AD_TRY(determineMemoryOwner(AD_ARGS, field_decl, *write_ops, *escape_sites, 
                                 *source_ops, *field_result));
    } else {
      // 有逃逸，不是内存持有者
      field_result->is_memory_owner = false;
      field_result->reason = ggc_strdup("value escapes");
      field_result->sources = ggc_alloc<vec<SourceOperation*>>();
      field_result->sources->create(0);
    }
    
    // 添加到函数结果
    function_result.field_decls->safe_push(field_decl);
    function_result.field_results->safe_push(field_result);
  }
  
  AD_DEBUG_PRINT("=== Function analysis completed ===");
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 辅助函数：在列表中查找字段索引
static int findFieldIndexInResults(vec<tree> const &field_decls, tree field_decl) {
  for (unsigned int i = 0; i < field_decls.length(); i++) {
    if (field_decls[i] == field_decl) {
      return (int)i;
    }
  }
  return -1;
}

// 整合所有函数的分析结果
ArrayDetectErrorCode integrateFunctionResults(
  AD_FUNC_ARGS,
  vec<FunctionAnalysisResult*> const &all_function_results,
  vec<tree> &field_decls,
  vec<FieldAnalysisResult*> &field_results
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  AD_DEBUG_PRINT("Integrating results from %u functions", all_function_results.length());
  
  // 初始化输出向量
  field_decls.create(0);
  field_results.create(0);
  
  // 合并所有函数的字段分析结果
  for (unsigned int i = 0; i < all_function_results.length(); i++) {
    FunctionAnalysisResult* func_result = all_function_results[i];
    if (!func_result || !func_result->field_decls || !func_result->field_results) {
      continue;
    }
    
    if (func_result->field_decls->length() != func_result->field_results->length()) {
      continue;
    }
    
    // 遍历该函数的所有字段结果
    for (unsigned int j = 0; j < func_result->field_decls->length(); j++) {
      tree field_decl = (*func_result->field_decls)[j];
      FieldAnalysisResult* field_result = (*func_result->field_results)[j];
      
      if (!field_result) {
        continue;
      }
      
      // 查找或创建全局字段结果
      int global_index = findFieldIndexInResults(field_decls, field_decl);
      FieldAnalysisResult* global_result = NULL;
      
      if (global_index < 0) {
        // 新字段，创建全局结果
        field_decls.safe_push(field_decl);
        global_result = ggc_alloc<FieldAnalysisResult>();
        memset(global_result, 0, sizeof(FieldAnalysisResult));
        global_result->field_decl = field_decl;
        global_result->write_ops = ggc_alloc<vec<FieldWriteCapture*>>();
        global_result->write_ops->create(0);
        global_result->source_vars = ggc_alloc<vec<tree>>();
        global_result->source_vars->create(0);
        global_result->escape_sites = ggc_alloc<vec<EscapeSite*>>();
        global_result->escape_sites->create(0);
        global_result->sources = ggc_alloc<vec<SourceOperation*>>();
        global_result->sources->create(0);
        
        field_results.safe_push(global_result);
        global_index = (int)(field_decls.length() - 1);
      } else {
        global_result = field_results[global_index];
      }
      
      // 合并写入操作
      if (field_result->write_ops) {
        for (unsigned int k = 0; k < field_result->write_ops->length(); k++) {
          global_result->write_ops->safe_push((*field_result->write_ops)[k]);
        }
      }
      
      // 合并逃逸位置
      if (field_result->escape_sites) {
        for (unsigned int k = 0; k < field_result->escape_sites->length(); k++) {
          global_result->escape_sites->safe_push((*field_result->escape_sites)[k]);
        }
      }
      
      // 合并源操作
      if (field_result->sources) {
        for (unsigned int k = 0; k < field_result->sources->length(); k++) {
          global_result->sources->safe_push((*field_result->sources)[k]);
        }
      }
    }
  }
  
  // 重新判定每个字段（基于全局信息）
  for (unsigned int i = 0; i < field_decls.length(); i++) {
    tree field_decl = field_decls[i];
    FieldAnalysisResult* result = field_results[i];
    
    if (!result->write_ops || result->write_ops->length() == 0) {
      result->is_memory_owner = false;
      result->reason = ggc_strdup("no write operations");
      continue;
    }
    
    if (!result->escape_sites || result->escape_sites->length() > 0) {
      result->is_memory_owner = false;
      result->reason = ggc_strdup("value escapes");
      continue;
    }
    
    if (!result->sources || result->sources->length() == 0) {
      result->is_memory_owner = false;
      result->reason = ggc_strdup("no source operations");
      continue;
    }
    
    // 重新判定
    AD_TRY(determineMemoryOwner(AD_ARGS, field_decl, *result->write_ops, 
                                *result->escape_sites, *result->sources, *result));
  }
  
  AD_DEBUG_PRINT("Integration completed: %u fields analyzed", field_decls.length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
