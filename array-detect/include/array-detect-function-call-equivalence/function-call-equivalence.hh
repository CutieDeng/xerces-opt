#pragma once

#include "gcc-common.hh"
#include "prelude.hh"
#include "field-source-variant.hh"
#include "analysis-data.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// 函数调用等价性判断模块
// ============================================================================
// 模块定位：基于 write-operation-trace 模块的二级信息（FunctionCallSource），
//          判断两个函数调用是否等价
// 
// 核心功能：
//   1. 从 FunctionCallSource 提取函数签名信息
//   2. 比较两个函数调用是否等价（对于虚函数调用，比较接口而非实际函数）
// ============================================================================

// 函数调用签名信息（用于等价性判断）
struct FunctionCallSignature {
  CallType call_type;          // 调用类型
  
  // 函数标识信息
  char const *function_name;   // 函数名（可能为 NULL，如果无法获取）
  
  // 函数签名信息（仅对虚函数调用需要）
  tree return_type;           // 返回类型（tree，GCC 内部管理，可能为 NULL）
  tree arg_types;             // 参数类型列表（tree 链表，GCC 内部管理，可能为 NULL）
  
  // 是否成功提取完整的签名信息
  bool is_complete;           // 是否包含完整的签名信息
};

// 从 FunctionCallSource 提取函数调用签名信息
// 返回值：ArrayDetectErrorCode
// 输入：call_source - FunctionCallSource 结构（来自 write-operation-trace 模块）
// 输出：signature - 函数调用签名信息
ArrayDetectErrorCode extractCallSignatureFromSource (
  AD_FUNC_ARGS,
  FunctionCallSource const &call_source,
  FunctionCallSignature &signature
);

// 比较两个函数调用是否等价
// 返回值：ArrayDetectErrorCode
// 输入：call1, call2 - 两个 FunctionCallSource 结构（来自 write-operation-trace 模块）
// 输出：is_same - 是否等价
// 注意：对于虚函数调用，比较的是接口（函数签名），不要求实际调用的函数一致
ArrayDetectErrorCode areFunctionCallsSame (
  AD_FUNC_ARGS,
  FunctionCallSource const &call1,
  FunctionCallSource const &call2,
  bool &is_same
);

} // namespace array_detector

