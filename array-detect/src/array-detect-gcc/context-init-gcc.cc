#include "gcc-common.hh"

#include "context.hh"
#include "context-init.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

ArrayDetectErrorCode initCapacityImpl(AD_FUNC_ARGS, size_t capacity) AD_FUNCTION_BEGIN {
  ctx.source_location_buffer_size = capacity;
  ctx.source_location_buffer = (char*)ggc_alloc_atomic(capacity);
  if (!ctx.source_location_buffer) {
    AD_RETURNV (MEMORY_ERROR);
  }
  ctx.source_line_buffer_size = capacity * 2; // 行内容可能更长
  ctx.source_line_buffer = (char*)ggc_alloc_atomic(ctx.source_line_buffer_size);
  if (!ctx.source_line_buffer) {
    AD_RETURNV (MEMORY_ERROR);
  }
  AD_RETURNV (OK);
} AD_FUNCTION_END

}
