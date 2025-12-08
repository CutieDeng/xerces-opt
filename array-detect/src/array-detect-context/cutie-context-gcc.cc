#include "cutie-context-gcc.hh"
#include "cutie-context-gcc-manager.h"
#include "gcc-common.hh"

namespace cutie_ns {

// Stack frame management - 直接使用所有 context 参数
void push_stack_frame(CutieContext& ctx, CutieContextGcc& gcc_ctx, uint64_t frame_id) {
  (void)ctx; // Suppress unused parameter warning
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void pop_stack_frame(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  (void)ctx; // Suppress unused parameter warning
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

void clear_stack_frames(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  (void)ctx; // Suppress unused parameter warning
  gcc_ctx.stack_frames.truncate(0);
}

// Stack frame query functions
size_t get_stack_depth(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  (void)ctx; // Suppress unused parameter warning
  return gcc_ctx.stack_frames.length();
}

uint64_t get_current_frame(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  (void)ctx; // Suppress unused parameter warning
  if (gcc_ctx.stack_frames.is_empty()) {
    return 0;
  }
  return const_cast<vec<uint64_t>&>(gcc_ctx.stack_frames).last();
}

bool is_stack_empty(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  (void)ctx; // Suppress unused parameter warning
  return gcc_ctx.stack_frames.is_empty();
}

// Debug output functions
void print_stack_frames(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  if (!ctx.debug_file) return;

  fprintf(ctx.debug_file, "=== GCC Stack Frame Information ===\n");
  fprintf(ctx.debug_file, "Stack depth: %zu\n", get_stack_depth(ctx, gcc_ctx));

  if (gcc_ctx.stack_frames.is_empty()) {
    fprintf(ctx.debug_file, "Stack is empty\n");
  } else {
    fprintf(ctx.debug_file, "Stack frames (top to bottom):\n");

    for (int i = (int)gcc_ctx.stack_frames.length() - 1; i >= 0; i--) {
      uint64_t frame_id = gcc_ctx.stack_frames[i];
      fprintf(ctx.debug_file, "  [%d] Frame ID: 0x%016lx\n",
              gcc_ctx.stack_frames.length() - i - 1, (unsigned long)frame_id);

      // 尝试获取函数名（如果可用）
      tree frame_tree = (tree)frame_id;
      if (DECL_P(frame_tree)) {
        const char* name = DECL_NAME(frame_tree) ? IDENTIFIER_POINTER(DECL_NAME(frame_tree)) : "<unknown>";
        fprintf(ctx.debug_file, "      Function: %s\n", name);
      }
    }
  }
  fprintf(ctx.debug_file, "========================================\n");
}

void print_current_frame(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  if (!ctx.debug_file) return;

  if (gcc_ctx.stack_frames.is_empty()) {
    fprintf(ctx.debug_file, "Current: No active stack frame\n");
    return;
  }

  uint64_t current_frame = get_current_frame(ctx, gcc_ctx);
  int stack_pos = gcc_ctx.stack_frames.length();

  fprintf(ctx.debug_file, "Current stack frame: [depth=%d] id=0x%016lx",
          stack_pos - 1, (unsigned long)current_frame);

  // 尝试获取函数名
  tree current_tree = (tree)current_frame;
  if (DECL_P(current_tree)) {
    const char* name = DECL_NAME(current_tree) ? IDENTIFIER_POINTER(DECL_NAME(current_tree)) : "<unknown>";
    fprintf(ctx.debug_file, " function=%s", name);
  }

  fprintf(ctx.debug_file, "\n");
}

// Convenience functions for function tracking
void enter_function(CutieContext& ctx, CutieContextGcc& gcc_ctx, uint64_t function_ptr) {
  push_stack_frame(ctx, gcc_ctx, function_ptr);

  if (ctx.debug_file) {
    const char* func_name = "<unknown>";
    tree func_tree = (tree)function_ptr;
    if (DECL_P(func_tree)) {
      func_name = DECL_NAME(func_tree) ? IDENTIFIER_POINTER(DECL_NAME(func_tree)) : "<unnamed>";
    }

    fprintf(ctx.debug_file, "[%s +%d] %s: Entering function (ptr: 0x%016lx) - function: %s - stack depth: %zu\n",
            __FILE__, __LINE__, __func__, (unsigned long)function_ptr, func_name, get_function_depth(ctx, gcc_ctx));
  }
}

void exit_function(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  if (ctx.debug_file && !gcc_ctx.stack_frames.is_empty()) {
    uint64_t leaving_frame = get_current_frame(ctx, gcc_ctx);
    const char* func_name = "<unknown>";

    tree leaving_tree = (tree)leaving_frame;
    if (DECL_P(leaving_tree)) {
      func_name = DECL_NAME(leaving_tree) ? IDENTIFIER_POINTER(DECL_NAME(leaving_tree)) : "<unnamed>";
    }

    fprintf(ctx.debug_file, "[%s +%d] %s: Exiting function (ptr: 0x%016lx) - function: %s - stack depth after: %zu\n",
            __FILE__, __LINE__, __func__, (unsigned long)leaving_frame, func_name, get_function_depth(ctx, gcc_ctx) - 1);
  }

  pop_stack_frame(ctx, gcc_ctx);
}

// Function depth analysis
size_t get_function_depth(CutieContext& ctx, CutieContextGcc& gcc_ctx) {
  return get_stack_depth(ctx, gcc_ctx);
}

bool is_in_function(CutieContext& ctx, CutieContextGcc& gcc_ctx, uint64_t function_ptr) {
  (void)ctx; // Suppress unused parameter warning
  for (unsigned int i = 0; i < gcc_ctx.stack_frames.length(); i++) {
    if (gcc_ctx.stack_frames[i] == function_ptr) {
      return true;
    }
  }
  return false;
}

// Utility function to get function pointer from GCC tree
uint64_t get_function_pointer(tree node) {
  if (!node) return 0;

  // 对于函数声明或函数定义，返回其指针
  if (TREE_CODE(node) == FUNCTION_DECL || TREE_CODE(node) == FUNCTION_TYPE) {
    return (uint64_t)node;
  }

  // 对于其他节点，尝试提取函数信息
  return (uint64_t)node;
}

} // namespace cutie_ns