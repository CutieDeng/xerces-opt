#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "cutie-context-gcc.hh"

namespace cutie_ns {

namespace {

void closeWrap(FILE *f) {
  (void) fclose(f);
}

void nothingWithFile(FILE *) {
}

}

CutieErrorCode initWithTmpFile(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  ctx.debug_file = fopen("/tmp/array-detect.log", "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    CUTIE_RETURNV (RESOURCE_ERROR);
  }
  CUTIE_TRY_LABEL (initCutieContextGcc (CUTIE_ARGS), fail0);
  CUTIE_TRY_LABEL (initCapacityImpl (CUTIE_ARGS, 512), fail0);
  CUTIE_RETURNV (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    CUTIE_RETURNR;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithNamedFile(CUTIE_FUNC_ARGS, char const *debug_file_path) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT2 (stderr, "set debug ostream -> %s\n", debug_file_path);
  ctx.debug_file = fopen(debug_file_path, "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    CUTIE_RETURNV (RESOURCE_ERROR);
  }
  CUTIE_TRY_LABEL (initCutieContextGcc (CUTIE_ARGS), fail0);
  CUTIE_TRY_LABEL (initCapacityImpl (CUTIE_ARGS, 512), fail0);
  CUTIE_RETURNV (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    CUTIE_RETURNR;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithStderr(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  ctx.debug_file = stderr;
  ctx.debug_file_dtor = nothingWithFile;
  CUTIE_TRY_LABEL (initCutieContextGcc (CUTIE_ARGS), fail0);
  CUTIE_TRY_LABEL (initCapacityImpl (CUTIE_ARGS, 512), fail0);
  CUTIE_RETURNV (OK);
  if (false) {
    fail0:
    fclose(ctx.debug_file);
    CUTIE_RETURNR;
  }
} CUTIE_FUNCTION_END

void deinit(CUTIE_FUNC_ARGS) {
  ctx.debug_file_dtor(ctx.debug_file);
  deinitCutieContextGcc (CUTIE_ARGS);
}

}
