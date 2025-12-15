#pragma once

#include <stdio.h>

namespace array_detect_ns {

struct ArrayDetectContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);
  
  // 匹配调试开关（虚函数匹配失败时输出调试信息）
  bool match_debug_tracer;
  
  // 预分配的缓冲区用于源码位置信息
  char *source_location_buffer;
  size_t source_location_buffer_size;
  char *source_line_buffer;
  size_t source_line_buffer_size;
  
  // 预分配的缓冲区用于地址解析格式化
  char *address_format_buffer;
  size_t address_format_buffer_size;
};

extern ArrayDetectContext g_array_detect_ctx;

// Forward declaration for GCC-specific context
struct ArrayDetectContextGcc;

} // namespace array_detect_ns
