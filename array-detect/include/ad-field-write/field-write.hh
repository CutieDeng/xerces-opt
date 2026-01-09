#pragma once

// ============================================================================
// Field 分析数据流总览
// ============================================================================
//
// 数据流变换（详见 doc/FIELD-ANALYSIS-DATAFLOW.md）：
//
// [写入级分析]
// collect-writes : field -> (listof write-info)
// trace-source   : write-info -> write-source
// analyze-uses   : write-source -> use-analysis
// conclude-escape: use-analysis -> escape-conclude
// analyze-move   : write-info, write-source -> move-analysis
//
// [字段级分析]
// conclude-field : (listof write-analysis) -> field-conclude
//
// 聚合结构：
// - WriteAnalysis : 单次写入的完整分析（聚合所有一对一关系）
// - FieldAnalysis : 字段级分析（聚合多个 WriteAnalysis）
//
// 新命名（简化）-> 旧命名（兼容）：
// - WriteInfo      -> FieldWriteInfo
// - WriteSource    -> WriteOriginalSource
// - UseInfo        -> SourceUseInfo
// - UseAnalysis    -> SourceUseResult
// - EscapeConclude -> SourceEscapeConclude
// - MoveAnalysis   -> OwnershipMoveResult
// - WriteAnalysis  -> FieldWriteAnalysisRecord
// - FieldConclude  -> FieldEscapeConclude
// - FieldAnalysis  -> TypeFieldAnalysisData
// ============================================================================

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"

namespace array_detect_ns {

// ============================================================================
// 字段写入信息 (FieldWriteInfo)
// ============================================================================
// 数据流位置：field -> (listof field-write-info)
// 表示单个字段写入操作的 GIMPLE 层信息
//
// (field-write-info
//   type           : tree           ; TYPE_MAIN_VARIANT
//   field-decl     : tree           ; FIELD_DECL
//   stmt           : gimple*        ; GIMPLE_ASSIGN
//   lhs            : tree           ; 左值 (MEM_REF/COMPONENT_REF)
//   rhs            : tree           ; 右值 (SSA_NAME)
//   location       : location_t     ; 源码位置)
// ============================================================================

struct FieldWriteInfo {
  // 核心信息：类型和字段
  tree type;                    // 类型（TYPE_MAIN_VARIANT）
  tree field_decl;              // 字段声明（FIELD_DECL）

  // 上下文信息：函数和基本块
  tree function_decl;           // 函数声明
  basic_block bb;               // 基本块
  char const * function_name;   // 所在函数名（ggc_strdup）

  // GIMPLE 语句信息
  gimple * stmt;                // GIMPLE_ASSIGN 语句
  tree lhs;                     // 左值表达式
  tree rhs;                     // 右值表达式

  // 源码位置
  location_t location;          // 源码位置

  // 辅助指针：后续阶段填充 WriteOriginalSource*
  void * aux;

