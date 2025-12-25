#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"
#include "source-escape-collection.hh"

namespace array_detector {

  class ArrayDetector;

} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 逃逸综合分析模块 (Escape Synthesizer)
// ============================================================================
// 对源操作数逃逸收集结果进行综合分析，分类为不同的逃逸情形
// 支持组合情况，不遗漏任何有效信息
// ============================================================================

// ============================================================================
// 逃逸综合类别定义（使用位图支持组合）
// ============================================================================

enum EscapeSynthesisCategory {
  ESC_SYNTH_NONE                = 0,       // 无分类（初始状态）
  ESC_SYNTH_NO_ESCAPE           = 1 << 0,  // 无逃逸（单个 field access）
  ESC_SYNTH_ARITHMETIC_POTENTIAL= 1 << 1,  // 未证明的算术运算潜在逃逸
  ESC_SYNTH_SAFE_DEBUG          = 1 << 2,  // 安全调试型逃逸（printf 等）
  ESC_SYNTH_UNKNOWN_CALL        = 1 << 3,  // 未知函数逃逸
  ESC_SYNTH_HEAP_ESCAPE         = 1 << 4,  // 堆逃逸
  ESC_SYNTH_RETURN_ESCAPE       = 1 << 5,  // 返回值逃逸
  ESC_SYNTH_PARAMETER_ESCAPE    = 1 << 6,  // 参数逃逸
  ESC_SYNTH_GLOBAL_ESCAPE       = 1 << 7,  // 全局变量逃逸
  ESC_SYNTH_FIELD_ESCAPE        = 1 << 8,  // 字段逃逸（多个 field access）
  ESC_SYNTH_VIRTUAL_CALL        = 1 << 9,  // 虚函数调用逃逸
  ESC_SYNTH_INDIRECT_CALL       = 1 << 10, // 间接调用逃逸
};

// 类别检查宏
#define ESC_SYNTH_HAS(result, category) (((result).category_bitmap & (category)) != 0)
#define ESC_SYNTH_ADD(result, category) ((result).category_bitmap |= (category))
#define ESC_SYNTH_REMOVE(result, category) ((result).category_bitmap &= ~(category))

// ============================================================================
// 各类别详细信息结构
// ============================================================================

// 无逃逸情况详细信息
struct NoEscapeInfo {
  tree field_access_expr;           // 字段访问表达式（来自原始源）
  location_t location;              // 位置
  char const * description;         // 描述
};

// 算术运算潜在逃逸信息
struct ArithmeticPotentialEscape {
  vec<gimple *, va_gc> * arithmetic_stmts; // 算术语句列表
  vec<char const *, va_gc> * operations;   // 运算类型（+, -, *, /, &, |, ^, <<, >>）
  unsigned int count;               // 数量
};

// 安全调试型逃逸信息
struct SafeDebugEscape {
  vec<gimple *, va_gc> * debug_calls;      // 调试函数调用列表
  vec<char const *, va_gc> * function_names; // 函数名列表
  unsigned int count;               // 数量
};

// 未知函数逃逸信息
struct UnknownCallEscape {
  vec<gimple *, va_gc> * unknown_calls;    // 未知函数调用列表
  vec<char const *, va_gc> * call_descriptions; // 调用描述
  unsigned int count;               // 数量
};

// 堆逃逸信息
struct HeapEscapeInfo {
  vec<gimple *, va_gc> * heap_stores;      // 堆存储语句
  vec<tree, va_gc> * stored_values;        // 存储的值
  unsigned int count;               // 数量
};

// 字段逃逸信息（多个 field access）
struct FieldEscapeInfo {
  vec<gimple *, va_gc> * field_stores;     // 字段存储语句
  vec<tree, va_gc> * field_decls;          // 字段声明列表
  vec<char const *, va_gc> * field_names;  // 字段名列表
  unsigned int count;               // 数量
};

// 直接逃逸信息（返回值/参数/全局变量）
struct DirectEscapeInfo {
  vec<gimple *, va_gc> * escape_stmts;     // 逃逸语句
  vec<char const *, va_gc> * targets;      // 目标描述
  unsigned int count;               // 数量
};

// 调用逃逸信息（虚函数/间接调用）
struct CallEscapeInfo {
  vec<gimple *, va_gc> * call_stmts;       // 调用语句
  vec<char const *, va_gc> * call_names;   // 调用名称
  unsigned int count;               // 数量
};

// ============================================================================
// 逃逸综合结果
// ============================================================================

struct EscapeSynthesisResult {
  // 类别位图（可组合）
  unsigned int category_bitmap;

  // 各类别详细信息（只在对应位被设置时有效，NULL 表示未设置）
  NoEscapeInfo * no_escape;
  ArithmeticPotentialEscape * arithmetic_potential;
  SafeDebugEscape * safe_debug;
  UnknownCallEscape * unknown_call;
  HeapEscapeInfo * heap_escape;
  FieldEscapeInfo * field_escape;
  DirectEscapeInfo * return_escape;
  DirectEscapeInfo * parameter_escape;
  DirectEscapeInfo * global_escape;
  CallEscapeInfo * virtual_call;
  CallEscapeInfo * indirect_call;

  // 统计信息
  unsigned int total_uses;          // 总使用次数
  unsigned int total_escapes;       // 总逃逸次数
  unsigned int category_count;      // 类别数量

  // 原始逃逸收集结果（引用）
  SourceUseAnalysisResult * raw_result;

  // 链表字段
  void * aux;
  void * original_write_info;       // 原始写入信息（FieldWriteCapture*）
};

