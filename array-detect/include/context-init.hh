#include "prelude.hh"
#include "context.hh"

namespace cutie_ns {

CutieErrorCode initWithTmpFile(CUTIE_FUNC_ARGS);
CutieErrorCode initWithNamedFile(CUTIE_FUNC_ARGS, char const *debug_file_path);
CutieErrorCode initWithStderr(CUTIE_FUNC_ARGS);
void deinit(CUTIE_FUNC_ARGS);

} // namespace cutie_ns
