#pragma once

#include <stdio.h>

namespace array_detect_ns {

struct ArrayDetectContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);

  // 结果输出文件路径（通过 AD_RESULT_FILE 环境变量配置）
  // 如果为 nullptr，则不输出结果
  char const *result_file_path;

  // 当前编译的主输入文件路径（用于在结果中区分不同编译流程）
  char const *current_input_file;

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

  // 预分配的缓冲区用于 Racket datum 结果输出
  char *result_datum_buffer;
  size_t result_datum_buffer_size;
  size_t result_datum_buffer_capacity;  // 当前容量（可动态扩展）
  char *escaped_string_buffer;          // 用于转义字符串
  size_t escaped_string_buffer_size;

  // LTO support: store unified results for later serialization
  // Type: vec<UnifiedFieldAnalysisResult*, va_gc>* (use void* to avoid header dependency)
  void *unified_results;
};

extern ArrayDetectContext g_array_detect_ctx;

// Forward declaration for GCC-specific context
struct ArrayDetectContextGcc;

} // namespace array_detect_ns
