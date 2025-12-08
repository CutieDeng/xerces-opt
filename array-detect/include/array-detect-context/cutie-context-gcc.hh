#pragma once

#include <stdint.h>

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"

namespace cutie_ns {

// GCC上下文管理函数 - 公开接口
::cutie_ns::CutieErrorCode init_cutie_context_gcc(CUTIE_FUNC_ARGS);
void deinit_cutie_context_gcc(CUTIE_FUNC_ARGS);
bool is_cutie_context_gcc_initialized();

// 内部函数 - 不暴露给外部使用
::cutie_ns::CutieErrorCode init_cutie_context_gcc_internal(CUTIE_FUNC_ARGS);
void deinit_cutie_context_gcc_internal(CUTIE_FUNC_ARGS);
bool is_cutie_context_gcc_initialized_internal();

// Stack frame management functions - 使用统一宏
void push_stack_frame(CUTIE_FUNC_ARGS, uint64_t frame_id);
void pop_stack_frame(CUTIE_FUNC_ARGS);
void clear_stack_frames(CUTIE_FUNC_ARGS);

// Stack frame query functions
size_t get_stack_depth(CUTIE_FUNC_ARGS);
uint64_t get_current_frame(CUTIE_FUNC_ARGS);
bool is_stack_empty(CUTIE_FUNC_ARGS);

// Debug output functions
void print_stack_frames(CUTIE_FUNC_ARGS);
void print_current_frame(CUTIE_FUNC_ARGS);

// Convenience functions for function tracking
void enter_function(CUTIE_FUNC_ARGS, uint64_t function_ptr);
void exit_function(CUTIE_FUNC_ARGS);

// Function depth analysis
size_t get_function_depth(CUTIE_FUNC_ARGS);
bool is_in_function(CUTIE_FUNC_ARGS, uint64_t function_ptr);

// GCC特定上下文类型 - 使用 public 变量，无访问控制
struct CutieContextGcc {
  // 栈帧信息 - 用于跟踪函数调用栈
  vec<uint64_t> stack_frames;
};

// 内部全局对象，不暴露给外部
extern CutieContextGcc g_cutie_ctx_gcc;

} // namespace cutie_ns
