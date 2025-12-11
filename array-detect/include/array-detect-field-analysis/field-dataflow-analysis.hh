#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "analysis-data.hh"

namespace array_detect_ns {

// ============================================================================
// 字段数据流分析
// ============================================================================
// 追踪字段值的完整数据流路径
// ============================================================================

// 数据流节点类型
enum DataFlowNodeType {
  DEF_FIELD_WRITE,    // 字段写入定义
  DEF_FIELD_COPY,     // 字段值复制
  DEF_FIELD_CALL,     // 函数调用写入
  USE_FIELD_READ,     // 字段读取使用
  USE_FIELD_COMPUTE,  // 字段值计算使用
  USE_FIELD_ARG,      // 字段值参数使用
  USE_FIELD_RETURN,   // 字段值返回使用
  TRANSFER_SSA,       // SSA 传递
  TRANSFER_PHI,       // PHI 传递
  TRANSFER_CALL       // 调用传递
};

// 数据流节点
struct DataFlowNode {
  DataFlowNodeType type;
  gimple* stmt;              // 相关语句（GCC 内部管理）
  tree ssa_var;              // 相关 SSA 变量（GCC 内部管理）
  tree field_decl;           // 相关字段（GCC 内部管理）
  location_t location;       // 源码位置（GCC 内部管理）
  const char* description;   // 节点描述（ggc_strdup 分配）
};

// 数据流边类型
enum DataFlowEdgeType {
  EDGE_DEF_USE,      // 定义到使用
  EDGE_TRANSFER,     // 传递边
  EDGE_SOURCE,       // 来源边
  EDGE_SINK          // 去向边
};

// 数据流边
struct DataFlowEdge {
  DataFlowNode* from;        // 源节点
  DataFlowNode* to;          // 目标节点
  DataFlowEdgeType type;      // 边类型
  const char* description;   // 边描述（ggc_strdup 分配）
};

// 数据流图
struct DataFlowGraph {
  vec<DataFlowNode*>* nodes;  // 节点列表（ggc_alloc<vec<...>>() 分配）
  vec<DataFlowEdge*>* edges;  // 边列表（ggc_alloc<vec<...>>() 分配）
  tree field_decl;            // 分析的字段（GCC 内部管理）
  function* fn;               // 所在函数（GCC 内部管理）
};

// 值来源类型
enum ValueSourceType {
  SOURCE_CALL,       // 函数调用
  SOURCE_CONSTANT,   // 常量
  SOURCE_VARIABLE,   // 变量
  SOURCE_EXPRESSION, // 表达式
  SOURCE_PHI         // PHI 节点
};

// 值来源
struct ValueSource {
  ValueSourceType type;
  gimple* stmt;              // 相关语句（GCC 内部管理）
  tree value;                // 值表达式（GCC 内部管理）
  const char* description;   // 来源描述（ggc_strdup 分配）
};

// 值去向类型
enum ValueSinkType {
  SINK_FIELD_WRITE,  // 字段写入
  SINK_CALL_ARG,     // 函数调用参数
  SINK_RETURN,       // 返回值
  SINK_COMPUTE       // 计算使用
};

// 值去向
struct ValueSink {
  ValueSinkType type;
  gimple* stmt;              // 相关语句（GCC 内部管理）
  tree value;                // 值表达式（GCC 内部管理）
  const char* description;   // 去向描述（ggc_strdup 分配）
};

// 构建字段数据流图
// 返回值：ArrayDetectErrorCode
// 输入：field_decl - 字段声明
// 输入：fn - 函数
// 输入：field_writes - 字段写入操作列表
// 输出：graph - 数据流图
ArrayDetectErrorCode buildDataFlowGraph(
  AD_FUNC_ARGS,
  tree field_decl,
  function* fn,
  vec<WriteOperation*> const &field_writes,
  DataFlowGraph &graph
);

// 前向数据流分析（从定义到使用）
// 返回值：ArrayDetectErrorCode
// 输入：graph - 数据流图
// 输出：use_nodes - 使用节点列表
ArrayDetectErrorCode forwardDataFlowAnalysis(
  AD_FUNC_ARGS,
  DataFlowGraph const &graph,
  vec<DataFlowNode*> &use_nodes
);

// 后向数据流分析（从使用到定义）
// 返回值：ArrayDetectErrorCode
// 输入：graph - 数据流图
// 输入：use_node - 使用节点
// 输出：def_nodes - 定义节点列表
ArrayDetectErrorCode backwardDataFlowAnalysis(
  AD_FUNC_ARGS,
  DataFlowGraph const &graph,
  DataFlowNode* use_node,
  vec<DataFlowNode*> &def_nodes
);

// 追踪字段值的来源
// 返回值：ArrayDetectErrorCode
// 输入：field_decl - 字段声明
// 输入：write_op - 写入操作
// 输出：sources - 值来源列表
ArrayDetectErrorCode traceValueSource(
  AD_FUNC_ARGS,
  tree field_decl,
  WriteOperation* write_op,
  vec<ValueSource*> &sources
);

// 追踪字段值的去向
// 返回值：ArrayDetectErrorCode
// 输入：field_decl - 字段声明
// 输入：read_stmt - 读取语句
// 输出：sinks - 值去向列表
ArrayDetectErrorCode traceValueSink(
  AD_FUNC_ARGS,
  tree field_decl,
  gimple* read_stmt,
  vec<ValueSink*> &sinks
);

} // namespace array_detect_ns
