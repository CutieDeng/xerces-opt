// 示例：演示如何启用自动栈帧管理
// 在文件开头定义 CUTIE_ENABLE_AUTO_STACK_FRAME 即可启用自动功能

#define CUTIE_ENABLE_AUTO_STACK_FRAME

#include "prelude.hh"
#include "cutie-context-gcc-interface.h"

// 示例函数 - 将自动管理栈帧
CutieErrorCode exampleFunctionWithAutoStack(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Inside exampleFunctionWithAutoStack");

  // 使用自动调用栈转储
  CUTIE_AUTO_DUMP_CALL_STACK();

  // 使用栈帧深度断言
  CUTIE_ASSERT_STACK_DEPTH(1); // 应该是深度1（当前函数）

  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END

// 手动栈帧控制的示例
CutieErrorCode exampleFunctionWithManualStack(CUTIE_FUNC_ARGS) CUTIE_FUNCTION_BEGIN {
  // 使用手动栈帧管理
  CUTIE_GCC_ENTER_FUNCTION((uint64_t)__builtin_return_address(0));

  CUTIE_DEBUG_PRINT("Inside exampleFunctionWithManualStack");

  // 手动调用栈转储
  CUTIE_GCC_DUMP_CALL_STACK();

  CUTIE_GCC_EXIT_FUNCTION();
  CUTIE_RETURNV(OK);
} CUTIE_FUNCTION_END