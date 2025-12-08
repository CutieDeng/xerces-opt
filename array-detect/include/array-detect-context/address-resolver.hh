#pragma once

#include <cstdint>
#include "prelude.hh"
#include "context.hh"
#include "cutie-context-gcc.hh"

namespace cutie_ns {

// 地址解析结果结构
struct AddressInfo {
  const char* symbol_name;      // 符号名称
  const char* source_file;      // 源文件路径
  unsigned int line_number;     // 行号
  unsigned int column_number;   // 列号
  size_t offset;                // 相对符号的偏移
  bool is_valid;                // 信息是否有效
};

// 地址解析器接口
class AddressResolver {
public:
  virtual ~AddressResolver() = default;

  // 解析地址到源码位置
  virtual bool resolveAddress(CUTIE_FUNC_ARGS, uint64_t addr, AddressInfo& info) = 0;

  // 使用 Context 缓冲区的便捷方法
  virtual bool resolveAddressToString(CUTIE_FUNC_ARGS, uint64_t addr, const char*& result) = 0;
};

// 默认地址解析器实现
class DefaultAddressResolver : public AddressResolver {
public:
  bool resolveAddress(CUTIE_FUNC_ARGS, uint64_t addr, AddressInfo& info) override;
  bool resolveAddressToString(CUTIE_FUNC_ARGS, uint64_t addr, const char*& result) override;

private:
  char format_buffer[512];  // 内部格式化缓冲区
};

// 全局地址解析器实例
extern DefaultAddressResolver g_address_resolver;

// 便捷函数声明
bool resolveAddress(CUTIE_FUNC_ARGS, uint64_t addr, AddressInfo& info);
bool resolveAddressToString(CUTIE_FUNC_ARGS, uint64_t addr, const char*& result);

} // namespace cutie_ns