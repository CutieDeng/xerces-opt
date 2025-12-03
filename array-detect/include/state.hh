#pragma once

#include <stdint.h>

namespace cutie_ns {

enum CutieErrorCode : int64_t {
#define CUTIE_ERROR_DEF(e, d) e,
#include "cutie-state.txt"
#undef CUTIE_ERROR_DEF
};

extern char const *ERROR_DESCRIPTION[];

} // namespace cutie_ns
