#include "prelude.hh"
#include "state.hh"
#include "write-operation-trace.hh"
#include "array-detector.hh"
#include "gcc-ext-util.hh"
#include "field-source-variant.hh"
#include "info-print.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// Pipeline 层：高级封装和协调逻辑
// ============================================================================
// 这些函数负责协调数据流分析和调用 driver 层的具体提取函数
// ============================================================================

// 可选自动缩减平凡 move 操作的分析器
// 追踪值的定义链，跳过简单赋值（平凡 move），直到找到真正的来源
// 输入：value - 要追踪的值（可以是任意 tree，如果是 SSA_NAME 则追踪，否则直接返回）
//       function - 所在函数（用于上下文信息）
//       bb - 所在基本块（用于上下文信息）
// 输出：最终的值（可能是 SSA_NAME、常量、或其他表达式）和对应的语句
// 输出：is_phi - 是否遇到 PHI 节点（因分支导致的来源不明）
// 注意：SSA 形式保证每个变量只有一个定义，不会有循环，因此不需要检查循环和深度
namespace {
ArrayDetectErrorCode reduceTrivialMoves (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree value,
  tree function,
  basic_block bb,
  tree &result_final_value,
  gimple *&result_final_stmt_nullable,
  bool &result_is_phi
) AD_FUNCTION_BEGIN {
  (void)detector;
  (void)function;
  (void)bb;
  
  result_is_phi = false;
  
  // 非 SSA_NAME 直接返回
  if (TREE_CODE (value) != SSA_NAME) {
    result_final_value = value;
    result_final_stmt_nullable = NULL;
    AD_RETURNE (OK);
  }
  
  // 获取定义语句
  // 注意：SSA_NAME_DEF_STMT 可能返回 NULL 的情况：
  // 1. 函数参数：在 SSA 构建的某些阶段，参数可能还没有显式的定义语句
  // 2. 默认定义：某些特殊变量（如 __builtin_unreachable 的结果）可能没有定义语句
  // 3. SSA 构建阶段：在 SSA 构建过程中，某些变量可能暂时没有定义语句
  // 4. 已释放的 SSA：在 SSA 释放阶段，定义语句可能已被清除
  gimple * def_stmt = SSA_NAME_DEF_STMT (value);
  if (!def_stmt) {
    result_final_value = value;
    result_final_stmt_nullable = NULL;
    AD_RETURNE (OK);
  }
  
  enum gimple_code code = gimple_code (def_stmt);
  
  // 处理 PHI 节点：多个来源的合并点（因分支导致），标记为来源不明
  if (code == GIMPLE_PHI) {
    result_final_value = value;
    result_final_stmt_nullable = def_stmt;
    result_is_phi = true;
    AD_RETURNE (OK);
  }
  
  // 处理函数调用：返回值直接赋值给 SSA_NAME
  if (code == GIMPLE_CALL) {
    result_final_value = value;
    result_final_stmt_nullable = def_stmt;
    AD_RETURNE (OK);
  }
  
  // 处理赋值语句
  if (code == GIMPLE_ASSIGN) {
    tree lhs = gimple_assign_lhs (def_stmt);
    tree rhs = gimple_assign_rhs1 (def_stmt);
    enum tree_code rhs_code = gimple_assign_rhs_code (def_stmt);
    
    // 判断是否为平凡赋值（trivial move/copy）
    // RHS 必须是 SSA_NAME
    bool is_trivial = false;
    
    if (TREE_CODE (rhs) == SSA_NAME) {
      // 情形 1: NOP_EXPR（无操作转换）
      if (rhs_code == NOP_EXPR) {
        is_trivial = true;
      }
      // 情形 2: CONVERT_EXPR + 纯位等价转换
      else if (rhs_code == CONVERT_EXPR) {
        // 使用 GCC 的 useless_type_conversion_p 判断是否为纯位等价转换
        if (useless_type_conversion_p (TREE_TYPE (lhs), rhs)) {
          is_trivial = true;
        }
      }
      // 情形 3: VIEW_CONVERT_EXPR（总是纯位等价）
      else if (rhs_code == VIEW_CONVERT_EXPR) {
        is_trivial = true;
      }
      // 情形 4: 相同类型的直接赋值
      else if (TREE_TYPE (lhs) == TREE_TYPE (rhs)) {
        // 检查是否为简单的复制赋值
        if (gimple_assign_copy_p (def_stmt)) {
          is_trivial = true;
        }
      }
    }
    
    // 如果是平凡赋值，继续追踪
    if (is_trivial) {
      basic_block def_bb = gimple_bb (def_stmt);
      AD_ASSERT_GCC_LOGIC (def_bb, "gimple_bb () returned NULL for def_stmt in reduceTrivialMoves: value=%p, def_stmt=%p, gimple_code=%d", 
                          (void*)value, (void*)def_stmt, (int)code);
      AD_TRY (reduceTrivialMoves (detector, AD_ARGS, rhs, function, def_bb, result_final_value, result_final_stmt_nullable, result_is_phi));
      AD_RETURNE (OK);
    }
    
    // 非平凡赋值（类型转换、计算等），找到真正的来源
    result_final_value = rhs;
    result_final_stmt_nullable = def_stmt;
    AD_RETURNE (OK);
  }
  
  // 其他类型的语句，返回当前值
  result_final_value = value;
  result_final_stmt_nullable = def_stmt;
  AD_RETURNE (OK);
} AD_FUNCTION_END
}

