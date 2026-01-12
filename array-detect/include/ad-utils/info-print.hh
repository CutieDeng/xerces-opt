#pragma once

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-write.hh"
#include "write-source.hh"

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printResults (AD_FUNC_ARGS, ::array_detector::ArrayDetector &detector);

// ----------------------------------------------------------------------------
// 调试输出函数：打印 GIMPLE_CALL 语句的详细信息
// ----------------------------------------------------------------------------
// 根据调用类型（VIRTUAL, DIRECT, INDIRECT）输出不同的调试信息
// 用于复杂调试场景
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printGimpleCallDetails (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  FILE * output_file
);

// ----------------------------------------------------------------------------
// 调试输出函数：打印字段写入来源信息
// ----------------------------------------------------------------------------
// 输出字段写入操作的来源信息，包括类型、字段、函数、基本块和来源详情
// 使用 LET 宏处理不同的来源类型
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printFieldWriteSourceInfo (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  ::array_detect_ns::FieldWriteInfo const &capture,
  ::array_detector::WriteOriginalSource *source_info
);

// 简化版本：不需要 capture 参数（用于 Wrapper 兼容）
ArrayDetectErrorCode printFieldWriteSourceInfoFromWrapper (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  void const *wrapper,  // Wrapper_FieldWrite_WriteSource_UseAnalysis_EscapeConclude*，但此函数不使用
  ::array_detector::WriteOriginalSource *source_info
);

// ----------------------------------------------------------------------------
// 辅助函数：提取虚函数调用的函数名
// ----------------------------------------------------------------------------
// 从 GIMPLE_CALL 语句中提取虚函数调用的实际函数名
// 使用 matchCallExpression API 来可靠地提取虚函数信息
// 输入：call_stmt - GIMPLE_CALL 语句
// 输出：function_name - 提取的函数名（如果成功，使用 ggc_strdup 分配）
// 返回值：OK 表示成功提取，其他值表示失败或不是虚函数调用
// ----------------------------------------------------------------------------

ArrayDetectErrorCode extractVirtualCallFunctionName (
  AD_FUNC_ARGS,
  gimple * call_stmt,
  char const * &function_name
);

} // namespace array_detect_ns
