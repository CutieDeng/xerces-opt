#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "context-init.hh"
#include "array-detect-context-gcc.hh"
#include "array-detect-context-gcc-interface.hh"

#include <cstdlib>  // for getenv
#include <cstring>  // for strcmp

namespace array_detect_ns {

namespace {

void closeWrap (FILE *f) {
  (void) fclose (f);
}

void nothingWithFile (FILE *) {
}

}

ArrayDetectErrorCode initContextWithTmpFile (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ctx.debug_file = fopen ("/tmp/array-detect.log", "w");
  ctx.debug_file_dtor = closeWrap;
  ctx.match_debug_tracer = false;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE_RAW (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    fclose (ctx.debug_file);
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithNamedFile (AD_FUNC_ARGS, char const *debug_file_path) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT2 (stderr, "set debug ostream -> %s\n", debug_file_path);
  ctx.debug_file = fopen (debug_file_path, "w");
  ctx.debug_file_dtor = closeWrap;
  ctx.match_debug_tracer = false;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE_RAW (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    fclose (ctx.debug_file);
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithStderr (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  ctx.debug_file = stderr;
  ctx.debug_file_dtor = nothingWithFile;
  ctx.match_debug_tracer = false;
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    fclose (ctx.debug_file);
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextAdaptive (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  char const *debug_file_env = getenv ("AD_DEBUG_FILE");
  char const *result_file_env = getenv ("AD_RESULT_FILE");

  // 初始化结果文件路径（如果设置了 AD_RESULT_FILE 环境变量）
  ctx.result_file_path = result_file_env;  // 直接使用环境变量字符串（生命周期足够长）

  if (debug_file_env == nullptr) {
    // 环境变量未设置，禁用调试输出
    ctx.debug_file = nullptr;
    ctx.debug_file_dtor = nothingWithFile;
    ctx.match_debug_tracer = false;
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  } else if (strcmp (debug_file_env, "stderr") == 0) {
    // 使用标准错误输出
    ctx.debug_file = stderr;
    ctx.debug_file_dtor = nothingWithFile;
    ctx.match_debug_tracer = false;
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  } else {
    // 打开指定文件
    AD_DEBUG_PRINT2 (stderr, "set debug ostream -> %s (from AD_DEBUG_FILE env)\n", debug_file_env);
    ctx.debug_file = fopen (debug_file_env, "w");
    ctx.debug_file_dtor = closeWrap;
    ctx.match_debug_tracer = false;
    if (ctx.debug_file == nullptr) {
      AD_RETURNE_RAW (RESOURCE_ERROR);
    }
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  }

  if (false) {
    fail0:
    if (ctx.debug_file && ctx.debug_file != stderr) {
      fclose (ctx.debug_file);
    }
    AD_RETURN ();
  }
} AD_FUNCTION_END

void deinitContext (AD_FUNC_ARGS) {
  ctx.debug_file_dtor (ctx.debug_file);
  deinitGccContext (AD_ARGS);
}

}