// 从 RHS 表达式提取来源信息
ArrayDetectErrorCode extractSourceFromRhs (
  ArrayDetector &detector,
  AD_FUNC_ARGS,
  tree rhs,
  gimple *stmt,
  location_t location,
  tree function,
  basic_block bb,
  FieldSourceInfo *&result
) AD_FUNCTION_BEGIN {
  // 第一步：可选自动缩减平凡 move 操作（放在数据流主路上）
  tree final_value;
  gimple *final_stmt_nullable;
  bool is_phi = false;
  
  AD_TRY (reduceTrivialMoves (detector, AD_ARGS, rhs, function, bb, final_value, final_stmt_nullable, is_phi));
  
  // 如果遇到 PHI 节点，提取 PHI 来源信息
  if (is_phi) {
    AD_TRY (extractSourceFromPhi (detector, AD_ARGS, final_stmt_nullable, final_value, location, function, bb, result));
    AD_RETURNE (OK);
  }
  
  // 第二步：根据最终值的类型提取来源信息
  if (TREE_CODE (final_value) == SSA_NAME) {
    // 仍然是 SSA_NAME，检查最终语句
    if (final_stmt_nullable && gimple_code (final_stmt_nullable) == GIMPLE_CALL) {
      // 来自函数调用（GIMPLE_CALL 可以返回值写入 SSA_NAME）
      // 获取调用语句所在的基本块
      // 在正常的 GIMPLE 流程中，每个语句都应该属于某个基本块
      basic_block call_bb = gimple_bb (final_stmt_nullable);
      AD_ASSERT_GCC_LOGIC (call_bb, "gimple_bb () returned NULL for call_stmt in extractSourceFromRhs: final_stmt=%p, final_value=%p, gimple_code=%d",
                          (void*)final_stmt_nullable, (void*)final_value, (int)gimple_code (final_stmt_nullable));
      AD_TRY (extractSourceFromCall (detector, AD_ARGS, final_stmt_nullable, function, call_bb, result));
    } else {
      // 来自变量（可能是参数或其他）
      AD_TRY (extractSourceFromVariable (detector, AD_ARGS, final_value, location, function, bb, result));
    }
    AD_RETURNE (OK);
  } else if (CONSTANT_CLASS_P (final_value)) {
    // 常量
    AD_TRY (extractSourceFromConstant (detector, AD_ARGS, final_value, function, bb, result));
    AD_RETURNE (OK);
  } else {
    // 检查是否是字段访问（COMPONENT_REF 或 MEM_REF）
    enum tree_code final_code = TREE_CODE (final_value);
    if (final_code == COMPONENT_REF || final_code == MEM_REF) {
      // 字段访问（如 b.ptr 或 ptr->field）
      AD_TRY (extractSourceFromFieldAccess (detector, AD_ARGS, final_value, final_stmt_nullable, stmt, location, function, bb, result));
      AD_RETURNE (OK);
    } else {
      // 其他计算表达式
      AD_TRY (extractSourceFromComputation (detector, AD_ARGS, final_value, final_stmt_nullable, stmt, location, function, bb, result));
      AD_RETURNE (OK);
    }
  }
} AD_FUNCTION_END

// 追踪字段赋值：分析字段赋值来源
ArrayDetectErrorCode traceFieldAssignments (ArrayDetector &detector, AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Tracing field assignments");
  
  // 直接遍历 hash_map，使用迭代器
  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;
  size_t processed_count = 0;
  size_t source_extracted_count = 0;
  
  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    // iter->first 是键（TypeFieldKey），iter->second 是值（TypeFieldWriteOps*）
    // 注意：虽然插入时不会插入 NULL 值，但 GCC 的 hash_map 可能允许 NULL 值，
    // 且 findOrCreateTypeFieldWriteOps 中已考虑了"键存在但值为 NULL"的情况。
    // 因此这里需要防御性检查，避免访问空指针。
    TypeFieldWriteOps *tfwo_nullable = (*iter).second;
    if (!tfwo_nullable || !tfwo_nullable->write_ops) {
      continue;
    }
    
    // 遍历该 type -> field 的所有写入操作
    for (unsigned int j = 0; j < tfwo_nullable->write_ops->length (); ++j) {
      FieldWriteCapture *capture_nullable = (*tfwo_nullable->write_ops)[j];
      if (!capture_nullable) {
        continue;
      }
      FieldWriteCapture &capture = *capture_nullable;
      
      // 提取来源信息
      processed_count++;
      
      tree rhs = capture.rhs;
      gimple * stmt = capture.stmt;
      location_t location = capture.location;
      tree function = capture.function_decl;
      basic_block bb = capture.bb;
      
      FieldSourceInfo *source_info;
      AD_TRY (extractSourceFromRhs (
        detector, AD_ARGS, rhs, stmt, location, function, bb, source_info));
      
      // 将来源信息存储到 FieldWriteCapture 的 aux 字段中
      capture.aux = source_info;
      source_extracted_count++;
      
      TypeFieldKey key = (*iter).first;
      
      // 调用调试模块的函数输出详细信息
      AD_TRY (printFieldWriteSourceInfo (AD_ARGS, key.type, key.field_decl, capture, source_info));
    }
  }
  
  AD_DEBUG_PRINT ("Tracing complete: %zu write operations processed, %zu sources extracted",
                processed_count, source_extracted_count);
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector
