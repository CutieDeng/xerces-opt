#include "state.hh"

namespace cutie_ns {

char const *ERROR_DESCRIPTION[] = {
#define CUTIE_ERROR_DEF(e, d) d,
#include "cutie-state.txt"
#undef CUTIE_ERROR_DEF
};

}
