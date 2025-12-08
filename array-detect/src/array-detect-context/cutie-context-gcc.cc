#include <cstdlib>
#include "cutie-context-gcc.hh"

namespace cutie_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
CutieContextGcc gCutieContextGcc;
static bool gCutieContextGccInitialized = false;

// Initialize GCC context
::cutie_ns::CutieErrorCode initCutieContextGcc(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  if (gCutieContextGccInitialized) {
    CUTIE_DEBUG_PRINT("GCC context already initialized, cleaning up first");
    deinitCutieContextGcc(CUTIE_ARGS);
  }

  // 初始化复杂对象（如果需要）
  // gCutieContextGcc 的 vec 会被自动初始化为空

  gCutieContextGccInitialized = true;
  CUTIE_DEBUG_PRINT("GCC context initialized successfully");
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// Cleanup GCC context
void deinitCutieContextGcc(CUTIE_FUNC_ARGS) {
  if (gCutieContextGccInitialized) {
    if (!isStackEmpty(CUTIE_ARGS)) {
      CUTIE_DEBUG_PRINT("Warning: GCC context has non-empty stack during cleanup");
      printStackFrames(CUTIE_ARGS);
    }

    // 清理复杂对象状态
    clearStackFrames(CUTIE_ARGS);

    gCutieContextGccInitialized = false;
    CUTIE_DEBUG_PRINT("GCC context deinitialized");
  }
}

// Check if GCC context is initialized
bool isCutieContextGccInitialized() {
  return gCutieContextGccInitialized;
}

// Get GCC context or assert if not initialized
CutieContextGcc& getCutieContextGccSafe() {
  if (!gCutieContextGccInitialized) {
    // In debug mode, this would cause an assertion failure
    // In production, this should return a safe default or throw
    fprintf(stderr, "FATAL: GCC context not initialized!\n");
    abort();
  }
  return gCutieContextGcc;
}

// Stack frame management functions
void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id) {
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void popStackFrame(CUTIE_FUNC_ARGS) {
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

void clearStackFrames(CUTIE_FUNC_ARGS) {
  gcc_ctx.stack_frames.truncate(0);
}

// Stack frame query functions
size_t getStackDepth(CUTIE_FUNC_ARGS) {
  return gcc_ctx.stack_frames.length();
}

uint64_t getCurrentFrame(CUTIE_FUNC_ARGS) {
  if (gcc_ctx.stack_frames.is_empty()) {
    return 0;
  }
  return gcc_ctx.stack_frames.last();
}

bool isStackEmpty(CUTIE_FUNC_ARGS) {
  return gcc_ctx.stack_frames.is_empty();
}

// Debug output functions
void printStackFrames(CUTIE_FUNC_ARGS) {
  CUTIE_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(CUTIE_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    CUTIE_DEBUG_PRINT("  [%zu]: 0x%lx", i, (unsigned long)gcc_ctx.stack_frames[i]);
  }
}

void printCurrentFrame(CUTIE_FUNC_ARGS) {
  if (!isStackEmpty(CUTIE_ARGS)) {
    CUTIE_DEBUG_PRINT("Current frame: 0x%lx", (unsigned long)getCurrentFrame(CUTIE_ARGS));
  } else {
    CUTIE_DEBUG_PRINT("Stack is empty");
  }
}

// Convenience functions for function tracking
void enterFunction(CUTIE_FUNC_ARGS, uint64_t function_ptr) {
  CUTIE_DEBUG_PRINT("Entering function (ptr: %lx)", (unsigned long)function_ptr);
  pushStackFrame(CUTIE_ARGS, function_ptr);
}

void exitFunction(CUTIE_FUNC_ARGS) {
  if (!isStackEmpty(CUTIE_ARGS)) {
    uint64_t current_frame = getCurrentFrame(CUTIE_ARGS);
    CUTIE_DEBUG_PRINT("Exiting function (ptr: %lx)", (unsigned long)current_frame);
  }
  popStackFrame(CUTIE_ARGS);
}

// Function depth analysis
size_t getFunctionDepth(CUTIE_FUNC_ARGS) {
  return getStackDepth(CUTIE_ARGS);
}

bool isInFunction(CUTIE_FUNC_ARGS, uint64_t function_ptr) {
  // Check if the function pointer is in the call stack
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    if (gcc_ctx.stack_frames[i] == function_ptr) {
      return true;
    }
  }
  return false;
}

} // namespace cutie_ns