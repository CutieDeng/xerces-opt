#pragma once

#define CUTIE_FUNCTION_BEGIN \
  { ::cutie_ns::CutieErrorCode ecode = ::cutie_ns::UNINIT;

#define CUTIE_FUNCTION_END2 \
  return ecode; }

#define CUTIE_FUNCTION_END \
  cleanup:; CUTIE_FUNCTION_END2

#define CUTIE_RETURN \
  do { goto cleanup; } while (0)

#define CUTIE_RETURNV(x) \
  do { ecode = ::cutie_ns::x; goto cleanup; } while (0)

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
  ::cutie_ns::CutieContext &ctx

#define CUTIE_ARGS \
  ctx
