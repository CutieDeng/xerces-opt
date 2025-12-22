#include "prelude.hh"
#include "state.hh"
#include "write-operation-trace.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "virtual-call-analysis.hh"
#include "field-source-variant.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// 从函数调用提取来源信息
ArrayDetectErrorCode extractSourceFromCall(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  gimple* call_stmt,
  tree return_ssa,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  (void)detector;
  
  if (!call_stmt || !out_source_info) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 分配来源信息结构
  FieldSourceInfo* source_info = ggc_alloc<FieldSourceInfo>();
  if (!source_info) {
    AD_RETURNE(MEMORY_ERROR);
  }
  memset(source_info, 0, sizeof(FieldSourceInfo));
  
  source_info->source_type = SOURCE_FUNCTION_CALL;
  
  // 初始化函数调用来源信息
  FunctionCallSource* call_source = &source_info->data.function_call;
  call_source->call_stmt = call_stmt;
  call_source->return_value_ssa = return_ssa;
  call_source->location = gimple_location(call_stmt);
  
  // 获取函数名
  tree fn = gimple_call_fn(call_stmt);
  if (fn) {
    if (TREE_CODE(fn) == FUNCTION_DECL) {
      if (DECL_NAME(fn)) {
        call_source->function_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(fn)));
      } else {
        call_source->function_name = ggc_strdup("<unnamed-function>");
      }
    } else if (TREE_CODE(fn) == OBJ_TYPE_REF) {
      // 虚函数调用
      tree method = OBJ_TYPE_REF_EXPR(fn);
      if (method && TREE_CODE(method) == FUNCTION_DECL && DECL_NAME(method)) {
        call_source->function_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(method)));
      } else {
        call_source->function_name = ggc_strdup("<virtual-call>");
      }
    } else {
      call_source->function_name = ggc_strdup("<indirect-call>");
    }
  } else {
    call_source->function_name = ggc_strdup("<unknown-call>");
  }
  
  // 分析调用类型
  bool is_virtual = false;
  CallType call_type = CALL_UNKNOWN;
  if (array_detect_ns::isVirtualFunctionCall(AD_ARGS, call_stmt, is_virtual, call_type) == OK && is_virtual) {
    call_source->call_type = CALL_VIRTUAL;
  } else if (fn && TREE_CODE(fn) == FUNCTION_DECL) {
    call_source->call_type = CALL_DIRECT;
  } else {
    call_source->call_type = CALL_INDIRECT;
  }
  
  // 提取调用签名
  const char* signature = NULL;
  if (array_detect_ns::extractCallSignature(AD_ARGS, call_stmt, signature) == OK && signature) {
    call_source->signature = ggc_strdup(signature);
  } else {
    call_source->signature = NULL;
  }
  
  *out_source_info = source_info;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 从变量提取来源信息
ArrayDetectErrorCode extractSourceFromVariable(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree ssa_name,
  location_t location,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  (void)detector;
  
  if (!ssa_name || TREE_CODE(ssa_name) != SSA_NAME || !out_source_info) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 分配来源信息结构
  FieldSourceInfo* source_info = ggc_alloc<FieldSourceInfo>();
  if (!source_info) {
    AD_RETURNE(MEMORY_ERROR);
  }
  memset(source_info, 0, sizeof(FieldSourceInfo));
  
  source_info->source_type = SOURCE_VARIABLE;
  
  // 初始化变量来源信息
  VariableSource* var_source = &source_info->data.variable;
  var_source->ssa_name = ssa_name;
  var_source->location = location;
  
  // 获取变量声明
  tree var_decl = SSA_NAME_VAR(ssa_name);
  if (var_decl) {
    var_source->var_decl = var_decl;
    if (DECL_NAME(var_decl)) {
      var_source->var_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(var_decl)));
    }
  }
  
  *out_source_info = source_info;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 从 RHS 表达式提取来源信息
