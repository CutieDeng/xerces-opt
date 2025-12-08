#pragma once

// 轻量级接口头文件，避免复杂依赖
// 只包含必要的函数声明，不包含复杂的数据结构

namespace cutie_ns {

// 前向声明复杂类型
struct CutieContextGcc;

namespace controlflow {

void pushStackFrame(CUTIE_FUNC_ARGS, uint64_t frame_id);
void popStackFrame(CUTIE_FUNC_ARGS);

} // namespace controlflow

} // namespace cutie_ns
