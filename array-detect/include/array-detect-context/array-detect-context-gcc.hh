#pragma once

#include <stdint.h>

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"

namespace array_detect_ns {

// GCC上下文管理函数 - 使用常规 aBC 命名法
void initArrayDetectContextGcc (AD_FUNC_ARGS);
void deinitArrayDetectContextGcc (AD_FUNC_ARGS);

namespace controlflow {

void pushStackFrame (AD_FUNC_ARGS, uint64_t frame_id);
void popStackFrame (AD_FUNC_ARGS);

} // namespace controlflow

// Stack frame management functions - 使用统一宏
void clearStackFrames (AD_FUNC_ARGS);

// Stack frame query functions
size_t getStackDepth (AD_FUNC_ARGS);
void getCurrentFrame (AD_FUNC_ARGS, uint64_t &result, bool &is_exists);
bool isStackEmpty (AD_FUNC_ARGS);

// Debug output functions
void printStackFrames (AD_FUNC_ARGS);
void printCurrentFrame (AD_FUNC_ARGS);

// Enhanced debug with source code locations
void printStackFramesWithSource (AD_FUNC_ARGS);
void printStackFrameSource (AD_FUNC_ARGS, uint64_t frame_addr);
bool getFrameSourceLocation (AD_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size);

// 引入增强的地址解析器
#include "address-resolver.hh"

// 使用新地址解析器的便捷包装
bool getFrameSourceLocationEnhanced (AD_FUNC_ARGS, uint64_t frame_addr, const char*& result);

// Convenience functions for function tracking
void enterFunction (AD_FUNC_ARGS, uint64_t function_ptr);
void exitFunction (AD_FUNC_ARGS);

// Function depth analysis
size_t getFunctionDepth (AD_FUNC_ARGS);
bool isInFunction (AD_FUNC_ARGS, uint64_t function_ptr);

// GCC特定上下文类型 - 使用 public 变量，无访问控制
struct ArrayDetectContextGcc {
  // 栈帧信息 - 用于跟踪函数调用栈，使用指针类型以支持延迟初始化
  vec<uint64_t> stack_frames;
};

// 内部全局对象，不暴露给外部
extern ArrayDetectContextGcc gArrayDetectContextGcc;

} // namespace array_detect_ns
