#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detector {

  class ArrayDetector;

} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 源逃逸收集模块 (Source Escape Collection)
// ============================================================================
// 收集字段写入操作中源操作数的逃逸信息
// 追踪 SSA 使用链，收集所有逃逸位置的详尽信息供后续综合分析
// ============================================================================

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

// 逃逸使用位置信息
struct SourceUseEscapeLocation {
  SourceUseEscapeKind kind;
  gimple * stmt;                    // 逃逸发生的语句
  tree use_operand;                 // 使用的操作数
  char const * escape_target;        // 逃逸目标描述
  location_t source_location;       // 源码位置
  unsigned int bb_index;            // 基本块索引

  union {
    tree function_decl;             // 对于函数调用
    tree field_decl;                // 对于字段存储
    tree global_var;                // 对于全局存储
  } target_info;
};

// ============================================================================
// 使用类型定义
// ============================================================================

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

// 单个使用信息
struct SourceUseInfo {
  SourceUseKind kind;
  gimple * use_stmt;                 // 使用语句
  tree use_operand;                 // 使用的操作数
  location_t source_location;       // 源码位置
  unsigned int bb_index;            // 基本块索引
  bool is_escape;                   // 是否逃逸
  SourceUseEscapeKind escape_kind;  // 逃逸类型
};

// ============================================================================
// 分析结果
// ============================================================================

struct SourceUseChainNode {
  tree ssa_name;                    // 当前 SSA 名称
  gimple * def_stmt;                // 定义语句
  vec<SourceUseInfo> * uses;        // 所有使用
  vec<SourceUseChainNode*> * next_nodes; // 下一步节点
};

struct SourceUseAnalysisResult {
  tree source_operand;              // 源操作数
  gimple * source_stmt;             // 源语句

  vec<SourceUseInfo> * all_uses;    // 所有使用
  unsigned int total_use_count;     // 总使用次数

  vec<SourceUseEscapeLocation> * escape_locations; // 逃逸位置
  unsigned int escape_count;        // 逃逸次数
  bool has_escape;                  // 是否存在逃逸
  SourceUseEscapeKind dominant_escape_kind; // 主要逃逸类型

  SourceUseChainNode * use_chain_root; // 使用链根节点

  unsigned int max_use_depth;       // 最大使用深度
  bool is_fully_analyzed;           // 是否完全分析

  void * aux;                       // 异构链表字段
  void * original_write_info;       // 原始写入信息
};

// ============================================================================
// 分析配置常量
// ============================================================================

// 最大分析深度（SSA 使用链追踪）
constexpr unsigned int MAX_ESCAPE_ANALYSIS_DEPTH = 5;

// ============================================================================
// 核心收集接口
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

// 收集单个源操作数的逃逸信息
// 输入：source_operand - 源操作数（SSA_NAME）
//       source_stmt - 源语句
// 输出：result - 收集结果指针（GC 管理）
// 注：采用全量逃逸检测策略，所有可能的逃逸情况均被检测
ArrayDetectErrorCode collectSourceOperandEscapes (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * source_stmt,
  SourceUseAnalysisResult * &result
);

// 从字段写入中收集逃逸信息
// 输入：write_capture - 字段写入捕获
// 输出：result - 收集结果指针（GC 管理）
// 注：采用全量逃逸检测策略，所有可能的逃逸情况均被检测
ArrayDetectErrorCode collectFieldWriteEscapes (
  AD_FUNC_ARGS,
  FieldWriteCapture * write_capture,
  SourceUseAnalysisResult * &result
);

// ============================================================================
// 辅助函数
// ============================================================================

// NOTE: isEscapeUse is not used and does not follow standard API - commented out
/*
// 判断使用是否为逃逸
bool isEscapeUse (
  SourceUseInfo const &use_info,
  SourceUseEscapeRules const &rules
);
*/

// 获取逃逸类型描述字符串
char const * getEscapeKindString (SourceUseEscapeKind kind);

// 获取使用类型描述字符串
char const * getUseKindString (SourceUseKind kind);

// 打印分析结果（调试用）
void printSourceUseAnalysisResult (
  SourceUseAnalysisResult const *result,
  FILE *output
);

} // namespace array_detect_ns
