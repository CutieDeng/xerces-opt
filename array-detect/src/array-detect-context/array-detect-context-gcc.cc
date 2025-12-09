#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "array-detect-context-gcc.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
ArrayDetectContextGcc gArrayDetectContextGcc;

// Initialize GCC context
void initGccContext(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  // 直接初始化 vec 容器
  gcc_ctx.stack_frames.create(0);
}

// Cleanup GCC context
void deinitGccContext(AD_FUNC_ARGS) {
  clearStackFrames(AD_ARGS);
}

namespace controlflow {

// Stack frame management functions
void pushStackFrame(AD_FUNC_ARGS, uint64_t frame_id) {
  AD_ARGS_WARN_DENY;
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void popStackFrame(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

} // namespace controlflow

void clearStackFrames(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  // gcc_ctx.stack_frames.truncate(0);
  // gcc_ctx.stack_frames.release ();
}

size_t getStackDepth(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.length();
}

void getCurrentFrame(AD_FUNC_ARGS, uint64_t &result, bool &is_exists) {
  AD_ARGS_WARN_DENY;
  if (gcc_ctx.stack_frames.is_empty()) {
    is_exists = false;
    return ;
  }
  result = gcc_ctx.stack_frames.last();
  is_exists = true;
}

bool isStackEmpty(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.is_empty();
}

// Debug output functions
void printStackFrames(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(AD_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    AD_DEBUG_PRINT("\t[%zu]: 0x%lx", i, (unsigned long)gcc_ctx.stack_frames[i]);
  }
}

void printCurrentFrame(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(AD_ARGS));
  if (!isStackEmpty(AD_ARGS)) {
    bool is_exists;
    uint64_t v;
    getCurrentFrame(AD_ARGS, v, is_exists);
    AD_DEBUG_PRINT("Current frame: 0x%lx", ((unsigned long) (is_exists ? v : 0)));
  } else {
    AD_DEBUG_PRINT("Current frame: <null>");
  }
}

// Enhanced debug with source code locations
void printStackFramesWithSource(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Call stack with source locations (depth: %zu):", getStackDepth(AD_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    uint64_t frame_addr = gcc_ctx.stack_frames[i];
    const char* source_info;
    bool has_source = resolveFrameAddressToSource(AD_ARGS, frame_addr, source_info);

    if (has_source) {
      AD_DEBUG_PRINT("  [%zu]: 0x%016lx -> %s", i, (unsigned long)frame_addr, source_info);
    } else {
      AD_DEBUG_PRINT("  [%zu]: 0x%016lx -> <unknown source>", i, (unsigned long)frame_addr);
    }
  }
}

void printStackFrameSource(AD_FUNC_ARGS, uint64_t frame_addr) {
  char source_buf[256];
  bool has_source = getFrameSourceLocation(AD_ARGS, frame_addr, source_buf, sizeof(source_buf));
  if (has_source) {
    AD_DEBUG_PRINT("Frame 0x%016lx -> %s", (unsigned long)frame_addr, source_buf);
  } else {
    AD_DEBUG_PRINT("Frame 0x%016lx -> <unknown source>", (unsigned long)frame_addr);
  }
}

bool getFrameSourceLocation(AD_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size) {
  AD_ARGS_WARN_DENY;
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
bool resolveFrameAddressToSource(AD_FUNC_ARGS, uint64_t frame_addr, const char*& result) {
  AD_ARGS_WARN_DENY;

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

} // namespace array_detect_ns
