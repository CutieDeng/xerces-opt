#include <stdlib.h>
#include <stdio.h>

#include "cutie-context-gcc.hh"
#include "cutie-context-gcc-interface.hh"

namespace cutie_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
CutieContextGcc gCutieContextGcc;

// Cleanup GCC context
void deinitCutieContextGcc(CUTIE_FUNC_ARGS) {
  clearStackFrames(CUTIE_ARGS);
}

namespace controlflow {

// Stack frame management functions
void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id) {
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void popStackFrame(CUTIE_FUNC_ARGS) {
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

} // namespace controlflow

void clearStackFrames(CUTIE_FUNC_ARGS) {
  CUTIE_ARGS_WARN_DENY;
  gcc_ctx.stack_frames.truncate(0);
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
    char source_buf[256];
    bool has_source = getFrameSourceLocation(CUTIE_ARGS, frame_addr, source_buf, sizeof(source_buf));

    if (has_source) {
      CUTIE_DEBUG_PRINT("  [%zu]: 0x%016lx -> %s", i, (unsigned long)frame_addr, source_buf);
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

  // Try to get source location using GCC's debug information
  // This is a simplified implementation - in a real scenario you might want to use
  // libbfd, libdw, or other debugging libraries to resolve addresses to symbols

  // For now, we'll provide a basic format
  snprintf(buffer, buffer_size, "return_addr:0x%lx", (unsigned long)frame_addr);

  // In a complete implementation, you could:
  // 1. Use dladdr() to get symbol information
  // 2. Use libdw (DWARF) to get exact source line
  // 3. Use GCC's internal debug information APIs
  // 4. Integrate with addr2line functionality

  return true;
}

} // namespace cutie_ns
