#pragma once

#include "gcc-common.hh"
#include "context.hh"
#include "array-detect-context-gcc.hh"
#include "state.hh"
#include "prelude.hh"
#include "field-write.hh"
#include "field-wrapper.hh"
#include "source-escape-use-info.hh"

namespace array_detector {
  class ArrayDetector;
} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 源使用分析模块 (Source Use Analysis)
// ============================================================================
// 数据流：write-original-source -> (listof write-original-source-use)
// 追踪 write-original-source 的 SSA 使用链，收集所有使用点
//
// (write-original-source-use  ; 即 SourceUseInfo
//   kind              : source-use-kind
//   use-stmt          : gimple*
//   escape-kind       : escape-kind
//   escape-target     : string)
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

// 逃逸目标信息联合体（仅当 escape_kind != SU_ESCAPE_NONE 时有效）
union EscapeTargetInfo {
  tree function_decl;             // 对于函数调用逃逸
  tree field_decl;                // 对于字段存储逃逸
  tree global_var;                // 对于全局存储逃逸
};

// ============================================================================
// 统一的使用信息结构
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
// 分析配置常量
// ============================================================================

// 最大分析深度（SSA 使用链追踪）
constexpr unsigned int MAX_ESCAPE_ANALYSIS_DEPTH = 5;

// ============================================================================
// 公开接口
// ============================================================================

// 主入口：分析源操作数的所有使用
// 数据流：source_operand -> (listof wrapper-source-use-info-source-escape-use-info)
ArrayDetectErrorCode analyzeSourceUse (
  AD_FUNC_ARGS,
  tree source_operand,
  gimple * exclude_stmt,
  vec<field_analysis::Wrapper_SourceUseInfo_SourceEscapeUseInfo*, va_gc>** out_uses
);

// ============================================================================
// Pipeline 接口
// ============================================================================

// 收集所有字段的使用信息
// 输入：detector - 包含 m_type_field_writes 的检测器
// 输出：total_analyzed - 总分析数量
// 注意：此函数只填充 uses，escape_use_info 由 source-escape-use-info 模块后续填充
ArrayDetectErrorCode collectAllFieldUses (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  unsigned int &total_analyzed
);

// ============================================================================
// 辅助函数
// ============================================================================

// 获取使用类型描述字符串
char const * getUseKindString (SourceUseKind kind);

} // namespace array_detect_ns
