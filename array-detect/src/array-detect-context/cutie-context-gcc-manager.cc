#include <cstdlib>
#include "cutie-context-gcc.hh"

namespace cutie_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
CutieContextGcc g_cutie_ctx_gcc;
static bool g_cutie_ctx_gcc_initialized = false;

// Initialize GCC context
::cutie_ns::CutieErrorCode init_cutie_context_gcc_internal(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  if (g_cutie_ctx_gcc_initialized) {
    CUTIE_DEBUG_PRINT("GCC context already initialized, cleaning up first");
    deinit_cutie_context_gcc_internal(CUTIE_ARGS);
  }

  // 初始化复杂对象（如果需要）
  // g_cutie_ctx_gcc 的 vec 会被自动初始化为空

  g_cutie_ctx_gcc_initialized = true;
  CUTIE_DEBUG_PRINT("GCC context initialized successfully");
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// Cleanup GCC context
void deinit_cutie_context_gcc_internal(CUTIE_FUNC_ARGS) {
  if (g_cutie_ctx_gcc_initialized) {
    if (!is_stack_empty(CUTIE_ARGS)) {
      CUTIE_DEBUG_PRINT("Warning: GCC context has non-empty stack during cleanup");
      print_stack_frames(CUTIE_ARGS);
    }

    // 清理复杂对象状态
    clear_stack_frames(CUTIE_ARGS);

    g_cutie_ctx_gcc_initialized = false;
    CUTIE_DEBUG_PRINT("GCC context deinitialized");
  }
}

// Check if GCC context is initialized
bool is_cutie_context_gcc_initialized_internal() {
  return g_cutie_ctx_gcc_initialized;
}

// Get GCC context or assert if not initialized
CutieContextGcc& get_cutie_context_gcc_safe() {
  if (!g_cutie_ctx_gcc_initialized) {
    // In debug mode, this would cause an assertion failure
    // In production, this should return a safe default or throw
    fprintf(stderr, "FATAL: GCC context not initialized!\n");
    abort();
  }
  return g_cutie_ctx_gcc;
}

} // namespace cutie_ns
