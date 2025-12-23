#pragma once

#include <stdint.h>

namespace array_detect_ns {

enum ArrayDetectErrorCode : int64_t {
#define AD_ERROR_DEF(e, d) e,
#include "array-detect-state.txt"
#undef AD_ERROR_DEF
};

extern char const *ERROR_DESCRIPTION[];

extern char const *ERROR_S_DESCRIPTION[];

// 获取错误码名称（用于调试输出）
inline char const* getErrorCodeName(ArrayDetectErrorCode code) {
  // 错误码从 0 开始，最大值为 UNKNOWN_ERROR
  if (code >= 0 && code <= UNKNOWN_ERROR) {
    return ERROR_S_DESCRIPTION[code];
  }
  return "UNKNOWN_ERROR_CODE";
}

} // namespace array_detect_ns
