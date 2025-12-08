#include <dlfcn.h>
#include <cstdio>
#include <cstring>

#include "address-resolver.hh"

namespace cutie_ns {

// 全局地址解析器实例
DefaultAddressResolver g_address_resolver;

// DefaultAddressResolver 实现
bool DefaultAddressResolver::resolveAddress(CUTIE_FUNC_ARGS, uint64_t addr, AddressInfo& info) {
  CUTIE_ARGS_WARN_DENY;

  // 初始化结构
  info.symbol_name = nullptr;
  info.source_file = nullptr;
  info.line_number = 0;
  info.column_number = 0;
  info.offset = 0;
  info.is_valid = false;

  if (addr == 0) {
    return false;
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

  return info.is_valid;
}

bool DefaultAddressResolver::resolveAddressToString(CUTIE_FUNC_ARGS, uint64_t addr, const char*& result) {
  CUTIE_ARGS_WARN_DENY;

  AddressInfo info;
  bool has_info = resolveAddress(CUTIE_ARGS, addr, info);

  if (!has_info) {
    snprintf(format_buffer, sizeof(format_buffer), "addr:0x%lx", (unsigned long)addr);
    result = format_buffer;
    return false;
  }

  // 格式化输出
  if (info.source_file) {
    if (info.offset == 0) {
      snprintf(format_buffer, sizeof(format_buffer),
               "%s (%s)", info.symbol_name ? info.symbol_name : "<unknown>",
               info.source_file);
    } else {
      snprintf(format_buffer, sizeof(format_buffer),
               "%s+0x%lx (%s)", info.symbol_name ? info.symbol_name : "<unknown>",
               (unsigned long)info.offset, info.source_file);
    }
  } else {
    if (info.offset == 0) {
      snprintf(format_buffer, sizeof(format_buffer),
               "%s", info.symbol_name ? info.symbol_name : "<unknown>");
    } else {
      snprintf(format_buffer, sizeof(format_buffer),
               "%s+0x%lx", info.symbol_name ? info.symbol_name : "<unknown>",
               (unsigned long)info.offset);
    }
  }

  result = format_buffer;
  return true;
}

// 便捷函数实现
bool resolveAddress(CUTIE_FUNC_ARGS, uint64_t addr, AddressInfo& info) {
  return g_address_resolver.resolveAddress(CUTIE_ARGS, addr, info);
}

bool resolveAddressToString(CUTIE_FUNC_ARGS, uint64_t addr, const char*& result) {
  return g_address_resolver.resolveAddressToString(CUTIE_ARGS, addr, result);
}

} // namespace cutie_ns