// ============================================================================
// 核心综合接口
// ============================================================================

// 综合所有字段的逃逸信息
// 输入：detector - 包含逃逸收集结果的检测器
// 输出：synthesis_results - 综合结果列表（vec<EscapeSynthesisResult*>*）
//       total_synthesized - 总综合数量
ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<EscapeSynthesisResult*> * &synthesis_results,
  unsigned int &total_synthesized
);

// 综合单个源操作数的逃逸信息
// 输入：raw_result - 原始逃逸收集结果
// 输出：result - 综合结果指针（GC 管理）
ArrayDetectErrorCode synthesizeEscapeInfo (
  AD_FUNC_ARGS,
  SourceUseAnalysisResult * raw_result,
  EscapeSynthesisResult * &result
);

// ============================================================================
// 辅助分类函数
// ============================================================================

// 判断是否为已知安全调试函数
bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
);

// 判断是否为算术运算
bool isArithmeticOperation (
  gimple * stmt,
  char const * &operation_name
);

// 分类单个逃逸位置
ArrayDetectErrorCode classifyEscapeLocation (
  AD_FUNC_ARGS,
  SourceUseEscapeLocation const &escape_loc,
  EscapeSynthesisResult * result
);

// 获取类别描述字符串
char const * getEscapeCategoryString (unsigned int category);

// 打印综合结果（调试用）
void printEscapeSynthesisResult (
  EscapeSynthesisResult const * result,
  FILE * output
);

// ============================================================================
// 二级综合器：所有权分析 (Ownership Analysis)
// ============================================================================
// 基于逃逸综合结果，判定字段是否支持 owned 指针的假设
// 默认假设：每个字段都有潜在可能是 owned 的指针字段
// 通过观察控制流现象来决定是否"不支持"该结论
// ============================================================================

// 所有权支持结论
enum OwnershipSupportVerdict {
  OWNERSHIP_VERDICT_SUPPORTED,      // 支持 owned（没有反对证据）
  OWNERSHIP_VERDICT_REJECTED,       // 不支持 owned（有反对证据）
  OWNERSHIP_VERDICT_UNCERTAIN,      // 不确定（需要更多分析）
};

// 反对原因位图（哪些逃逸类别导致了反对 owned）
enum OwnershipRejectionReason {
  REJECT_NONE                = 0,
  REJECT_HEAP_ESCAPE         = 1 << 0,  // 逃逸到堆（可能被共享）
  REJECT_RETURN_ESCAPE       = 1 << 1,  // 返回值逃逸（所有权转移）
  REJECT_PARAMETER_ESCAPE    = 1 << 2,  // 参数逃逸（可能被共享）
  REJECT_GLOBAL_ESCAPE       = 1 << 3,  // 全局变量逃逸（可能被共享）
  REJECT_VIRTUAL_CALL        = 1 << 4,  // 虚函数调用（不确定行为）
  REJECT_INDIRECT_CALL       = 1 << 5,  // 间接调用（不确定行为）
  REJECT_UNKNOWN_CALL        = 1 << 6,  // 未知函数调用（不确定行为）
  REJECT_FIELD_ESCAPE        = 1 << 7,  // 字段逃逸（多个 field access，可能被共享）
};

// 所有权分析结果（针对单个 type, field）
struct OwnershipAnalysisResult {
  // (type, field) 标识
  tree type;
  tree field_decl;
  char const * type_name;
  char const * field_name;

  // 结论
  OwnershipSupportVerdict verdict;

  // 证据统计
  unsigned int total_writes;          // 总写入次数
  unsigned int supporting_writes;     // 支持 owned 的写入（无反对证据）
  unsigned int rejecting_writes;      // 反对 owned 的写入（有反对证据）
  unsigned int uncertain_writes;      // 不确定的写入

  // 反对原因（位图）
  unsigned int rejection_reasons;     // 哪些类别导致了反对

  // 详细信息
  vec<EscapeSynthesisResult*> * all_write_results;  // 所有写入操作的综合结果

  // 结论描述
  char const * verdict_description;
};

// 检查宏
#define OWNERSHIP_HAS_REJECTION(result, reason) (((result).rejection_reasons & (reason)) != 0)

// ============================================================================
// 二级综合器核心接口
// ============================================================================

// 分析单个 (type, field) 的所有权支持情况
// 输入：write_results - 该 (type, field) 的所有写入操作的综合结果
//       type, field_decl - 类型和字段标识
// 输出：result - 所有权分析结果
ArrayDetectErrorCode analyzeFieldOwnershipSupport (
  AD_FUNC_ARGS,
  vec<EscapeSynthesisResult*> * write_results,
  tree type,
  tree field_decl,
  OwnershipAnalysisResult * &result
);

// 判断单个逃逸综合结果是否反对 owned
// 返回：true - 反对，false - 不反对
bool isEscapeResultRejectingOwnership (
  EscapeSynthesisResult const * synth_result,
  unsigned int &rejection_reasons
);

// 获取结论描述字符串
char const * getOwnershipVerdictString (OwnershipSupportVerdict verdict);

// 获取反对原因描述字符串
char const * getOwnershipRejectionReasonString (unsigned int reason);

// 打印所有权分析结果（调试用）
void printOwnershipAnalysisResult (
  OwnershipAnalysisResult const * result,
  FILE * output
);

// 打印字段的所有写入操作的逃逸综合结果（调试用）
void printFieldEscapeSynthesisResults (
  AD_FUNC_ARGS,
  tree type,
  tree field_decl,
  vec<EscapeSynthesisResult*> * write_results,
  FILE * output
);

} // namespace array_detect_ns