ArrayDetectErrorCode extractSourceFromRhs(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree rhs,
  gimple* stmt,
  location_t location,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  if (!rhs || !stmt || !out_source_info) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 检查是否是函数调用
  if (TREE_CODE(rhs) == SSA_NAME) {
    // 检查 SSA_NAME 的定义语句
    gimple* def_stmt = SSA_NAME_DEF_STMT(rhs);
    if (def_stmt && gimple_code(def_stmt) == GIMPLE_CALL) {
      // 来自函数调用
      return extractSourceFromCall(detector, AD_ARGS, def_stmt, rhs, out_source_info);
    } else {
      // 来自变量
      return extractSourceFromVariable(detector, AD_ARGS, rhs, location, out_source_info);
    }
  } else if (CONSTANT_CLASS_P(rhs)) {
    // 常量
    FieldSourceInfo* source_info = ggc_alloc<FieldSourceInfo>();
    if (!source_info) {
      AD_RETURNE(MEMORY_ERROR);
    }
    memset(source_info, 0, sizeof(FieldSourceInfo));
    
    source_info->source_type = SOURCE_CONSTANT;
    ConstantSource* const_source = &source_info->data.constant;
    const_source->constant_value = rhs;
    
    // 尝试获取常量字符串表示
    if (TREE_CODE(rhs) == INTEGER_CST) {
      // 整数常量
      if (ctx.address_format_buffer && ctx.address_format_buffer_size > 0) {
        snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size, 
                 "%lld", (long long)TREE_INT_CST_LOW(rhs));
        const_source->constant_str = ggc_strdup(ctx.address_format_buffer);
      }
    } else if (TREE_CODE(rhs) == STRING_CST) {
      // 字符串常量
      const_source->constant_str = ggc_strdup(TREE_STRING_POINTER(rhs));
    } else {
      const_source->constant_str = ggc_strdup("<constant>");
    }
    
    *out_source_info = source_info;
    AD_RETURNE(OK);
  } else {
    // 计算表达式
    FieldSourceInfo* source_info = ggc_alloc<FieldSourceInfo>();
    if (!source_info) {
      AD_RETURNE(MEMORY_ERROR);
    }
    memset(source_info, 0, sizeof(FieldSourceInfo));
    
    source_info->source_type = SOURCE_COMPUTATION;
    ComputationSource* comp_source = &source_info->data.computation;
    comp_source->compute_stmt = stmt;
    comp_source->compute_expr = rhs;
    comp_source->location = location;
    comp_source->description = ggc_strdup("<computation>");
    
    *out_source_info = source_info;
    AD_RETURNE(OK);
  }
} AD_FUNCTION_END

// 追踪字段赋值：分析字段赋值来源
ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT("Tracing field assignments");
  
  // 检查 hash_map 和键列表是否已初始化
  if (!detector.m_type_field_writes || !detector.m_type_field_keys) {
    AD_DEBUG_PRINT("Warning: m_type_field_writes or m_type_field_keys not initialized");
    AD_RETURNE(OK);
  }
  
  // 直接遍历键列表，然后从 hash_map 中获取对应的写入操作列表
  // 这样避免了遍历所有函数，效率更高
  // 使用 hash_set 去重，避免重复处理同一个键（虽然理论上不应该重复，但为了安全）
  hash_set<TypeFieldKey> processed_keys;
  processed_keys.create_ggc(0);
  
  size_t processed_count = 0;
  size_t source_extracted_count = 0;
  
  for (unsigned int i = 0; i < detector.m_type_field_keys->length(); ++i) {
    TypeFieldKey key = (*detector.m_type_field_keys)[i];
    
    // 检查是否已处理过该键（去重）
    if (processed_keys.contains(key)) {
      continue;
    }
    processed_keys.add(key);
    
    // 从 hash_map 中获取对应的写入操作列表
    TypeFieldWriteOps** tfwo_ptr = detector.m_type_field_writes->get(key);
    if (!tfwo_ptr || !*tfwo_ptr) {
      continue;
    }
    
    TypeFieldWriteOps* tfwo = *tfwo_ptr;
    if (!tfwo->write_ops) {
      continue;
    }
    
    // 遍历该 type -> field 的所有写入操作
    for (unsigned int j = 0; j < tfwo->write_ops->length(); ++j) {
      FieldWriteCapture* capture = (*tfwo->write_ops)[j];
      if (!capture) {
        continue;
      }
      
      // 提取来源信息
      processed_count++;
      
      tree rhs = capture->rhs;
      gimple* stmt = capture->stmt;
      location_t location = capture->location;
      
      FieldSourceInfo* source_info = NULL;
      ArrayDetectErrorCode extract_result = extractSourceFromRhs(
        detector, AD_ARGS, rhs, stmt, location, &source_info);
      
      if (extract_result == OK && source_info) {
        // 将来源信息存储到 FieldWriteCapture 的 next 字段中
        capture->next = source_info;
        source_extracted_count++;
        
        AD_DEBUG_PRINT("Extracted source for field write: type=%p, field=%p, source_type=%d",
                      (void*)key.type, (void*)key.field_decl, source_info->source_type);
      }
    }
  }
  
  AD_DEBUG_PRINT("Tracing complete: %zu write operations processed, %zu sources extracted",
                processed_count, source_extracted_count);
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detector
