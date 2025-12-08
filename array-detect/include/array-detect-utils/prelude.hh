#pragma once

// 默认版本，不包含 GCC context 栈帧操作
#define CUTIE_FUNCTION_BEGIN \
  { ::cutie_ns::CutieErrorCode ecode = ::cutie_ns::UNINIT;

#define CUTIE_FUNCTION_BEGIN2 \
  { ::cutie_ns::CutieErrorCode ecode = ::cutie_ns::UNINIT;

#define CUTIE_FUNCTION_END2 \
  return ecode; }

#define CUTIE_FUNCTION_END \
  CUTIE_DEBUG_PRINT("ERROR: walk reachable tail of the function"); \
  return ::cutie_ns::UNREACHABLE; \
  cleanup:; CUTIE_FUNCTION_END2

#define CUTIE_FUNCTION_END_FALLTHROUGH \
  cleanup:; CUTIE_FUNCTION_END2

#define CUTIE_RETURNR \
  do { goto cleanup; } while (0)

#define CUTIE_RETURNV(x) \
  do { ecode = ::cutie_ns::x; goto cleanup; } while (0)

#define CUTIE_RETURNS(v) \
  do { result = (v); ecode = ::cutie_ns::OK; goto cleanup; } while (0)

#define CUTIE_ARGS_WARN_DENY \
  do { (void) ctx; } while (0)

#define CUTIE_DEBUG_PRINT2(file, fmt_msg, ...) \
  do { \
    fprintf(file, "[%s +%d] %s: ", __FILE__, __LINE__, __func__); \
    fprintf(file, fmt_msg, ##__VA_ARGS__); \
    fprintf(file, "\n"); \
  } while (0)

#define CUTIE_DEBUG_PRINT(fmt_msg, ...) \
  CUTIE_DEBUG_PRINT2(ctx.debug_file, fmt_msg, ##__VA_ARGS__)

#define CUTIE_TRY2(rst, brk_label, succ_debug, err_debug, dbg_msg, ...) \
  do { ::cutie_ns::CutieErrorCode ecode1 = (rst); \
    if (ecode1 != ::cutie_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define CUTIE_ETRY2(rst, brk_label, succ_debug, err_debug, dbg_msg, ...) \
  do { ::cutie_ns::CutieErrorCode ecode1 = (rst); \
    if (ecode1 != ::cutie_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        CUTIE_DEBUG_PRINT2(stderr, dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define CUTIE_TRY(rst) \
  CUTIE_TRY2(rst, cleanup, false, true, "Failed: %s")

#define CUTIE_TRY_MSG(rst, msg, ...) \
  CUTIE_TRY2(rst, cleanup, false, true, msg, ##__VA_ARGS__)

#define CUTIE_TRY_LABEL(rst, label) \
  CUTIE_TRY2(rst, label, false, true, "Failed: %s")

#define CUTIE_RTRY2(rst, unmatch_label, brk_label, succ_debug, unmatch_debug, err_debug, dbg_msg, ...) \
  do { \
    ::cutie_ns::CutieErrorCode ecode1 = (rst); \
    if (ecode1 == ::cutie_ns::RECOVERABLE_ERROR) { \
      if (unmatch_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto unmatch_label; \
    } else if (ecode1 != ::cutie_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define CUTIE_FUNC_ARGS \
  ::cutie_ns::CutieContext &ctx, ::cutie_ns::CutieContextGcc &gcc_ctx

#define CUTIE_ARGS \
  ctx, gcc_ctx


// Enhanced stack frame management macros
// 注意：以下宏仅用于需要手动控制栈帧的特殊情况

#define CUTIE_GCC_ENTER_FUNCTION(func_ptr) \
  do { \
    CUTIE_DEBUG_PRINT("GCC: Entering function (ptr: 0x%016lx)", (unsigned long)(func_ptr)); \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::enter_function(ctx, ::cutie_ns::get_cutie_context_gcc_safe(), (func_ptr)); \
    } \
  } while (0)

#define CUTIE_GCC_EXIT_FUNCTION() \
  do { \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::exit_function(ctx, ::cutie_ns::get_cutie_context_gcc_safe()); \
    } \
  } while (0)

// 手动栈帧操作 - 仅在需要特殊控制时使用
#define CUTIE_GCC_PUSH_STACK_FRAME(frame_id) \
  do { \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::push_stack_frame(ctx, ::cutie_ns::get_cutie_context_gcc_safe(), (frame_id)); \
    } \
  } while (0)

#define CUTIE_GCC_POP_STACK_FRAME() \
  do { \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::pop_stack_frame(ctx, ::cutie_ns::get_cutie_context_gcc_safe()); \
    } \
  } while (0)

// Debug macros with GCC context integration
#define CUTIE_GCC_PRINT_STACK_FRAMES() \
  do { \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::print_stack_frames(ctx, ::cutie_ns::get_cutie_context_gcc_safe()); \
    } \
  } while (0)

#define CUTIE_GCC_PRINT_CURRENT_FRAME() \
  do { \
    if (::cutie_ns::is_cutie_context_gcc_initialized()) { \
      ::cutie_ns::print_current_frame(ctx, ::cutie_ns::get_cutie_context_gcc_safe()); \
    } \
  } while (0)
