#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-write.hh"

namespace array_detector {
  class ArrayDetector;
} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 源使用分析模块 (Source Use Analysis)
// ============================================================================
// 数据流位置：write-original-source -> (listof source-use)
// 追踪 write-original-source 的 SSA 使用链，收集所有使用点
//
// (source-use-result
//   source-operand    : tree             ; SSA_NAME
//   all-uses          : (listof use-info)
//   escape-count      : nat
//   has-escape        : bool)
// ============================================================================

// 使用类型枚举
enum SourceUseKind {
  SU_USE_LOAD,              // 读取使用
  SU_USE_STORE,             // 存储使用
  SU_USE_CALL_ARG,          // 函数调用参数
  SU_USE_RETURN,            // 返回值
  SU_USE_PHI,               // PHI 节点
  SU_USE_ASSIGN,            // 赋值
  SU_USE_ARITHMETIC,        // 算术运算
  SU_USE_COMPARISON,        // 比较运算
  SU_USE_ADDRESS_TAKEN,     // 取地址
  SU_USE_CONDITIONAL,       // 条件表达式
  SU_USE_OTHER              // 其他
};

// ============================================================================
// 逃逸类型定义
// ============================================================================

enum SourceUseEscapeKind {
  SU_ESCAPE_NONE = 0,           // 无逃逸
  SU_ESCAPE_RETURN,             // 通过返回值逃逸
  SU_ESCAPE_PARAMETER,          // 通过参数传递逃逸
  SU_ESCAPE_GLOBAL_STORE,       // 存储到全局变量
  SU_ESCAPE_HEAP_STORE,         // 存储到堆对象
  SU_ESCAPE_FIELD_STORE,        // 存储到对象字段
  SU_ESCAPE_INDIRECT_CALL,      // 通过间接调用逃逸
  SU_ESCAPE_VIRTUAL_CALL,       // 通过虚函数调用逃逸
  SU_ESCAPE_EXTERNAL_CALL,      // 传递给外部函数
  SU_ESCAPE_UNKNOWN             // 未知逃逸路径
};

// 逃逸目标信息联合体（仅当 escape_kind != SU_ESCAPE_NONE 时有效）
union EscapeTargetInfo {
  tree function_decl;             // 对于函数调用逃逸
  tree field_decl;                // 对于字段存储逃逸
  tree global_var;                // 对于全局存储逃逸
};

// ============================================================================
// 统一的使用信息结构（合并原 SourceUseInfo 和 SourceUseEscapeLocation）
// ============================================================================

struct SourceUseInfo {
  // 基本使用信息
  SourceUseKind kind;               // 使用类型
  gimple * use_stmt;                // 使用语句
  tree use_operand;                 // 使用的操作数
  location_t source_location;       // 源码位置
  unsigned int bb_index;            // 基本块索引

  // 逃逸信息（escape_kind != SU_ESCAPE_NONE 时有效）
  SourceUseEscapeKind escape_kind;  // 逃逸类型
  char const * escape_target;       // 逃逸目标描述（函数名、字段名等）
  EscapeTargetInfo target_info;     // 逃逸目标详细信息

  // 便捷方法
  inline bool is_escape() const { return escape_kind != SU_ESCAPE_NONE; }
};

// ============================================================================
// 源使用分析结果 (SourceUseResult)
// ============================================================================

struct SourceUseChainNode {
  tree ssa_name;                    // 当前 SSA 名称
  gimple * def_stmt;                // 定义语句
  vec<SourceUseInfo> * uses;        // 所有使用
  vec<SourceUseChainNode*> * next_nodes; // 下一步节点
};

struct SourceUseResult {
  tree source_operand;              // 源操作数
  gimple * source_stmt;             // 源语句

  vec<SourceUseInfo> * all_uses;    // 所有使用（包含逃逸信息）
  unsigned int total_use_count;     // 总使用次数

