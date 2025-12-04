#pragma once

#include <stdio.h>

namespace cutie_ns {

struct CutieContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);
  
  // 预分配的缓冲区用于源码位置信息
  char source_location_buffer[512];
  char source_line_buffer[1024];
};

extern CutieContext g_cutie_ctx;

} // namespace cutie_ns
