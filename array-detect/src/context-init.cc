#include "prelude.hh"
#include "state.hh"
#include "context.hh"

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
    ecode = RESOURCE_ERROR;
    CUTIE_RETURN;
  } else {
    ecode = OK;
    CUTIE_RETURN;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithNamedFile(CUTIE_FUNC_ARGS, char const *debug_file_path) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT_RAW (stderr, "set debug ostream -> %s\n", debug_file_path);
  ctx.debug_file = fopen(debug_file_path, "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    ecode = RESOURCE_ERROR;
    CUTIE_RETURN;
  } else {
    ecode = OK;
    CUTIE_RETURN;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithStderr(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  ctx.debug_file = stderr;
  ctx.debug_file_dtor = nothingWithFile;
  ecode = OK;
  CUTIE_RETURN;
} CUTIE_FUNCTION_END

void deinit(CUTIE_FUNC_ARGS) {
  ctx.debug_file_dtor(ctx.debug_file);
}

}
