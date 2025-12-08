#pragma once

// 轻量级接口头文件，避免复杂依赖
// 只包含必要的函数声明，不包含复杂的数据结构

namespace cutie_ns {

// 前向声明复杂类型
struct CutieContextGcc;

// 公开的 GCC context 管理函数
::cutie_ns::CutieErrorCode initCutieContextGcc(CUTIE_FUNC_ARGS);
void deinitCutieContextGcc(CUTIE_FUNC_ARGS);

namespace controlflow {

void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id);
void popStackFrame(CUTIE_FUNC_ARGS);

} // namespace controlflow

bool isStackEmpty(CUTIE_FUNC_ARGS);

// 增强调试函数（带源码位置信息）
void printStackFramesWithSource(CUTIE_FUNC_ARGS);
void printStackFrameSource(CUTIE_FUNC_ARGS, uint64_t frame_addr);
bool getFrameSourceLocation(CUTIE_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size);

} // namespace cutie_ns
