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

// ============================================================================
// 初始化环境变量相关配置
// ============================================================================

void initContextEnvVars (AD_FUNC_ARGS) {
  (void)gcc_ctx;

  // 读取 AD_RESULT_FILE 环境变量
  char const *result_file_env = getenv ("AD_RESULT_FILE");
  ctx.result_file_path = result_file_env;

  // current_input_file 需要从 GCC 获取，在 initContextBuffers 中设置
  ctx.current_input_file = nullptr;
}

// ============================================================================
// Context 初始化函数
// ============================================================================

ArrayDetectErrorCode initContextWithTmpFile (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  initContextEnvVars (AD_ARGS);
  if (!g_array_detect_ctx.debug_file) {
    g_array_detect_ctx.debug_file = fopen ("/tmp/array-detect.log", "w");
    g_array_detect_ctx.debug_file_dtor = g_array_detect_ctx.debug_file ? closeWrap : nothingWithFile;
  }
  ctx.debug_file = g_array_detect_ctx.debug_file;
  ctx.debug_file_dtor = nothingWithFile;
  ctx.match_debug_tracer = false;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE_RAW (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithNamedFile (AD_FUNC_ARGS, char const *debug_file_path) AD_FUNCTION_BEGIN {
  initContextEnvVars (AD_ARGS);
  AD_DEBUG_PRINT2 (stderr, "set debug ostream -> %s\n", debug_file_path);
  if (!g_array_detect_ctx.debug_file) {
    g_array_detect_ctx.debug_file = fopen (debug_file_path, "w");
    g_array_detect_ctx.debug_file_dtor = g_array_detect_ctx.debug_file ? closeWrap : nothingWithFile;
  }
  ctx.debug_file = g_array_detect_ctx.debug_file;
  ctx.debug_file_dtor = nothingWithFile;
  ctx.match_debug_tracer = false;
  if (ctx.debug_file == nullptr) {
    AD_RETURNE_RAW (RESOURCE_ERROR);
  }
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextWithStderr (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  initContextEnvVars (AD_ARGS);
  if (!g_array_detect_ctx.debug_file) {
    g_array_detect_ctx.debug_file = stderr;
    g_array_detect_ctx.debug_file_dtor = nothingWithFile;
  }
  ctx.debug_file = g_array_detect_ctx.debug_file;
  ctx.debug_file_dtor = nothingWithFile;
  ctx.match_debug_tracer = false;
  AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
  AD_RETURNE_RAW (OK);
  if (false) {
    fail0:
    AD_RETURN ();
  }
} AD_FUNCTION_END

ArrayDetectErrorCode initContextAdaptive (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  initContextEnvVars (AD_ARGS);
  char const *debug_file_env = getenv ("AD_DEBUG_FILE");

  if (debug_file_env == nullptr) {
    // 环境变量未设置，禁用调试输出
    ctx.debug_file = nullptr;
    ctx.debug_file_dtor = nothingWithFile;
    ctx.match_debug_tracer = false;
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  } else if (strcmp (debug_file_env, "stderr") == 0) {
    // 使用标准错误输出
    ctx.debug_file = g_array_detect_ctx.debug_file;
    ctx.debug_file_dtor = nothingWithFile;
    ctx.match_debug_tracer = false;
    if (ctx.debug_file == nullptr) {
      AD_RETURNE_RAW (RESOURCE_ERROR);
    }
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  } else {
    // 打开指定文件
    AD_DEBUG_PRINT2 (stderr, "set debug ostream -> %s (from AD_DEBUG_FILE env)\n", debug_file_env);
    ctx.debug_file = g_array_detect_ctx.debug_file;
    ctx.debug_file_dtor = nothingWithFile;
    ctx.match_debug_tracer = false;
    if (ctx.debug_file == nullptr) {
      AD_RETURNE_RAW (RESOURCE_ERROR);
    }
    AD_TRY_LABEL (initContextBuffers (AD_ARGS, 512), fail0);
    AD_RETURNE_RAW (OK);
  }

  if (false) {
    fail0:
    AD_RETURN ();
  }
} AD_FUNCTION_END

void deinitContext (AD_FUNC_ARGS) {
  ctx.debug_file_dtor (ctx.debug_file);
  deinitGccContext (AD_ARGS);
}

void initGlobalDebugFileFromEnv () {
  char const *debug_file_env = getenv ("AD_DEBUG_FILE");
  if (debug_file_env == nullptr) {
    g_array_detect_ctx.debug_file = nullptr;
    g_array_detect_ctx.debug_file_dtor = nothingWithFile;
    return;
  }

  if (strcmp (debug_file_env, "stderr") == 0) {
    g_array_detect_ctx.debug_file = stderr;
    g_array_detect_ctx.debug_file_dtor = nothingWithFile;
    return;
  }

  g_array_detect_ctx.debug_file = fopen (debug_file_env, "w");
  g_array_detect_ctx.debug_file_dtor = g_array_detect_ctx.debug_file ? closeWrap : nothingWithFile;
}

void closeGlobalDebugFile () {
  if (!g_array_detect_ctx.debug_file) return;
  g_array_detect_ctx.debug_file_dtor (g_array_detect_ctx.debug_file);
  g_array_detect_ctx.debug_file = nullptr;
  g_array_detect_ctx.debug_file_dtor = nothingWithFile;
}

}