  unsigned int escape_count;        // 逃逸次数
  bool has_escape;                  // 是否存在逃逸
  SourceUseEscapeKind dominant_escape_kind; // 主要逃逸类型

  unsigned int max_use_depth;       // 最大使用深度
  bool is_fully_analyzed;           // 是否完全分析

  void * aux;                       // 辅助指针
  void * original_write_info;       // 原始 FieldWriteInfo*
};

// 向后兼容别名
typedef SourceUseResult SourceUseAnalysisResult;

// ============================================================================
// 分析配置常量
// ============================================================================

// 最大分析深度（SSA 使用链追踪）
constexpr unsigned int MAX_ESCAPE_ANALYSIS_DEPTH = 5;

// ============================================================================
// 核心分析接口（新模块命名）
// ============================================================================

// 主入口：分析源操作数的所有使用
// 数据流：source_operand -> SourceUseResult
ArrayDetectErrorCode analyzeSourceUse (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  gimple * exclude_stmt,
  SourceUseResult *& result
);

// 子函数：递归追踪 SSA 使用链
ArrayDetectErrorCode analyzeSourceUse_traceSSAUseChain (
  AD_FUNC_ARGS,
  tree ssa_name,
  SourceUseResult * result,
  unsigned int depth,
  gimple * exclude_stmt
);

// 子函数：分类 SSA 使用类型
ArrayDetectErrorCode analyzeSourceUse_classifyUseKind (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree ssa_name,
  SourceUseKind & kind
);

// 子函数：检测逃逸类型
ArrayDetectErrorCode analyzeSourceUse_detectEscapeKind (
  AD_FUNC_ARGS,
  SourceUseInfo const & use_info,
  SourceUseEscapeKind & escape_kind
);

// 子函数：判断函数是否为外部函数
ArrayDetectErrorCode analyzeSourceUse_isFunctionExternal (
  AD_FUNC_ARGS,
  tree function_decl,
  bool & result
);

// 子函数：记录使用点
ArrayDetectErrorCode analyzeSourceUse_recordUsePoint (
  AD_FUNC_ARGS,
  gimple * use_stmt,
  tree use_operand,
  SourceUseKind use_kind,
  SourceUseEscapeKind escape_kind,
  char const * escape_target,
  EscapeTargetInfo const & target_info,
  SourceUseResult * result
);

// ============================================================================
// Pipeline 接口
// ============================================================================

// 收集所有字段的逃逸信息
// 输入：detector - 包含 m_type_field_writes 的检测器
// 输出：total_analyzed - 总收集数量
//       total_escaped - 总逃逸数量
ArrayDetectErrorCode collectAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_analyzed,
  unsigned int &total_escaped
);

// ============================================================================
// 向后兼容接口
// ============================================================================

// 收集单个源操作数的使用信息（别名）
inline ArrayDetectErrorCode collectSourceOperandEscapes (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  gimple * exclude_stmt,
  SourceUseResult * &result
) {
  return analyzeSourceUse (AD_ARGS, source_operand, source_stmt, exclude_stmt, result);
}

// 从字段写入中收集源使用信息
// 输入：write_info - 字段写入信息
// 输出：result - SourceUseResult 指针（GC 管理）
ArrayDetectErrorCode collectFieldWriteEscapes (
  AD_FUNC_ARGS,
  FieldWriteInfo * write_info,
  SourceUseResult * &result
);

// ============================================================================
// 辅助函数
// ============================================================================

// 获取逃逸类型描述字符串
char const * getEscapeKindString (SourceUseEscapeKind kind);

// 获取使用类型描述字符串
char const * getUseKindString (SourceUseKind kind);

// 打印分析结果（调试用）
void printSourceUseResult (
  SourceUseResult const *result,
  FILE *output
);

// 向后兼容别名
inline void printSourceUseAnalysisResult (
  SourceUseResult const *result,
  FILE *output
) { printSourceUseResult(result, output); }

} // namespace array_detect_ns
