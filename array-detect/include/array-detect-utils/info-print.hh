#pragma once

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "gcc-common.hh"

namespace array_detector {
  class ArrayDetector;
  struct FieldSourceInfo;
}

namespace array_detect_ns {
  struct FieldWriteCapture;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// ----------------------------------------------------------------------------
// printResults 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

ArrayDetectErrorCode printResults (AD_FUNC_ARGS, ArrayDetector &detector);

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
  ::array_detect_ns::FieldWriteCapture const &capture,
  ::array_detector::FieldSourceInfo *source_info
);

} // namespace array_detect_ns
