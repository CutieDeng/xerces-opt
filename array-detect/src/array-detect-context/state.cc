#include "state.hh"

namespace array_detect_ns {

char const *ERROR_DESCRIPTION[] = {
#define AD_ERROR_DEF(e, d) d,
#include "array-detect-state.txt"
#undef AD_ERROR_DEF
};

char const *ERROR_S_DESCRIPTION[] = {
#define AD_ERROR_DEF(e, d) #e,
#include "array-detect-state.txt"
#undef AD_ERROR_DEF
};

}
