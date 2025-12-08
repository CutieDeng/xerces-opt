#pragma once

// 轻量级接口头文件，避免复杂依赖
// 只包含必要的函数声明，不包含复杂的数据结构

namespace cutie_ns {

// 前向声明复杂类型
struct CutieContextGcc;

// 公开的 GCC context 管理函数
::cutie_ns::CutieErrorCode init_cutie_context_gcc(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx);
void deinit_cutie_context_gcc(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx);
bool is_cutie_context_gcc_initialized();

// 安全访问函数
CutieContextGcc& get_cutie_context_gcc_safe();

// 基础栈帧管理函数（如果其他模块需要）
void push_stack_frame(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx, uint64_t frame_id);
void pop_stack_frame(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx);
bool is_stack_empty(::cutie_ns::CutieContext& ctx, ::cutie_ns::CutieContextGcc& gcc_ctx);

} // namespace cutie_ns