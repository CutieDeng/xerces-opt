#pragma once

#include <stdint.h>
#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detect_ns {

// 地址解析结果结构
struct AddressInfo {
  char const * symbol_name;      // 符号名称
  char const * source_file;      // 源文件路径
  unsigned int line_number;     // 行号
  unsigned int column_number;   // 列号
  size_t offset;                // 相对符号的偏移
  bool is_valid;                // 信息是否有效
};

// 解析地址到源码位置信息
// 返回值：ArrayDetectErrorCode
// 输出：通过 info 参数返回解析结果
ArrayDetectErrorCode resolveAddress (AD_FUNC_ARGS, uint64_t addr, AddressInfo &info);

// 解析地址到字符串形式（使用 Context 缓冲区）
// 返回值：ArrayDetectErrorCode
// 输出：通过 result 参数返回格式化字符串
// 输出：通过 out_is_valid 参数返回是否成功解析
ArrayDetectErrorCode resolveAddressToString (AD_FUNC_ARGS, uint64_t addr, char const * &result, bool &out_is_valid);

} // namespace array_detect_ns