  // 调试字段
  int bb_index;                 // 基本块索引
};

// 向后兼容别名
typedef FieldWriteInfo FieldWriteCapture;

// ============================================================================
// 数据结构：逃逸位置信息
// ============================================================================

enum EscapeType {
  ESCAPE_WRITE,      // 写入其他位置
  ESCAPE_ARGUMENT,  // 作为函数参数传递
  ESCAPE_COMPUTE,   // 参与计算
  ESCAPE_RETURN     // 作为返回值
};

struct EscapeSite {
  gimple * stmt;                 // 发生逃逸的语句（GCC 内部管理）
  EscapeType escape_type;      // 逃逸类型
  char const * function_name;    // 所在函数名（ggc_strdup 分配）
  location_t location;         // 源码位置（GCC 内部管理）
  char const * description;     // 逃逸描述（ggc_strdup 分配）
};

// ============================================================================
// 数据结构：源操作信息（函数调用）
// ============================================================================

enum CallType {
  CALL_VIRTUAL,     // C++ 虚函数调用
  CALL_DIRECT,      // 直接函数调用
  CALL_INDIRECT,    // 间接函数调用（函数指针）
  CALL_UNKNOWN      // 未知类型调用
};

struct SourceOperation {
  gimple * call_stmt;           // GIMPLE_CALL 语句（GCC 内部管理）
  CallType call_type;          // 调用类型
  char const * function_name;   // 函数名（mangled，ggc_strdup 分配）
  tree return_value_ssa;       // 返回值的 SSA_NAME（GCC 内部管理）
  tree vtable_ref;             // 虚表引用（如果是虚函数，GCC 内部管理）
  char const * signature;       // 调用签名（ggc_strdup 分配）
  location_t location;         // 调用位置（GCC 内部管理）
};

// ============================================================================
// 数据结构：字段分析结果
// ============================================================================

struct FieldAnalysisResult {
  tree field_decl;                    // 字段声明（GCC 内部管理）
  vec<FieldWriteCapture*>* write_ops; // 字段写入捕获列表（ggc_alloc<vec<...>>() 分配）
  vec<tree>* source_vars;             // 源变量列表（SSA_NAME，ggc_alloc<vec<...>>() 分配）
  vec<EscapeSite*>* escape_sites;     // 逃逸位置列表（ggc_alloc<vec<...>>() 分配）
  vec<SourceOperation*>* sources;     // 源操作列表（ggc_alloc<vec<...>>() 分配）
  bool is_memory_owner;                // 是否为内存持有者
  char const * reason;                  // 判定原因（ggc_strdup 分配）
};

// ============================================================================
// 数据结构：函数级分析结果
// ============================================================================

// map: field_decl -> FieldAnalysisResult
// 使用 vec 配对实现映射关系
struct FunctionAnalysisResult {
  char const * function_name;           // 函数名（ggc_strdup 分配）
  tree function_decl;                  // 函数声明（GCC 内部管理）
  vec<tree>* field_decls;              // 字段声明列表
  vec<FieldAnalysisResult*>* field_results; // 对应的分析结果列表
};

// ============================================================================
// 简化命名别名
// ============================================================================
// 新的简化命名，更清晰地反映数据流关系
// 详见 field-analysis.hh 中的完整聚合结构定义

// WriteInfo 是 FieldWriteInfo 的简化别名
typedef FieldWriteInfo WriteInfo;

} // namespace array_detect_ns

// ============================================================================
// 收集函数声明
// ============================================================================
// 数据流：遍历编译单元 -> (type, field) -> (listof FieldWriteInfo)

namespace array_detector {
  class ArrayDetector;
}

namespace array_detect_ns {

using array_detector::ArrayDetector;

// 主入口：收集所有字段写入
// 遍历编译单元中的所有函数，收集字段写入操作
ArrayDetectErrorCode collectAllFieldWrites (
  AD_FUNC_ARGS,
  ArrayDetector& detector
);

// 子函数：扫描单个函数
ArrayDetectErrorCode collectAllFieldWrites_scanFunction (
  AD_FUNC_ARGS,
  struct cgraph_node* node,
  ArrayDetector& detector,
  unsigned int& write_count
);

// 子函数：扫描基本块
ArrayDetectErrorCode collectAllFieldWrites_scanBasicBlock (
  AD_FUNC_ARGS,
  basic_block bb,
  tree func_decl,
  ArrayDetector& detector,
  unsigned int& write_count
);

// 子函数：检查语句是否为字段写入
// 返回 OK 并设置 result 为 FieldWriteInfo 指针，如果不是字段写入则 result 为 NULL
ArrayDetectErrorCode collectAllFieldWrites_checkStatement (
  AD_FUNC_ARGS,
  gimple* stmt,
  basic_block bb,
  tree func_decl,
  FieldWriteInfo*& result
);

// 子函数：创建 FieldWriteInfo 并添加到 detector
ArrayDetectErrorCode collectAllFieldWrites_createWriteInfo (
  AD_FUNC_ARGS,
  gimple* stmt,
  tree lhs,
  tree rhs,
  tree field_decl,
  tree containing_type,
  basic_block bb,
  tree func_decl,
  ArrayDetector& detector
);

} // namespace array_detect_ns
