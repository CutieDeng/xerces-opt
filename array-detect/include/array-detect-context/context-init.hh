#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace cutie_ns {

CutieErrorCode initWithTmpFile(CUTIE_FUNC_ARGS);
CutieErrorCode initWithNamedFile(CUTIE_FUNC_ARGS, char const *debug_file_path);
CutieErrorCode initWithStderr(CUTIE_FUNC_ARGS);
void deinit(CUTIE_FUNC_ARGS);

CutieErrorCode initCapacityImpl(CUTIE_FUNC_ARGS, size_t capacity);

} // namespace cutie_ns
