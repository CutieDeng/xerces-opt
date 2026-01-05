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
  gimple * call_stmt,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
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
    AD_DEBUG_PRINT ("[extractSourceFromCall] gimple_call_fn tree code: %s", 
                    fn ? get_tree_code_name (TREE_CODE (fn)) : "<null>");
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
      } else if (TREE_CODE (fn) == SSA_NAME) {
        // 间接调用：尝试追踪 SSA_NAME 的定义来找到函数名
        AD_DEBUG_PRINT ("[extractSourceFromCall] fn is SSA_NAME, tracing definition");
        gimple * def_stmt = SSA_NAME_DEF_STMT (fn);
        AD_DEBUG_PRINT ("[extractSourceFromCall] def_stmt: %p, gimple_code: %d", 
                        (void*)def_stmt, def_stmt ? (int)gimple_code (def_stmt) : -1);
        if (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN) {
          tree rhs = gimple_assign_rhs1 (def_stmt);
          enum tree_code rhs_code = gimple_assign_rhs_code (def_stmt);
          AD_DEBUG_PRINT ("[extractSourceFromCall] rhs tree code: %s, rhs_code: %s",
                          rhs ? get_tree_code_name (TREE_CODE (rhs)) : "<null>",
                          get_tree_code_name (rhs_code));
          
          // 首先检查是否是 OBJ_TYPE_REF（虚函数调用）
          if (TREE_CODE (rhs) == OBJ_TYPE_REF) {
            AD_DEBUG_PRINT ("[extractSourceFromCall] Found OBJ_TYPE_REF in SSA_NAME definition!");
            call_source.call_type = CALL_VIRTUAL;  // 直接设置为虚函数调用
            tree method = OBJ_TYPE_REF_EXPR (rhs);
            AD_DEBUG_PRINT ("[extractSourceFromCall] OBJ_TYPE_REF_EXPR: %p, method tree code: %s",
                            (void*)method, method ? get_tree_code_name (TREE_CODE (method)) : "<null>");
            
            // OBJ_TYPE_REF_EXPR 可能返回 SSA_NAME 而不是直接的 FUNCTION_DECL
            // 需要追踪 SSA_NAME 的定义
            tree method_decl = method;
            if (method && TREE_CODE (method) == SSA_NAME) {
              AD_DEBUG_PRINT ("[extractSourceFromCall] method is SSA_NAME, tracing definition");
              gimple * method_def = SSA_NAME_DEF_STMT (method);
              if (method_def && gimple_code (method_def) == GIMPLE_ASSIGN) {
                tree method_rhs = gimple_assign_rhs1 (method_def);
                AD_DEBUG_PRINT ("[extractSourceFromCall] method_def rhs tree code: %s",
                                method_rhs ? get_tree_code_name (TREE_CODE (method_rhs)) : "<null>");
                if (method_rhs && TREE_CODE (method_rhs) == FUNCTION_DECL) {
                  method_decl = method_rhs;
                  AD_DEBUG_PRINT ("[extractSourceFromCall] Found FUNCTION_DECL in SSA_NAME definition");
                } else if (method_rhs && TREE_CODE (method_rhs) == ADDR_EXPR) {
                  tree addr_operand = TREE_OPERAND (method_rhs, 0);
                  if (addr_operand && TREE_CODE (addr_operand) == FUNCTION_DECL) {
                    method_decl = addr_operand;
                    AD_DEBUG_PRINT ("[extractSourceFromCall] Found FUNCTION_DECL in ADDR_EXPR");
                  }
                }
              }
            }
            
            if (method_decl && TREE_CODE (method_decl) == FUNCTION_DECL && DECL_NAME (method_decl)) {
              char const * method_name = IDENTIFIER_POINTER (DECL_NAME (method_decl));
              AD_DEBUG_PRINT ("[extractSourceFromCall] Extracted virtual function name: %s", method_name);
              call_source.function_name = ggc_strdup (method_name);
            } else {
              AD_DEBUG_PRINT ("[extractSourceFromCall] Failed to extract method name from OBJ_TYPE_REF, method_decl tree code: %s",
                              method_decl ? get_tree_code_name (TREE_CODE (method_decl)) : "<null>");
              call_source.function_name = ggc_strdup ("<virtual-call>");
            }
            // 跳过后续的 call_type 分析，因为已经确定是虚函数调用
            AD_RETURNO (info);
          }
          // 检查是否是 ADDR_EXPR（函数地址）
          else if (rhs_code == ADDR_EXPR) {
            tree addr_expr = TREE_OPERAND (rhs, 0);
            if (addr_expr && TREE_CODE (addr_expr) == FUNCTION_DECL && DECL_NAME (addr_expr)) {
              call_source.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (addr_expr)));
            } else {
              call_source.function_name = ggc_strdup ("<indirect-call>");
            }
          } else if (TREE_CODE (rhs) == FUNCTION_DECL && DECL_NAME (rhs)) {
            // 直接赋值函数声明
            call_source.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (rhs)));
          } else {
            call_source.function_name = ggc_strdup ("<indirect-call>");
          }
        } else {
          call_source.function_name = ggc_strdup ("<indirect-call>");
        }
      } else if (TREE_CODE (fn) == ADDR_EXPR) {
        // 函数地址表达式
        tree addr_expr = TREE_OPERAND (fn, 0);
        if (addr_expr && TREE_CODE (addr_expr) == FUNCTION_DECL && DECL_NAME (addr_expr)) {
          call_source.function_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (addr_expr)));
        } else {
          call_source.function_name = ggc_strdup ("<indirect-call>");
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

// 从无法追踪的来源提取信息（原 extractSourceFromVariable）
// 注意：SOURCE_VARIABLE 已被移除，因为它语义模糊（可能是函数参数或追踪失败）
// 现在统一使用 SOURCE_UNKNOWN 表示无法确定的来源
ArrayDetectErrorCode extractSourceFromUnknown (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  (void)ssa_name;
  (void)location;

  // 分配来源信息结构
  FieldSourceInfo * info = ggc_alloc<FieldSourceInfo> ();
  if (!info) {
    AD_RETURNE (MEMORY_ERROR);
  }
  memset (info, 0, sizeof (FieldSourceInfo));

  // 使用 SOURCE_UNKNOWN 表示无法追踪到明确来源
  // 可能的情况：
  // 1. 函数参数（SSA_NAME 没有定义语句）
  // 2. 追踪深度达到限制
  // 3. 复杂的控制流导致无法追踪
  info->source_type = SOURCE_UNKNOWN;

  AD_RETURNO (info);
} AD_FUNCTION_END

// 从常量提取来源信息
ArrayDetectErrorCode extractSourceFromConstant (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree constant_value,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
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
  gimple * phi_stmt,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
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

// 从字段访问提取来源信息
ArrayDetectErrorCode extractSourceFromFieldAccess (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree access_expr,
  gimple * access_stmt_nullable,
  gimple * fallback_stmt,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
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

  info->source_type = SOURCE_FIELD_ACCESS;
  LET_SOURCE_FIELD_ACCESS (field_access, *info)
    // 初始化字段访问来源信息
    field_access.access_stmt = access_stmt_nullable ? access_stmt_nullable : fallback_stmt;
    field_access.access_expr = access_expr;
    field_access.location = location;

    // 提取字段信息和基对象
    tree field_decl = NULL_TREE;
    tree base_object = NULL_TREE;
    tree object_type = NULL_TREE;

    enum tree_code code = TREE_CODE (access_expr);

    if (code == COMPONENT_REF) {
      // COMPONENT_REF: 直接的结构体字段访问（如 obj.field）
      field_decl = TREE_OPERAND (access_expr, 1);  // 字段声明
      base_object = TREE_OPERAND (access_expr, 0);  // 基对象

      // 获取对象类型
      if (base_object) {
        object_type = TREE_TYPE (base_object);
        // 如果是指针类型，需要解引用
        if (object_type && TREE_CODE (object_type) == POINTER_TYPE) {
          object_type = TREE_TYPE (object_type);
        }
      }
    } else if (code == MEM_REF) {
      // MEM_REF: 通过指针访问（如 ptr->field）
      base_object = TREE_OPERAND (access_expr, 0);  // 基对象（通常是指针）

      // 获取类型信息
      tree mem_type = TREE_TYPE (access_expr);
      if (mem_type) {
        // 尝试从类型中提取字段信息
        // MEM_REF 可能没有直接的 field_decl，需要通过偏移量推断
        // 这里我们获取访问的类型，后续可以根据偏移量匹配字段

        // 获取基对象的类型
        if (base_object) {
          tree base_type = TREE_TYPE (base_object);
          if (base_type && TREE_CODE (base_type) == POINTER_TYPE) {
            object_type = TREE_TYPE (base_type);
          }
        }

        // 尝试从 MEM_REF 的偏移量中推断字段
        // 注意：这是一个简化的实现，完整的字段推断需要更复杂的分析
        tree offset = TREE_OPERAND (access_expr, 1);
        if (object_type && TREE_CODE (object_type) == RECORD_TYPE && offset) {
          // 遍历类型的字段，尝试匹配偏移量
          for (tree field = TYPE_FIELDS (object_type); field; field = DECL_CHAIN (field)) {
            if (TREE_CODE (field) == FIELD_DECL) {
              tree field_offset = DECL_FIELD_OFFSET (field);
              tree field_bit_offset = DECL_FIELD_BIT_OFFSET (field);

              // 简化匹配：如果偏移量为0且这是第一个字段，或者需要更精确的匹配
              // 这里我们简单地取第一个匹配的字段作为示例
              if (field_decl == NULL_TREE) {
                // TODO: 实现精确的偏移量匹配
                // 目前简单地将字段设为未知，或者可以尝试匹配
                (void)field_offset;
                (void)field_bit_offset;
                // field_decl = field;  // 暂时不设置，因为偏移量匹配不精确
                break;
              }
            }
          }
        }
      }
    }

    // 设置字段信息
    field_access.field_decl = field_decl;
    if (field_decl && DECL_NAME (field_decl)) {
      field_access.field_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (field_decl)));
    } else {
      field_access.field_name = ggc_strdup ("<unknown-field>");
    }

    // 设置对象类型信息
    field_access.object_type = object_type;
    if (object_type && TYPE_NAME (object_type)) {
      tree type_name_tree = TYPE_NAME (object_type);
      if (TREE_CODE (type_name_tree) == TYPE_DECL && DECL_NAME (type_name_tree)) {
        field_access.type_name = ggc_strdup (IDENTIFIER_POINTER (DECL_NAME (type_name_tree)));
      } else if (TREE_CODE (type_name_tree) == IDENTIFIER_NODE) {
        field_access.type_name = ggc_strdup (IDENTIFIER_POINTER (type_name_tree));
      } else {
        field_access.type_name = ggc_strdup ("<unnamed-type>");
      }
    } else {
      field_access.type_name = ggc_strdup ("<unknown-type>");
    }

    // 设置基对象
    field_access.base_object = base_object;

    AD_DEBUG_PRINT ("[extractSourceFromFieldAccess] Extracted field access: type=%s, field=%s, tree_code=%s",
                    field_access.type_name, field_access.field_name, get_tree_code_name (code));

    AD_RETURNO (info);
  END_LET ()
} AD_FUNCTION_END

// 从计算表达式提取来源信息
ArrayDetectErrorCode extractSourceFromComputation (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree compute_expr,
  gimple * compute_stmt_nullable,
  gimple * fallback_stmt,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo * &result
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

