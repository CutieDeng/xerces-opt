#include "cutie-context-gcc.hh"
#include "cutie-context-gcc-manager.h"
#include "gcc-common.hh"

namespace cutie_ns {

// Stack frame management
void push_stack_frame(CutieContextGcc& self, uint64_t frame_id) {
  self.stack_frames.safe_push(frame_id);
}

void pop_stack_frame(CutieContextGcc& self) {
  if (!self.stack_frames.is_empty()) {
    self.stack_frames.pop();
  }
}

void clear_stack_frames(CutieContextGcc& self) {
  self.stack_frames.truncate(0);
}

// Stack frame query
size_t get_stack_depth(CutieContextGcc const &self) {
  return self.stack_frames.length();
}

bool is_stack_empty(CutieContextGcc const &self) {
  return self.stack_frames.is_empty();
}

uint64_t get_current_frame(CutieContextGcc const &self) {
  if (self.stack_frames.is_empty()) {
    return 0;
  }
  return const_cast<vec<uint64_t>&>(self.stack_frames).last();
}

// Debug output functions
void print_stack_frames(CutieContextGcc const &self, CutieContext& ctx) {
  if (!ctx.debug_file) return;

  fprintf(ctx.debug_file, "=== GCC Stack Frame Information ===\n");
  fprintf(ctx.debug_file, "Stack depth: %zu\n", get_stack_depth(self));

  if (self.stack_frames.is_empty()) {
    fprintf(ctx.debug_file, "Stack is empty\n");
  } else {
    fprintf(ctx.debug_file, "Stack frames (top to bottom):\n");

    for (int i = (int)self.stack_frames.length() - 1; i >= 0; i--) {
      uint64_t frame_id = self.stack_frames[i];
      fprintf(ctx.debug_file, "  [%d] Frame ID: 0x%016lx\n",
              self.stack_frames.length() - i - 1, (unsigned long)frame_id);

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

void print_current_frame(CutieContextGcc const &self, CutieContext& ctx) {
  if (!ctx.debug_file) return;

  if (self.stack_frames.is_empty()) {
    fprintf(ctx.debug_file, "Current: No active stack frame\n");
    return;
  }

  uint64_t current_frame = get_current_frame(self);
  int stack_pos = self.stack_frames.length();

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

// Get current function's stack frame depth
size_t get_function_depth(CutieContextGcc const &self) {
  return get_stack_depth(self);
}

// Check if currently in specified function (for depth analysis)
bool is_in_function(CutieContextGcc const &self, uint64_t function_ptr) {
  for (unsigned int i = 0; i < self.stack_frames.length(); i++) {
    if (self.stack_frames[i] == function_ptr) {
      return true;
    }
  }
  return false;
}

// Convenience functions for function tracking
void enter_function(CutieContextGcc& self, uint64_t function_ptr, CutieContext& ctx) {
  push_stack_frame(self, function_ptr);

  if (ctx.debug_file) {
    const char* func_name = "<unknown>";
    tree func_tree = (tree)function_ptr;
    if (DECL_P(func_tree)) {
      func_name = DECL_NAME(func_tree) ? IDENTIFIER_POINTER(DECL_NAME(func_tree)) : "<unnamed>";
    }

    fprintf(ctx.debug_file, "[%s +%d] %s: Entering function (ptr: 0x%016lx) - function: %s - stack depth: %zu\n",
            __FILE__, __LINE__, __func__, (unsigned long)function_ptr, func_name, get_function_depth(self));
  }
}

void exit_function(CutieContextGcc& self, CutieContext& ctx) {
  if (ctx.debug_file && !self.stack_frames.is_empty()) {
    uint64_t leaving_frame = get_current_frame(self);
    const char* func_name = "<unknown>";

    tree leaving_tree = (tree)leaving_frame;
    if (DECL_P(leaving_tree)) {
      func_name = DECL_NAME(leaving_tree) ? IDENTIFIER_POINTER(DECL_NAME(leaving_tree)) : "<unnamed>";
    }

    fprintf(ctx.debug_file, "[%s +%d] %s: Exiting function (ptr: 0x%016lx) - function: %s - stack depth after: %zu\n",
            __FILE__, __LINE__, __func__, (unsigned long)leaving_frame, func_name, get_function_depth(self) - 1);
  }

  pop_stack_frame(self);
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