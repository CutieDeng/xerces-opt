#include "prelude.hh"
#include "state.hh"
#include "write-operation-trace.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "virtual-call-analysis.hh"
#include "field-source-variant.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// Driver 层：具体来源提取函数实现
// ============================================================================
// 这些函数负责从特定类型的来源（函数调用、变量、常量、PHI、计算）提取信息
// ============================================================================

// 从函数调用提取来源信息
ArrayDetectErrorCode extractSourceFromCall (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  gimple* call_stmt,
  tree function,
  basic_block bb,
  FieldSourceInfo* &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  // 分配来源信息结构
  FieldSourceInfo *info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));

  info->source_type = SOURCE_FUNCTION_CALL;
  LET_SOURCE_FUNCTION_CALL (call_source, *info)
    // 初始化函数调用来源信息
    call_source.call_stmt = call_stmt;
    call_source.location = gimple_location (call_stmt);
    
    // 获取函数名
    tree fn = gimple_call_fn (call_stmt);
    if (fn) {
      if (TREE_CODE (fn) == FUNCTION_DECL) {
        if (DECL_NAME (fn)) {
          call_source.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (fn)));
        } else {
          call_source.function_name = ggc_strdup ("<unnamed-function>");
        }
      } else if (TREE_CODE (fn) == OBJ_TYPE_REF) {
        // 虚函数调用
        tree method = OBJ_TYPE_REF_EXPR (fn);
        if (method && TREE_CODE (method) == FUNCTION_DECL && DECL_NAME (method)) {
          call_source.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (method)));
        } else {
          call_source.function_name = ggc_strdup ("<virtual-call>");
        }
      } else {
        call_source.function_name = ggc_strdup ("<indirect-call>");
      }
    } else {
      call_source.function_name = ggc_strdup ("<unknown-call>");
    }
    
    // 分析调用类型
    bool is_virtual = false;
    CallType call_type = CALL_UNKNOWN;
    if (isVirtualFunctionCall (AD_ARGS, call_stmt, is_virtual, call_type) == OK && is_virtual) {
      call_source.call_type = CALL_VIRTUAL;
    } else if (fn && TREE_CODE (fn) == FUNCTION_DECL) {
      call_source.call_type = CALL_DIRECT;
    } else {
      call_source.call_type = CALL_INDIRECT;
    }
    
    AD_RETURNO (info);
  END_LET ()
} AD_FUNCTION_END

// 从变量提取来源信息
ArrayDetectErrorCode extractSourceFromVariable (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo* &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  // 分配来源信息结构
  FieldSourceInfo* info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));
  
  info->source_type = SOURCE_VARIABLE;
  
  // 初始化变量来源信息
  VariableSource* var_source = &info->data.variable;
  var_source->ssa_name = ssa_name;
  var_source->location = location;
  
  // 获取变量声明
  tree var_decl_nullable = SSA_NAME_VAR (ssa_name);
  if (var_decl_nullable) {
    var_source->var_decl = var_decl_nullable;
    if (DECL_NAME (var_decl_nullable)) {
      var_source->var_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (var_decl_nullable)));
    }
  }
  
  AD_RETURNO (info);
} AD_FUNCTION_END

// 从常量提取来源信息
ArrayDetectErrorCode extractSourceFromConstant (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree constant_value,
  tree function,
  basic_block bb,
  FieldSourceInfo* &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  // 分配来源信息结构
  FieldSourceInfo *info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));
  
  info->source_type = SOURCE_CONSTANT;
  LET_SOURCE_CONSTANT (const_source, *info)
    // 初始化常量来源信息
    const_source.constant_value = constant_value;
    
    // 尝试获取常量字符串表示
    if (TREE_CODE (constant_value) == INTEGER_CST) {
      if (ctx.address_format_buffer && ctx.address_format_buffer_size > 0) {
        snprintf (ctx.address_format_buffer, ctx.address_format_buffer_size, 
                 "%lld", (long long)TREE_INT_CST_LOW (constant_value));
        const_source.constant_str = ggc_strdup (ctx.address_format_buffer);
      }
    } else if (TREE_CODE (constant_value) == STRING_CST) {
      const_source.constant_str = ggc_strdup (TREE_STRING_POINTER (constant_value));
    } else {
      const_source.constant_str = ggc_strdup ("<constant>");
    }
    
    AD_RETURNO (info);
  END_LET ()
} AD_FUNCTION_END

// 从 PHI 节点提取来源信息
ArrayDetectErrorCode extractSourceFromPhi (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  gimple* phi_stmt,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo* &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  // 分配来源信息结构
  FieldSourceInfo *info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));
  
  info->source_type = SOURCE_PHI;
  LET_SOURCE_PHI (phi_source, *info)
    // 初始化 PHI 来源信息
    phi_source.phi_stmt = phi_stmt;
    phi_source.ssa_name = ssa_name;
    phi_source.location = location;
    
    // 获取变量声明和变量名
    tree var_decl_nullable = SSA_NAME_VAR (ssa_name);
    if (var_decl_nullable) {
      phi_source.var_decl = var_decl_nullable;
      if (DECL_NAME (var_decl_nullable)) {
        phi_source.var_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (var_decl_nullable)));
      }
    }
    
    AD_RETURNO (info);
  END_LET ()
} AD_FUNCTION_END

// 从计算表达式提取来源信息
ArrayDetectErrorCode extractSourceFromComputation (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree compute_expr,
  gimple* compute_stmt_nullable,
  gimple* fallback_stmt,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo* &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  // 分配来源信息结构
  FieldSourceInfo *info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));
  
  info->source_type = SOURCE_COMPUTATION;
  LET_SOURCE_COMPUTATION (comp_source, *info)
    // 初始化计算来源信息
    comp_source.compute_stmt = compute_stmt_nullable ? compute_stmt_nullable : fallback_stmt;
    comp_source.compute_expr = compute_expr;
    comp_source.location = location;
    comp_source.description = ggc_strdup ("<computation>");
    
    AD_RETURNO (info);
  END_LET ()
} AD_FUNCTION_END

} // namespace array_detector

