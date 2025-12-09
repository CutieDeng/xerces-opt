#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc.hh"

namespace array_detect_ns {

ArrayDetectErrorCode initWithTmpFile(AD_FUNC_ARGS);
ArrayDetectErrorCode initWithNamedFile(AD_FUNC_ARGS, char const *debug_file_path);
ArrayDetectErrorCode initWithStderr(AD_FUNC_ARGS);
void deinit(AD_FUNC_ARGS);

ArrayDetectErrorCode initCapacityImpl(AD_FUNC_ARGS, size_t capacity);

} // namespace array_detect_ns
