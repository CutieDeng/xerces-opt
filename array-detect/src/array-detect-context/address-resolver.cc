#include <dlfcn.h>
#include <cstdio>
#include <cstring>

#include "address-resolver.hh"
#include "array-detect-context-gcc.hh"

namespace array_detect_ns {

// 解析地址到源码位置信息
ArrayDetectErrorCode resolveAddress(AD_FUNC_ARGS, uint64_t addr, AddressInfo &info) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;

  // 初始化结构
  info.symbol_name = nullptr;
  info.source_file = nullptr;
  info.line_number = 0;
  info.column_number = 0;
  info.offset = 0;
  info.is_valid = false;

  if (addr == 0) {
    AD_RETURNE(OK);
  }

  // 使用 dladdr 进行基本符号解析
  Dl_info dl_info;
  if (dladdr((void*)addr, &dl_info)) {
    if (dl_info.dli_sname) {
      info.symbol_name = dl_info.dli_sname;
      info.offset = (size_t)((char*)addr - (char*)dl_info.dli_saddr);
      info.is_valid = true;
    }

    if (dl_info.dli_fname) {
      info.source_file = dl_info.dli_fname;
    }
  }

  AD_RETURNE(OK);
} AD_FUNCTION_END

// 解析地址到字符串形式（使用 Context 缓冲区）
ArrayDetectErrorCode resolveAddressToString(AD_FUNC_ARGS, uint64_t addr, const char* &result, bool &out_is_valid) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;

  // 检查缓冲区是否已初始化
  if (!ctx.address_format_buffer || ctx.address_format_buffer_size == 0) {
    result = "<no buffer>";
    out_is_valid = false;
    AD_RETURNE(OK);
  }

  AddressInfo info;
  AD_TRY(resolveAddress(AD_ARGS, addr, info));

  out_is_valid = info.is_valid;

  if (!info.is_valid) {
    snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size, 
             "addr:0x%lx", (unsigned long)addr);
    result = ctx.address_format_buffer;
    AD_RETURNE(OK);
  }

  // 格式化输出
  if (info.source_file) {
    if (info.offset == 0) {
      snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size,
               "%s (%s)", info.symbol_name ? info.symbol_name : "<unknown>",
               info.source_file);
    } else {
      snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size,
               "%s+0x%lx (%s)", info.symbol_name ? info.symbol_name : "<unknown>",
               (unsigned long)info.offset, info.source_file);
    }
  } else {
    if (info.offset == 0) {
      snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size,
               "%s", info.symbol_name ? info.symbol_name : "<unknown>");
    } else {
      snprintf(ctx.address_format_buffer, ctx.address_format_buffer_size,
               "%s+0x%lx", info.symbol_name ? info.symbol_name : "<unknown>",
               (unsigned long)info.offset);
    }
  }

  result = ctx.address_format_buffer;
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
