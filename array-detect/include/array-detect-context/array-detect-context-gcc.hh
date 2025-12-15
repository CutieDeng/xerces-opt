#pragma once

#include <stdint.h>

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"

namespace array_detect_ns {

// GCC上下文管理函数
void initGccContext (AD_FUNC_ARGS);
void deinitGccContext (AD_FUNC_ARGS);

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
// 将栈帧地址解析为源码位置字符串（使用内部缓冲区）
// 返回值：ArrayDetectErrorCode
// 输出：通过 buffer 参数返回格式化字符串
ArrayDetectErrorCode getFrameSourceLocation (AD_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size);

} // namespace array_detect_ns

// 引入增强的地址解析器
#include "address-resolver.hh"

namespace array_detect_ns {

// 将栈帧地址解析为源码位置字符串（使用上下文缓冲区，更高效）
// 返回值：ArrayDetectErrorCode
// 输出：通过 result 参数返回格式化字符串
// 输出：通过 out_is_valid 参数返回是否成功解析
ArrayDetectErrorCode resolveFrameAddressToSource (AD_FUNC_ARGS, uint64_t frame_addr, const char* &result, bool &out_is_valid);

// 当前栈帧信息结构
struct CurrentFrameInfo {
  uint64_t frame_address;        // 栈帧地址
  bool is_valid;                  // 是否有效
  const char* source_location;    // 源码位置信息（使用上下文缓冲区）
  const char* demangled_name;    // 解析后的函数名（使用上下文缓冲区）
  const char* source_file;        // 源码文件名（使用上下文缓冲区）
  unsigned int source_line;       // 源码行号
  size_t stack_depth;             // 当前栈深度
};

// 获取当前栈帧信息
void getCurrentFrameInfo (AD_FUNC_ARGS, CurrentFrameInfo& info);

// 输出当前栈帧信息（格式化输出到调试流）
void printCurrentFrameInfo (AD_FUNC_ARGS);

// 输出所有栈帧信息（从栈底到栈顶）
void printAllStackFramesInfo (AD_FUNC_ARGS);

// GCC特定上下文类型 - 使用 public 变量，无访问控制
struct ArrayDetectContextGcc {
  // 栈帧信息 - 用于跟踪函数调用栈，使用指针类型以支持延迟初始化
  vec<uint64_t> stack_frames;
};

} // namespace array_detect_ns
