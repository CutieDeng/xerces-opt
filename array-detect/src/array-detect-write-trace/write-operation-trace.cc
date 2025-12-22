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
  tree function,
  basic_block bb,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
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
  const char* signature;
  AD_TRY(array_detect_ns::extractCallSignature(AD_ARGS, call_stmt, signature));
  if (!signature) {
    AD_RETURNE(GCC_LOGIC_ERROR);
  }
  call_source->signature = ggc_strdup(signature);
  
  *out_source_info = source_info;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 从变量提取来源信息
ArrayDetectErrorCode extractSourceFromVariable(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree ssa_name,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
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
  tree var_decl_nullable = SSA_NAME_VAR(ssa_name);
  if (var_decl_nullable) {
    var_source->var_decl = var_decl_nullable;
    if (DECL_NAME(var_decl_nullable)) {
      var_source->var_name = ggc_strdup(IDENTIFIER_POINTER(DECL_NAME(var_decl_nullable)));
    }
  }
  
  *out_source_info = source_info;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 可选自动缩减平凡 move 操作的分析器
// 追踪值的定义链，跳过简单赋值（平凡 move），直到找到真正的来源
// 输入：value - 要追踪的值（可以是任意 tree，如果是 SSA_NAME 则追踪，否则直接返回）
//       function - 所在函数（用于上下文信息）
//       bb - 所在基本块（用于上下文信息）
// 输出：最终的值（可能是 SSA_NAME、常量、或其他表达式）和对应的语句
// 输出：is_phi - 是否遇到 PHI 节点（因分支导致的来源不明）
// 注意：SSA 形式保证每个变量只有一个定义，不会有循环，因此不需要检查循环和深度
namespace {
ArrayDetectErrorCode reduceTrivialMoves(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree value,
  tree function,
  basic_block bb,
  tree &out_final_value,
  gimple* &out_final_stmt_nullable,
  bool &out_is_phi
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  out_is_phi = false;
  
  // 非 SSA_NAME 直接返回
  if (TREE_CODE(value) != SSA_NAME) {
    out_final_value = value;
    out_final_stmt_nullable = NULL;
    AD_RETURNE(OK);
  }
  
  // 获取定义语句
  // 注意：SSA_NAME_DEF_STMT 可能返回 NULL 的情况：
  // 1. 函数参数：在 SSA 构建的某些阶段，参数可能还没有显式的定义语句
  // 2. 默认定义：某些特殊变量（如 __builtin_unreachable 的结果）可能没有定义语句
  // 3. SSA 构建阶段：在 SSA 构建过程中，某些变量可能暂时没有定义语句
  // 4. 已释放的 SSA：在 SSA 释放阶段，定义语句可能已被清除
  gimple* def_stmt = SSA_NAME_DEF_STMT(value);
  if (!def_stmt) {
    out_final_value = value;
    out_final_stmt_nullable = NULL;
    AD_RETURNE(OK);
  }
  
  enum gimple_code code = gimple_code(def_stmt);
  
  // 处理 PHI 节点：多个来源的合并点（因分支导致），标记为来源不明
  if (code == GIMPLE_PHI) {
    out_final_value = value;
    out_final_stmt_nullable = def_stmt;
    out_is_phi = true;
    AD_RETURNE(OK);
  }
  
  // 处理函数调用：返回值直接赋值给 SSA_NAME
  if (code == GIMPLE_CALL) {
    out_final_value = value;
    out_final_stmt_nullable = def_stmt;
    AD_RETURNE(OK);
  }
  
  // 处理赋值语句
  if (code == GIMPLE_ASSIGN) {
    tree rhs = gimple_assign_rhs1(def_stmt);
    enum tree_code rhs_code = gimple_assign_rhs_code(def_stmt);
    
    // 平凡 move：RHS 是 SSA_NAME 且操作码是 NOP_EXPR（无操作），继续追踪
    if (TREE_CODE(rhs) == SSA_NAME && rhs_code == NOP_EXPR) {
      basic_block def_bb = gimple_bb(def_stmt);
      if (!def_bb) {
        AD_DEBUG_PRINT("Error: gimple_bb() returned NULL for def_stmt in reduceTrivialMoves");
        AD_DEBUG_PRINT("  value: %p, def_stmt: %p, gimple_code: %d", (void*)value, (void*)def_stmt, (int)code);
        AD_RETURNE(GCC_LOGIC_ERROR);
      }
      AD_TRY(reduceTrivialMoves(detector, AD_ARGS, rhs, function, def_bb, out_final_value, out_final_stmt_nullable, out_is_phi));
      AD_RETURNE(OK);
    }
    
    // 非平凡赋值（类型转换、计算等），找到真正的来源
    out_final_value = rhs;
    out_final_stmt_nullable = def_stmt;
    AD_RETURNE(OK);
  }
  
  // 其他类型的语句，返回当前值
  out_final_value = value;
  out_final_stmt_nullable = def_stmt;
  AD_RETURNE(OK);
} AD_FUNCTION_END
}

// 从 RHS 表达式提取来源信息
ArrayDetectErrorCode extractSourceFromRhs(
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree rhs,
  gimple* stmt,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo** out_source_info
) AD_FUNCTION_BEGIN {
  if (!rhs || !stmt || !out_source_info) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 第一步：可选自动缩减平凡 move 操作（放在数据流主路上）
  tree final_value;
  gimple* final_stmt_nullable;
  bool is_phi = false;
  
  AD_TRY(reduceTrivialMoves(detector, AD_ARGS, rhs, function, bb, final_value, final_stmt_nullable, is_phi));
  
  if (!final_value) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  // 如果遇到 PHI 节点，抛出错误
  if (is_phi) {
    AD_RETURNE(GCC_LOGIC_ERROR);
  }
  
  // 第二步：根据最终值的类型提取来源信息
  if (TREE_CODE(final_value) == SSA_NAME) {
    // 仍然是 SSA_NAME，检查最终语句
    if (final_stmt_nullable && gimple_code(final_stmt_nullable) == GIMPLE_CALL) {
      // 来自函数调用（GIMPLE_CALL 可以返回值写入 SSA_NAME）
      // 获取调用语句所在的基本块
      // 在正常的 GIMPLE 流程中，每个语句都应该属于某个基本块
      basic_block call_bb = gimple_bb(final_stmt_nullable);
      if (!call_bb) {
        AD_DEBUG_PRINT("Error: gimple_bb() returned NULL for call_stmt in extractSourceFromRhs");
        AD_DEBUG_PRINT("  final_stmt: %p", (void*)final_stmt_nullable);
        AD_DEBUG_PRINT("  final_value: %p", (void*)final_value);
        AD_DEBUG_PRINT("  gimple_code: %d", (int)gimple_code(final_stmt_nullable));
        AD_RETURNE(GCC_LOGIC_ERROR);
      }
      AD_TRY(extractSourceFromCall(detector, AD_ARGS, final_stmt_nullable, final_value, function, call_bb, out_source_info));
    } else {
      // 来自变量（可能是参数或其他）
      AD_TRY(extractSourceFromVariable(detector, AD_ARGS, final_value, location, function, bb, out_source_info));
    }
  } else if (CONSTANT_CLASS_P(final_value)) {
    // 常量
    FieldSourceInfo* source_info = ggc_alloc<FieldSourceInfo>();
    if (!source_info) {
      AD_RETURNE(MEMORY_ERROR);
    }
    memset(source_info, 0, sizeof(FieldSourceInfo));
    
    source_info->source_type = SOURCE_CONSTANT;
    ConstantSource* const_source = &source_info->data.constant;
    const_source->constant_value = final_value;
    
    // 尝试获取常量字符串表示
    if (TREE_CODE(final_value) == INTEGER_CST) {
      if (ctx.address_format_buffer && ctx.address_format_buffer_size > 0) {
        snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size, 
                 "%lld", (long long)TREE_INT_CST_LOW(final_value));
        const_source->constant_str = ggc_strdup(ctx.address_format_buffer);
      }
    } else if (TREE_CODE(final_value) == STRING_CST) {
      const_source->constant_str = ggc_strdup(TREE_STRING_POINTER(final_value));
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
    comp_source->compute_stmt = final_stmt_nullable ? final_stmt_nullable : stmt;
    comp_source->compute_expr = final_value;
    comp_source->location = location;
    comp_source->description = ggc_strdup("<computation>");
    
    *out_source_info = source_info;
    AD_RETURNE(OK);
  }
} AD_FUNCTION_END

// 追踪字段赋值：分析字段赋值来源
ArrayDetectErrorCode traceFieldAssignments(ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT("Tracing field assignments");
  
  // 检查 hash_map 是否已初始化
  if (!detector.m_type_field_writes) {
    AD_RETURNE(NOT_INITIALIZED);
  }
  
  // 直接遍历 hash_map，使用迭代器
  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;
  size_t processed_count = 0;
  size_t source_extracted_count = 0;
  
  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin();
       iter != detector.m_type_field_writes->end();
       ++iter) {
    // iter->first 是键（TypeFieldKey），iter->second 是值（TypeFieldWriteOps*）
    TypeFieldWriteOps* tfwo_nullable = (*iter).second;
    if (!tfwo_nullable || !tfwo_nullable->write_ops) {
      continue;
    }
    
    // 遍历该 type -> field 的所有写入操作
    for (unsigned int j = 0; j < tfwo_nullable->write_ops->length(); ++j) {
      FieldWriteCapture* capture_nullable = (*tfwo_nullable->write_ops)[j];
      if (!capture_nullable) {
        continue;
      }
      FieldWriteCapture &capture = *capture_nullable;
      
      // 提取来源信息
      processed_count++;
      
      tree rhs = capture.rhs;
      gimple* stmt = capture.stmt;
      location_t location = capture.location;
      tree function = capture.function_decl;
      basic_block bb = capture.bb;
      
      FieldSourceInfo* source_info;
      AD_TRY(extractSourceFromRhs(
        detector, AD_ARGS, rhs, stmt, location, function, bb, &source_info));
      
      // 将来源信息存储到 FieldWriteCapture 的 aux 字段中
      capture.aux = source_info;
      source_extracted_count++;
      
      TypeFieldKey key = (*iter).first;
      AD_DEBUG_PRINT("Extracted source for field write: type=%p, field=%p, source_type=%d",
                    (void*)key.type, (void*)key.field_decl, source_info->source_type);
    }
  }
  
  AD_DEBUG_PRINT("Tracing complete: %zu write operations processed, %zu sources extracted",
                processed_count, source_extracted_count);
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detector
