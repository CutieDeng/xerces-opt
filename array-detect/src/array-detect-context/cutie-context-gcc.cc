#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "cutie-context-gcc.hh"
#include "cutie-context-gcc-interface.hh"

namespace cutie_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
CutieContextGcc gCutieContextGcc;

// Initialize GCC context
void initCutieContextGcc(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  // 直接初始化 vec 容器
  gcc_ctx.stack_frames.create(0);
}

// Cleanup GCC context
void deinitCutieContextGcc(CUTIE_FUNC_ARGS) {
  clearStackFrames(CUTIE_ARGS);
}

namespace controlflow {

// Stack frame management functions
void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id) {
  CUTIE_ARGS_WARN_DENY;
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void popStackFrame(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

} // namespace controlflow

void clearStackFrames(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  // gcc_ctx.stack_frames.truncate(0);
  // gcc_ctx.stack_frames.release ();
}

size_t getStackDepth(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.length();
}

void getCurrentFrame(CUTIE_FUNC_ARGS, uint64_t &result, bool &is_exists) {
  CUTIE_ARGS_WARN_DENY;
  if (gcc_ctx.stack_frames.is_empty()) {
    is_exists = false;
    return ;
  }
  result = gcc_ctx.stack_frames.last();
  is_exists = true;
}

bool isStackEmpty(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.is_empty();
}

// Debug output functions
void printStackFrames(CUTIE_FUNC_ARGS) {
  CUTIE_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(CUTIE_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    CUTIE_DEBUG_PRINT("\t[%zu]: 0x%lx", i, (unsigned long)gcc_ctx.stack_frames[i]);
  }
}

void printCurrentFrame(CUTIE_FUNC_ARGS) {
  CUTIE_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(CUTIE_ARGS));
  if (!isStackEmpty(CUTIE_ARGS)) {
    bool is_exists;
    uint64_t v;
    getCurrentFrame(CUTIE_ARGS, v, is_exists);
    CUTIE_DEBUG_PRINT("Current frame: 0x%lx", ((unsigned long) (is_exists ? v : 0)));
  } else {
    CUTIE_DEBUG_PRINT("Current frame: <null>");
  }
}

// Enhanced debug with source code locations
void printStackFramesWithSource(CUTIE_FUNC_ARGS) {
  CUTIE_DEBUG_PRINT("Call stack with source locations (depth: %zu):", getStackDepth(CUTIE_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    uint64_t frame_addr = gcc_ctx.stack_frames[i];
    const char* source_info;
    bool has_source = getFrameSourceLocationEnhanced(CUTIE_ARGS, frame_addr, source_info);

    if (has_source) {
      CUTIE_DEBUG_PRINT("  [%zu]: 0x%016lx -> %s", i, (unsigned long)frame_addr, source_info);
    } else {
      CUTIE_DEBUG_PRINT("  [%zu]: 0x%016lx -> <unknown source>", i, (unsigned long)frame_addr);
    }
  }
}

void printStackFrameSource(CUTIE_FUNC_ARGS, uint64_t frame_addr) {
  char source_buf[256];
  bool has_source = getFrameSourceLocation(CUTIE_ARGS, frame_addr, source_buf, sizeof(source_buf));
  if (has_source) {
    CUTIE_DEBUG_PRINT("Frame 0x%016lx -> %s", (unsigned long)frame_addr, source_buf);
  } else {
    CUTIE_DEBUG_PRINT("Frame 0x%016lx -> <unknown source>", (unsigned long)frame_addr);
  }
}

bool getFrameSourceLocation(CUTIE_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size) {
  CUTIE_ARGS_WARN_DENY;
  if (!buffer || buffer_size == 0) {
    return false;
  }

  // Initialize buffer
  buffer[0] = '\0';

  // 保持原有简单实现
  snprintf(buffer, buffer_size, "return_addr:0x%lx", (unsigned long)frame_addr);
  return true;
}

// 使用新地址解析器的增强版本
bool getFrameSourceLocationEnhanced(CUTIE_FUNC_ARGS, uint64_t frame_addr, const char*& result) {
  CUTIE_ARGS_WARN_DENY;

  // 使用 Context 中的 source_location_buffer
  if (!ctx.source_location_buffer || ctx.source_location_buffer_size == 0) {
    result = "<no buffer>";
    return false;
  }

  // 直接使用 dladdr 进行地址解析
  if (frame_addr == 0) {
    snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, "addr:0x%lx", (unsigned long)frame_addr);
    result = ctx.source_location_buffer;
    return false;
  }

  Dl_info info;
  if (dladdr((void*)frame_addr, &info) && info.dli_sname) {
    snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
             "%s+0x%lx", info.dli_sname,
             (unsigned long)((char*)frame_addr - (char*)info.dli_saddr));
    result = ctx.source_location_buffer;
    return true;
  }

  snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, "addr:0x%lx", (unsigned long)frame_addr);
  result = ctx.source_location_buffer;
  return false;
}

} // namespace cutie_ns
