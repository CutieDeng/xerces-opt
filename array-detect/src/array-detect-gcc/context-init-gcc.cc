#include "gcc-common.hh"

#include "context.hh"
#include "context-init.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

ArrayDetectErrorCode initContextBuffers(AD_FUNC_ARGS, size_t capacity) AD_FUNCTION_BEGIN {
  ctx.source_location_buffer_size = capacity;
  ctx.source_location_buffer = (char*)ggc_alloc_atomic(capacity);
  if (!ctx.source_location_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }
  ctx.source_line_buffer_size = capacity * 2; // 行内容可能更长
  ctx.source_line_buffer = (char*)ggc_alloc_atomic(ctx.source_line_buffer_size);
  if (!ctx.source_line_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }
  ctx.address_format_buffer_size = 512; // 地址格式化缓冲区大小
  ctx.address_format_buffer = (char*)ggc_alloc_atomic(ctx.address_format_buffer_size);
  if (!ctx.address_format_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }
  AD_RETURNE (OK);
} AD_FUNCTION_END

}
