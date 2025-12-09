// 示例：演示如何启用自动栈帧管理
// 在文件开头定义 AD_ENABLE_AUTO_STACK_FRAME 即可启用自动功能

#define AD_ENABLE_AUTO_STACK_FRAME

#include "prelude.hh"
#include "array-detect-context-gcc-interface.h"

// 示例函数 - 将自动管理栈帧
ArrayDetectErrorCode exampleFunctionWithAutoStack(AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT("Inside exampleFunctionWithAutoStack");

  // 使用自动调用栈转储
  AD_AUTO_DUMP_CALL_STACK();

  // 使用栈帧深度断言
  AD_ASSERT_STACK_DEPTH(1); // 应该是深度1（当前函数）

  AD_RETURNV(OK);
} AD_FUNCTION_END

// 手动栈帧控制的示例
ArrayDetectErrorCode exampleFunctionWithManualStack(AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  // 使用手动栈帧管理
  AD_GCC_ENTER_FUNCTION((uint64_t)__builtin_return_address(0));

  AD_DEBUG_PRINT("Inside exampleFunctionWithManualStack");

  // 手动调用栈转储
  AD_GCC_DUMP_CALL_STACK();

  AD_GCC_EXIT_FUNCTION();
  AD_RETURNV(OK);
} AD_FUNCTION_END