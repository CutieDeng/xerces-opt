#include "gcc-common.hh"

#include "context.hh"
#include "context-init.hh"

namespace cutie_ns {

CutieErrorCode initCapacityImpl(CUTIE_FUNC_ARGS, size_t capacity) CUTIE_FUNCTION_BEGIN {
  ctx.source_location_buffer_size = capacity;
  ctx.source_location_buffer = (char*)ggc_alloc_atomic(capacity);
  if (!ctx.source_location_buffer) {
    CUTIE_RETURNV (MEMORY_ERROR);
  }
  ctx.source_line_buffer_size = capacity * 2; // 行内容可能更长
  ctx.source_line_buffer = (char*)ggc_alloc_atomic(ctx.source_line_buffer_size);
  if (!ctx.source_line_buffer) {
    CUTIE_RETURNV (MEMORY_ERROR);
  }
  CUTIE_RETURNV (OK);
} CUTIE_FUNCTION_END

}
