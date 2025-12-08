#pragma once

#include <stdint.h>

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"

namespace cutie_ns {

// GCC上下文管理函数 - 使用常规 aBC 命名法
void deinitCutieContextGcc(CUTIE_FUNC_ARGS);

namespace controlflow {

void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id);
void popStackFrame(CUTIE_FUNC_ARGS);

} // namespace controlflow

// Stack frame management functions - 使用统一宏
void clearStackFrames(CUTIE_FUNC_ARGS);

// Stack frame query functions
size_t getStackDepth(CUTIE_FUNC_ARGS);
void getCurrentFrame(CUTIE_FUNC_ARGS, uint64_t &result, bool &is_exists);
bool isStackEmpty(CUTIE_FUNC_ARGS);

// Debug output functions
void printStackFrames(CUTIE_FUNC_ARGS);
void printCurrentFrame(CUTIE_FUNC_ARGS);

// Enhanced debug with source code locations
void printStackFramesWithSource(CUTIE_FUNC_ARGS);
void printStackFrameSource(CUTIE_FUNC_ARGS, uint64_t frame_addr);
bool getFrameSourceLocation(CUTIE_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size);

// Convenience functions for function tracking
void enterFunction(CUTIE_FUNC_ARGS, uint64_t function_ptr);
void exitFunction(CUTIE_FUNC_ARGS);

// Function depth analysis
size_t getFunctionDepth(CUTIE_FUNC_ARGS);
bool isInFunction(CUTIE_FUNC_ARGS, uint64_t function_ptr);

// GCC特定上下文类型 - 使用 public 变量，无访问控制
struct CutieContextGcc {
  // 栈帧信息 - 用于跟踪函数调用栈
  vec<uint64_t> stack_frames;
};

// 内部全局对象，不暴露给外部
extern CutieContextGcc gCutieContextGcc;

} // namespace cutie_ns
