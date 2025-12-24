#pragma once

#include "gcc-plugin.h"
#include "tree.h"
#include "gimple.h"
#include "vec.h"

namespace array_detect_ns {

// ============================================================================
// 逃逸类型定义
// ============================================================================

enum SourceUseEscapeKind {
  ESCAPE_NONE = 0,           // 无逃逸
  ESCAPE_RETURN,             // 通过返回值逃逸
  ESCAPE_PARAMETER,          // 通过参数传递逃逸（传递给其他函数）
  ESCAPE_GLOBAL_STORE,       // 存储到全局变量
  ESCAPE_HEAP_STORE,         // 存储到堆对象
  ESCAPE_FIELD_STORE,        // 存储到对象字段
  ESCAPE_INDIRECT_CALL,      // 通过间接调用逃逸（函数指针）
  ESCAPE_VIRTUAL_CALL,       // 通过虚函数调用逃逸
  ESCAPE_EXTERNAL_CALL,      // 传递给外部函数（非内联）
  ESCAPE_UNKNOWN             // 未知逃逸路径
};

// 逃逸使用位置信息
struct SourceUseEscapeLocation {
  SourceUseEscapeKind kind;
  gimple* stmt;                    // 逃逸发生的语句
  tree use_operand;                // 使用的操作数
  const char* escape_target;       // 逃逸目标描述（函数名、字段名等）
  location_t source_location;      // 源码位置
  unsigned int bb_index;           // 基本块索引

  // 辅助信息
  union {
    tree function_decl;            // 对于函数调用：被调用的函数
    tree field_decl;               // 对于字段存储：目标字段
    tree global_var;               // 对于全局存储：全局变量
  } target_info;
};

// ============================================================================
// 使用类型定义
// ============================================================================

enum SourceUseKind {
  USE_LOAD,              // 读取使用（右值）
  USE_STORE,             // 存储使用（作为右值存储到其他位置）
  USE_CALL_ARG,          // 作为函数调用参数
  USE_RETURN,            // 作为返回值
  USE_PHI,               // PHI 节点使用
  USE_ASSIGN,            // 赋值使用
  USE_ARITHMETIC,        // 算术运算使用
  USE_COMPARISON,        // 比较运算使用
  USE_ADDRESS_TAKEN,     // 取地址使用
  USE_CONDITIONAL,       // 条件表达式使用
  USE_OTHER              // 其他使用
};

// 单个使用信息
struct SourceUseInfo {
  SourceUseKind kind;
  gimple* use_stmt;                // 使用语句
  tree use_operand;                // 使用的操作数（SSA_NAME 或其他）
  location_t source_location;      // 源码位置
  unsigned int bb_index;           // 基本块索引
  bool is_escape;                  // 是否为逃逸使用
  SourceUseEscapeKind escape_kind; // 如果逃逸，逃逸类型
};

// ============================================================================
// 源操作数使用分析结果
// ============================================================================

// 使用链信息（追踪数据流）
struct SourceUseChainNode {
  tree ssa_name;                   // 当前 SSA 名称
  gimple* def_stmt;                // 定义语句
  vec<SourceUseInfo>* uses;        // 该节点的所有使用
  vec<SourceUseChainNode*>* next_nodes; // 数据流下一步节点
};

// 顶层分析结果（带 aux 字段用于异构链表）
struct SourceUseAnalysisResult {
  // ========== 核心数据 ==========
  tree source_operand;             // 源操作数（write 的右值）
  gimple* source_stmt;             // 源操作数的定义语句

  // 使用信息
  vec<SourceUseInfo>* all_uses;    // 所有使用列表
  unsigned int total_use_count;    // 总使用次数

  // 逃逸信息
  vec<SourceUseEscapeLocation>* escape_locations; // 所有逃逸位置
  unsigned int escape_count;       // 逃逸使用次数
  bool has_escape;                 // 是否存在逃逸
  SourceUseEscapeKind dominant_escape_kind; // 主要逃逸类型

  // 数据流链
  SourceUseChainNode* use_chain_root; // 使用链根节点

  // 统计信息
  unsigned int max_use_depth;      // 最大使用深度
  bool is_fully_analyzed;          // 是否完全分析（未遇到分析边界）

  // ========== 异构链表字段 ==========
  void* aux;                       // 用于挂载到原 hash_map 的异构链表
                                   // 可以将原 FieldWriteSourceInfo 的 aux 摘下
                                   // 换成本结构，再用本结构的 aux 重新挂回原数据

  // ========== 关联原始数据 ==========
  void* original_write_info;       // 指向原始 FieldWriteSourceInfo 的指针
                                   // 用于反向查找对应的 write 操作
};

// ============================================================================
// 逃逸规则配置
// ============================================================================

struct SourceUseEscapeRules {
  // 函数调用相关
  bool external_call_is_escape;    // 外部函数调用是否算逃逸（默认 true）
  bool virtual_call_is_escape;     // 虚函数调用是否算逃逸（默认 true）
  bool indirect_call_is_escape;    // 间接调用是否算逃逸（默认 true）

  // 存储相关
  bool global_store_is_escape;     // 存储到全局变量是否算逃逸（默认 true）
  bool heap_store_is_escape;       // 存储到堆对象是否算逃逸（默认 true）
  bool field_store_is_escape;      // 存储到对象字段是否算逃逸（默认 false）

  // 返回值
  bool return_is_escape;           // 返回是否算逃逸（默认 true）

  // 参数传递
  bool param_to_external_is_escape; // 传递给外部函数参数是否算逃逸（默认 true）
  bool param_to_internal_is_escape; // 传递给内部函数参数是否算逃逸（默认 false）

  // 分析深度
  unsigned int max_analysis_depth; // 最大分析深度（默认 5）
  bool stop_at_first_escape;       // 发现第一个逃逸后是否停止分析（默认 false）
};

// 获取默认逃逸规则
SourceUseEscapeRules getDefaultEscapeRules();

// ============================================================================
// 分析接口
// ============================================================================

// 分析单个源操作数的使用情况
SourceUseAnalysisResult* analyzeSourceOperandUse(
  tree source_operand,
  gimple* source_stmt,
  const SourceUseEscapeRules& rules
);

// 从 FieldWriteSourceInfo 中提取源操作数并分析
SourceUseAnalysisResult* analyzeFromWriteSourceInfo(
  void* write_source_info,  // 实际类型是 FieldWriteSourceInfo*
  const SourceUseEscapeRules& rules
);

// 释放分析结果
void freeSourceUseAnalysisResult(SourceUseAnalysisResult* result);

// ============================================================================
// 辅助函数
// ============================================================================

// 判断使用是否为逃逸
bool isEscapeUse(
  const SourceUseInfo& use_info,
  const SourceUseEscapeRules& rules
);

// 获取逃逸类型描述字符串
const char* getEscapeKindString(SourceUseEscapeKind kind);

// 获取使用类型描述字符串
const char* getUseKindString(SourceUseKind kind);

// 打印分析结果（调试用）
void printSourceUseAnalysisResult(
  const SourceUseAnalysisResult* result,
  FILE* output
);

} // namespace array_detect_ns
