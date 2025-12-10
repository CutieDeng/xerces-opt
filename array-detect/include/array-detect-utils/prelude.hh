#pragma once

// ============================================================================
// 函数框架宏
// ============================================================================

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
  AD_RETURNE (UNREACHABLE); \
  cleanup:; \
  AD_FUNCTION_END2

// ============================================================================
// 返回控制宏
// ============================================================================

// 返回错误码并跳转到 cleanup
#define AD_RETURNE(x) \
  do { ecode = ::array_detect_ns::x; goto cleanup; } while (0)

// 直接跳转到 cleanup（用于提前返回）
#define AD_RETURN() \
  do { goto cleanup; } while (0)

// 成功返回并设置返回值（用于有输出参数的函数）
#define AD_RETURNO(v) \
  do { result = (v); ecode = ::array_detect_ns::OK; goto cleanup; } while (0)

// ============================================================================
// 向后兼容的旧宏（已废弃，保留以避免破坏现有代码）
// ============================================================================

#define AD_RETURNV(x) AD_RETURNE(x)
#define AD_RETURNR AD_RETURN()
#define AD_RETURNS(v) AD_RETURNO(v)

// ============================================================================
// 参数和调试辅助宏
// ============================================================================

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

// ============================================================================
// 错误处理宏
// ============================================================================

// 通用错误处理宏（带自定义标签和调试控制）
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

// 错误处理宏（输出到 stderr）
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

// 简化版错误处理（默认跳转到 cleanup，失败时输出错误）
#define AD_TRY(rst) \
  AD_TRY2(rst, cleanup, false, true, "Failed: %s")

// 带自定义消息的错误处理
#define AD_TRY_MSG(rst, msg, ...) \
  AD_TRY2(rst, cleanup, false, true, msg, ##__VA_ARGS__)

// 带自定义标签的错误处理
#define AD_TRY_LABEL(rst, label) \
  AD_TRY2(rst, label, false, true, "Failed: %s")

// 可恢复错误处理（支持 RECOVERABLE_ERROR 分支）
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

// ============================================================================
// 函数参数宏
// ============================================================================

#define AD_FUNC_ARGS \
  ::array_detect_ns::ArrayDetectContext &ctx, ::array_detect_ns::ArrayDetectContextGcc &gcc_ctx

#define AD_ARGS \
  ctx, gcc_ctx

// ============================================================================
// 栈帧信息输出宏
// ============================================================================

// 输出当前栈帧信息
#define AD_GCC_PRINT_CURRENT_FRAME_INFO() \
  do { \
    ::array_detect_ns::printCurrentFrameInfo(AD_ARGS); \
  } while (0)

// 输出所有栈帧信息（从栈底到栈顶）
#define AD_GCC_PRINT_ALL_STACK_FRAMES() \
  do { \
    ::array_detect_ns::printAllStackFramesInfo(AD_ARGS); \
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
