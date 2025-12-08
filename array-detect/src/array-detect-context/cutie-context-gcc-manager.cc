#include <cstdlib>
#include "cutie-context-gcc.hh"

namespace cutie_ns {

// Global GCC-specific context instance
CutieContextGcc* g_cutie_ctx_gcc = nullptr;

// Initialize GCC context
::cutie_ns::CutieErrorCode init_cutie_context_gcc(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  if (g_cutie_ctx_gcc) {
    CUTIE_DEBUG_PRINT("GCC context already initialized, cleaning up first");
    deinit_cutie_context_gcc(CUTIE_ARGS);
  }

  g_cutie_ctx_gcc = new CutieContextGcc();
  if (!g_cutie_ctx_gcc) {
    CUTIE_DEBUG_PRINT("Failed to allocate GCC context");
    CUTIE_RETURNV(MEMORY_ERROR);
  }

  CUTIE_DEBUG_PRINT("GCC context initialized successfully");
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// Cleanup GCC context
void deinit_cutie_context_gcc(CUTIE_FUNC_ARGS) {
  if (g_cutie_ctx_gcc) {
    if (!is_stack_empty(*g_cutie_ctx_gcc)) {
      CUTIE_DEBUG_PRINT("Warning: GCC context has non-empty stack during cleanup");
      print_stack_frames(*g_cutie_ctx_gcc, ctx);
    }

    delete g_cutie_ctx_gcc;
    g_cutie_ctx_gcc = nullptr;
    CUTIE_DEBUG_PRINT("GCC context deinitialized");
  }
}

// Check if GCC context is initialized
bool is_cutie_context_gcc_initialized() {
  return g_cutie_ctx_gcc != nullptr;
}

// Get GCC context (will return nullptr if not initialized)
CutieContextGcc* get_cutie_context_gcc() {
  return g_cutie_ctx_gcc;
}

// Get GCC context or assert if not initialized
CutieContextGcc& get_cutie_context_gcc_safe() {
  if (!g_cutie_ctx_gcc) {
    // In debug mode, this would cause an assertion failure
    // In production, this should return a safe default or throw
    fprintf(stderr, "FATAL: GCC context not initialized!\n");
    abort();
  }
  return *g_cutie_ctx_gcc;
}

} // namespace cutie_ns