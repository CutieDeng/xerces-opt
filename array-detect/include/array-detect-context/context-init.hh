#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "array-detect-context-gcc.hh"

namespace array_detect_ns {

// 使用临时文件初始化上下文
ArrayDetectErrorCode initContextWithTmpFile (AD_FUNC_ARGS);
// 使用指定文件路径初始化上下文
ArrayDetectErrorCode initContextWithNamedFile (AD_FUNC_ARGS, char const *debug_file_path);
// 使用标准错误输出初始化上下文
ArrayDetectErrorCode initContextWithStderr (AD_FUNC_ARGS);
// 清理上下文资源
void deinitContext (AD_FUNC_ARGS);

// 初始化上下文内部缓冲区（内部实现函数）
ArrayDetectErrorCode initContextBuffers (AD_FUNC_ARGS, size_t capacity);

} // namespace array_detect_ns
