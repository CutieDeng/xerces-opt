#include "gcc-common.hh"

#include "context.hh"
#include "context-init.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

ArrayDetectErrorCode initContextBuffers (AD_FUNC_ARGS, size_t capacity) AD_FUNCTION_BEGIN {
  ctx.source_location_buffer_size = capacity;
  ctx.source_location_buffer = (char*)ggc_alloc_atomic (capacity);
  if (!ctx.source_location_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }
  ctx.source_line_buffer_size = capacity * 2; // 行内容可能更长
  ctx.source_line_buffer = (char*)ggc_alloc_atomic (ctx.source_line_buffer_size);
  if (!ctx.source_line_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }
  ctx.address_format_buffer_size = 512; // 地址格式化缓冲区大小
  ctx.address_format_buffer = (char*)ggc_alloc_atomic (ctx.address_format_buffer_size);
  if (!ctx.address_format_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // 初始化 Racket datum 结果输出缓冲区
  ctx.result_datum_buffer_capacity = 4096;  // 初始容量 4KB
  ctx.result_datum_buffer_size = 0;         // 当前使用量
  ctx.result_datum_buffer = (char*)ggc_alloc_atomic (ctx.result_datum_buffer_capacity);
  if (!ctx.result_datum_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }

  // 初始化转义字符串缓冲区
  ctx.escaped_string_buffer_size = 512;
  ctx.escaped_string_buffer = (char*)ggc_alloc_atomic (ctx.escaped_string_buffer_size);
  if (!ctx.escaped_string_buffer) {
    AD_RETURNE (MEMORY_ERROR);
  }

  AD_RETURNE (OK);
} AD_FUNCTION_END

}
