#pragma once

#define AD_FUNCTION_BEGIN \
  { ::array_detect_ns::ArrayDetectErrorCode ecode = ::array_detect_ns::UNINIT; \
    void *ad_auto_ret_addr = (void*)((uintptr_t)__builtin_return_address(0) - 4); \
      ::array_detect_ns::controlflow::pushStackFrame(AD_ARGS, (uint64_t)ad_auto_ret_addr); \

#define AD_FUNCTION_BEGIN2 \
  { ::array_detect_ns::ArrayDetectErrorCode ecode = ::array_detect_ns::UNINIT;

#define AD_FUNCTION_END2 \
  do { \
    ::array_detect_ns::controlflow::popStackFrame(AD_ARGS); \
  } while (0); \
  AD_FUNCTION_END3

#define AD_FUNCTION_END3 \
  return ecode; }

#define AD_FUNCTION_END \
  AD_RETURNV (UNREACHABLE); \
  cleanup:; \
  AD_FUNCTION_END2

#define AD_RETURNR \
  do { goto cleanup; } while (0)

#define AD_RETURNV(x) \
  do { ecode = ::array_detect_ns::x; goto cleanup; } while (0)

#define AD_RETURNS(v) \
  do { result = (v); ecode = ::array_detect_ns::OK; goto cleanup; } while (0)

#define AD_ARGS_WARN_DENY \
  do { (void) ctx; (void) gcc_ctx; } while (0)

#define AD_DEBUG_PRINT2(file, fmt_msg, ...) \
  do { \
    fprintf(file, "[%s +%d] %s: ", __FILE__, __LINE__, __func__); \
    fprintf(file, fmt_msg, ##__VA_ARGS__); \
    fprintf(file, "\n"); \
  } while (0)

#define AD_DEBUG_PRINT(fmt_msg, ...) \
  AD_DEBUG_PRINT2(ctx.debug_file, fmt_msg, ##__VA_ARGS__)

#define AD_TRY2(rst, brk_label, succ_debug, err_debug, dbg_msg, ...) \
  do { ::array_detect_ns::ArrayDetectErrorCode ecode1 = (rst); \
    if (ecode1 != ::array_detect_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define AD_ETRY2(rst, brk_label, succ_debug, err_debug, dbg_msg, ...) \
  do { ::array_detect_ns::ArrayDetectErrorCode ecode1 = (rst); \
    if (ecode1 != ::array_detect_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        AD_DEBUG_PRINT2(stderr, dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define AD_TRY(rst) \
  AD_TRY2(rst, cleanup, false, true, "Failed: %s")

#define AD_TRY_MSG(rst, msg, ...) \
  AD_TRY2(rst, cleanup, false, true, msg, ##__VA_ARGS__)

#define AD_TRY_LABEL(rst, label) \
  AD_TRY2(rst, label, false, true, "Failed: %s")

#define AD_RTRY2(rst, unmatch_label, brk_label, succ_debug, unmatch_debug, err_debug, dbg_msg, ...) \
  do { \
    ::array_detect_ns::ArrayDetectErrorCode ecode1 = (rst); \
    if (ecode1 == ::array_detect_ns::RECOVERABLE_ERROR) { \
      if (unmatch_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto unmatch_label; \
    } else if (ecode1 != ::array_detect_ns::OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        AD_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define AD_FUNC_ARGS \
  ::array_detect_ns::ArrayDetectContext &ctx, ::array_detect_ns::ArrayDetectContextGcc &gcc_ctx

#define AD_ARGS \
  ctx, gcc_ctx

// ============================================================================
// 手动栈帧管理宏 - 用于特殊控制需求
// ============================================================================

#define AD_GCC_ENTER_FUNCTION(func_ptr) \
  do { \
    AD_DEBUG_PRINT("GCC: Entering function (ptr: 0x%016lx)", (unsigned long)(func_ptr)); \
    ::array_detect_ns::pushFunctionCall(AD_ARGS, (func_ptr)); \
  } while (0)

#define AD_GCC_EXIT_FUNCTION() \
  do { \
    ::array_detect_ns::popFunctionCall(AD_ARGS); \
  } while (0)

// Debug macros with GCC context integration
#define AD_GCC_PRINT_STACK_FRAMES() \
  do { \
    ::array_detect_ns::printStackFrames(AD_ARGS); \
  } while (0)

#define AD_GCC_PRINT_CURRENT_FRAME() \
  do { \
    ::array_detect_ns::printCurrentFrame(AD_ARGS); \
  } while (0)

// Enhanced macros with source code location information
#define AD_GCC_PRINT_STACK_WITH_SOURCE() \
  do { \
    ::array_detect_ns::printStackFramesWithSource(AD_ARGS); \
  } while (0)

#define AD_GCC_PRINT_FRAME_SOURCE(frame_addr) \
  do { \
    ::array_detect_ns::printStackFrameSource(AD_ARGS, (frame_addr)); \
  } while (0)

// Macro to print current call stack with source locations
#define AD_GCC_DUMP_CALL_STACK() \
  do { \
    AD_DEBUG_PRINT("=== Call Stack Dump ==="); \
    AD_GCC_PRINT_STACK_WITH_SOURCE(); \
    AD_DEBUG_PRINT("=== End Call Stack ==="); \
  } while (0)

