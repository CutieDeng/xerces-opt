#pragma once

#include <stdio.h>

namespace cutie_ns {

struct CutieContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);
  
  // 预分配的缓冲区用于源码位置信息
  char *source_location_buffer;
  size_t source_location_buffer_size;
  char *source_line_buffer;
  size_t source_line_buffer_size;
};

extern CutieContext g_cutie_ctx;

// Forward declaration for GCC-specific context
struct CutieContextGcc;

} // namespace cutie_ns
