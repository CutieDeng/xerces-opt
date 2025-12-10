#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "array-detect-context-gcc.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

namespace {

void closeWrap(FILE *f) {
  (void) fclose(f);
}

void nothingWithFile(FILE *) {
}

}

ArrayDetectErrorCode initContextWithTmpFile(AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ctx.debug_file = fopen("/tmp/array-detect.log", "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    AD_RETURN();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithNamedFile(AD_FUNC_ARGS, char const *debug_file_path) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT2 (stderr, "set debug ostream -> %s\n", debug_file_path);
  ctx.debug_file = fopen(debug_file_path, "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    AD_RETURN();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithStderr(AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ctx.debug_file = stderr;
  ctx.debug_file_dtor = nothingWithFile;
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    AD_RETURN();
  }
} AD_FUNCTION_END

void deinitContext(AD_FUNC_ARGS) {
  ctx.debug_file_dtor(ctx.debug_file);
  deinitGccContext (AD_ARGS);
}

}
