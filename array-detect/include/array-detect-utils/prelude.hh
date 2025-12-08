#pragma once

#define CUTIE_FUNCTION_BEGIN \
  { ::cutie_ns::CutieErrorCode ecode = ::cutie_ns::UNINIT; \
    void *cutie_auto_ret_addr = (void*)((uintptr_t)__builtin_return_address(0) - 4); \
      ::cutie_ns::controlflow::pushStackFrame(CUTIE_ARGS, (uint64_t)cutie_auto_ret_addr); \

#define CUTIE_FUNCTION_BEGIN2 \
  { ::cutie_ns::CutieErrorCode ecode = ::cutie_ns::UNINIT;

#define CUTIE_FUNCTION_END2 \
  do { \
    ::cutie_ns::controlflow::popStackFrame(CUTIE_ARGS); \
  } while (0); \
  CUTIE_FUNCTION_END3

#define CUTIE_FUNCTION_END3 \
  return ecode; }

#define CUTIE_FUNCTION_END \
  CUTIE_RETURNV (UNREACHABLE); \
  cleanup:; \
  CUTIE_FUNCTION_END2

#define CUTIE_RETURNR \
  do { goto cleanup; } while (0)

#define CUTIE_RETURNV(x) \
  do { ecode = ::cutie_ns::x; goto cleanup; } while (0)

#define CUTIE_RETURNS(v) \
  do { result = (v); ecode = ::cutie_ns::OK; goto cleanup; } while (0)

#define CUTIE_ARGS_WARN_DENY \
  do { (void) ctx; (void) gcc_ctx; } while (0)

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

// ============================================================================
// 手动栈帧管理宏 - 用于特殊控制需求
// ============================================================================

#define CUTIE_GCC_ENTER_FUNCTION(func_ptr) \
  do { \
    CUTIE_DEBUG_PRINT("GCC: Entering function (ptr: 0x%016lx)", (unsigned long)(func_ptr)); \
    ::cutie_ns::enterFunction(CUTIE_ARGS, (func_ptr)); \
  } while (0)

#define CUTIE_GCC_EXIT_FUNCTION() \
  do { \
    ::cutie_ns::exitFunction(CUTIE_ARGS); \
  } while (0)

// Debug macros with GCC context integration
#define CUTIE_GCC_PRINT_STACK_FRAMES() \
  do { \
    ::cutie_ns::printStackFrames(CUTIE_ARGS); \
  } while (0)

#define CUTIE_GCC_PRINT_CURRENT_FRAME() \
  do { \
    ::cutie_ns::printCurrentFrame(CUTIE_ARGS); \
  } while (0)

// Enhanced macros with source code location information
#define CUTIE_GCC_PRINT_STACK_WITH_SOURCE() \
  do { \
    ::cutie_ns::printStackFramesWithSource(CUTIE_ARGS); \
  } while (0)

#define CUTIE_GCC_PRINT_FRAME_SOURCE(frame_addr) \
  do { \
    ::cutie_ns::printStackFrameSource(CUTIE_ARGS, (frame_addr)); \
  } while (0)

// Macro to print current call stack with source locations
#define CUTIE_GCC_DUMP_CALL_STACK() \
  do { \
    CUTIE_DEBUG_PRINT("=== Call Stack Dump ==="); \
    CUTIE_GCC_PRINT_STACK_WITH_SOURCE(); \
    CUTIE_DEBUG_PRINT("=== End Call Stack ==="); \
  } while (0)

