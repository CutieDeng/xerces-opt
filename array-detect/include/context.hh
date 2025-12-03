#pragma once

#include <stdio.h>

namespace cutie_ns {

struct CutieContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);
};

extern CutieContext g_cutie_ctx;

} // namespace cutie_ns
