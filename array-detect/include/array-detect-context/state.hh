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

} // namespace array_detect_ns
