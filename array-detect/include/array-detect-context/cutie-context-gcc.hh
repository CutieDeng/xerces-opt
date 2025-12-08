#pragma once

#include <stdint.h>

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "gcc-common.hh"

namespace cutie_ns {

class CutieContextGcc;

// Stack frame management functions using ADL pattern
void push_stack_frame(CutieContextGcc& self, uint64_t frame_id);
void pop_stack_frame(CutieContextGcc& self);
void clear_stack_frames(CutieContextGcc& self);

// Stack frame query functions
size_t get_stack_depth(CutieContextGcc const &self);
uint64_t get_current_frame(CutieContextGcc const &self);
bool is_stack_empty(CutieContextGcc const &self);

// Debug output functions
void print_stack_frames(CutieContextGcc const &self, CutieContext& ctx);
void print_current_frame(CutieContextGcc const &self, CutieContext& ctx);

// Convenience functions for function tracking
void enter_function(CutieContextGcc& self, uint64_t function_ptr, CutieContext& ctx);
void exit_function(CutieContextGcc& self, CutieContext& ctx);

// Function depth analysis
size_t get_function_depth(CutieContextGcc const &self);
bool is_in_function(CutieContextGcc const &self, uint64_t function_ptr);

// GCC上下文管理函数
::cutie_ns::CutieErrorCode init_cutie_context_gcc(CUTIE_FUNC_ARGS);
void deinit_cutie_context_gcc(CUTIE_FUNC_ARGS);
bool is_cutie_context_gcc_initialized();
CutieContextGcc* get_cutie_context_gcc();
CutieContextGcc& get_cutie_context_gcc_safe();

// GCC特定上下文类型
struct CutieContextGcc {
  friend void push_stack_frame(CutieContextGcc& self, uint64_t frame_id);
  friend void pop_stack_frame(CutieContextGcc& self);
  friend void clear_stack_frames(CutieContextGcc& self);
  friend size_t get_stack_depth(CutieContextGcc const &self);
  friend uint64_t get_current_frame(CutieContextGcc const &self);
  friend bool is_stack_empty(CutieContextGcc const &self);
  friend void print_stack_frames(CutieContextGcc const &self, CutieContext& ctx);
  friend void print_current_frame(CutieContextGcc const &self, CutieContext& ctx);
  friend void enter_function(CutieContextGcc& self, uint64_t function_ptr, CutieContext& ctx);
  friend void exit_function(CutieContextGcc& self, CutieContext& ctx);
  friend size_t get_function_depth(CutieContextGcc const &self);
  friend bool is_in_function(CutieContextGcc const &self, uint64_t function_ptr);

private:
  // 栈帧信息 - 用于跟踪函数调用栈
  vec<uint64_t> stack_frames;

public:
  CutieContextGcc() : stack_frames() {}
};

// Stack frame management macros for GCC integration
#define CUTIE_GCC_ENTER_FUNCTION(func_ptr) \
  do { \
    if (ctx.debug_file) { \
      fprintf(ctx.debug_file, "[%s +%d] %s: Entering function (ptr: %lu)\n", \
              __FILE__, __LINE__, __func__, (unsigned long)(func_ptr)); \
    } \
    push_stack_frame(gcc_ctx, func_ptr); \
  } while (0)

#define CUTIE_GCC_EXIT_FUNCTION() \
  do { \
    if (ctx.debug_file && !is_stack_empty(gcc_ctx)) { \
      fprintf(ctx.debug_file, "[%s +%d] %s: Exiting function (ptr: %lu)\n", \
              __FILE__, __LINE__, __func__, (unsigned long)get_current_frame(gcc_ctx)); \
    } \
    pop_stack_frame(gcc_ctx); \
  } while (0)

// Enhanced stack frame tracking with debug information
#define CUTIE_GCC_STACK_BEGIN(info) \
  do { \
    if (ctx.debug_file) { \
      fprintf(ctx.debug_file, "[%s +%d] %s: STACK BEGIN: %s\n", \
              __FILE__, __LINE__, __func__, info); \
    } \
    push_stack_frame(gcc_ctx, get_function_pointer()); \
  } while (0)

#define CUTIE_GCC_STACK_END(info) \
  do { \
    if (ctx.debug_file && !is_stack_empty(gcc_ctx)) { \
      fprintf(ctx.debug_file, "[%s +%d] %s: STACK END: %s\n", \
              __FILE__, __LINE__, __func__, info); \
    } \
    pop_stack_frame(gcc_ctx); \
  } while (0)

// Extended function arguments for GCC-specific context
#define CUTIE_GCC_FUNC_ARGS \
  ::cutie_ns::CutieContext &ctx, ::cutie_ns::CutieContextGcc &gcc_ctx

#define CUTIE_GCC_ARGS \
  ctx, gcc_ctx

} // namespace cutie_ns
