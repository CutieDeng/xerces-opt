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
  struct TypeFieldAnalysisData;

} // namespace array_detector

namespace array_detect_ns {

// ============================================================================
// 逃逸综合分析模块 (Escape Synthesizer) - 两层架构
// ============================================================================
// 第一层：逃逸提取模块 - 从 all_uses 中提取逃逸使用
// 第二层：逃逸证据模块 - 统计非调试逃逸，生成 owned 判定证据
// ============================================================================

// ============================================================================
// 第一层：逃逸提取结果 (Escape Extraction Result)
// ============================================================================
// 从 SourceUseAnalysisResult 中提取所有逃逸使用，形成汇总列表

struct EscapeExtractionResult {
  tree source_operand;              // 源操作数
  gimple * source_stmt;             // 源语句

  vec<SourceUseInfo const *> * escapes;  // 所有逃逸使用（指向原 all_uses 中的元素）
  unsigned int escape_count;        // 逃逸数量

  void * aux;
  void * original_write_info;       // 原始写入信息（FieldWriteCapture*）
};

// ============================================================================
// 第二层：逃逸证据结果 (Escape Evidence Result)
// ============================================================================
// 统计非调试逃逸数目，生成 (type, field, write-location) 的证据

struct EscapeEvidenceResult {
  // 写入位置标识
  tree type;
  tree field_decl;
  location_t write_location;

  // 证据统计
  unsigned int total_escapes;           // 总逃逸数
  unsigned int safe_debug_escapes;      // 调试逃逸数
  unsigned int rejecting_escapes;       // 非法逃逸数（= total - safe_debug）

  // 核心判定
  bool has_rejecting_evidence;          // rejecting_escapes > 0

  void * aux;
  void * original_write_info;           // 原始写入信息（FieldWriteCapture*）
};

// ============================================================================
// 第三层：(type, field) 级别逃逸汇总 (Type Field Escape Summary)
// ============================================================================
// 汇总单个 (type, field) 的所有写入操作的逃逸信息

struct TypeFieldEscapeSummary {
  // === 标识 ===
  tree type;
  tree field_decl;

  // === 字段写入操作统计 ===
  unsigned int total_field_writes;              // 总字段写入操作数
  unsigned int field_writes_with_escape;        // 有逃逸的字段写入操作数
  unsigned int field_writes_with_rejecting;     // 有拒绝证据的字段写入操作数
  unsigned int field_writes_without_analysis;   // 未分析的字段写入操作数

  // === 逃逸统计（聚合所有写入操作）===
  unsigned int total_escapes;             // 总逃逸数
  unsigned int safe_debug_escapes;        // 调试逃逸数
  unsigned int rejecting_escapes;         // 拒绝性逃逸数

  // === 来源类型分布 ===
  unsigned int source_function_call;      // 函数调用来源数
  unsigned int source_field_access;       // 字段访问来源数
  unsigned int source_constant;           // 常量来源数
  unsigned int source_computation;        // 计算来源数
  unsigned int source_phi;                // PHI 节点来源数
  unsigned int source_unknown;            // 未知来源数

  // === 核心判定 ===
  bool has_rejecting_evidence;            // 是否存在拒绝证据
  float rejection_ratio;                  // 拒绝比例 = field_writes_with_rejecting / total_field_writes

  // === 详细记录引用 ===
  vec<EscapeEvidenceResult*> * all_evidences;  // 所有写入操作的证据列表
};

// ============================================================================
// 第三层模块接口：(type, field) 级别汇总
// ============================================================================

// 对单个 (type, field) 的所有写入操作进行逃逸汇总
// 输入：field_data - 类型字段分析数据（包含所有写入操作记录）
// 输出：result - 汇总结果（GC 管理）
ArrayDetectErrorCode summarizeTypeFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::TypeFieldAnalysisData * field_data,
  TypeFieldEscapeSummary * &result
);

// ============================================================================
// 第一层模块接口：逃逸提取
// ============================================================================

// 从 SourceUseAnalysisResult 中提取所有逃逸使用
// 输入：raw_result - 原始使用分析结果
// 输出：result - 逃逸提取结果（GC 管理）
ArrayDetectErrorCode extractEscapes (
  AD_FUNC_ARGS,
  SourceUseAnalysisResult * raw_result,
  EscapeExtractionResult * &result
);

// ============================================================================
// 第二层模块接口：逃逸证据生成
// ============================================================================

// 根据逃逸提取结果，生成 owned 判定证据
// 输入：extraction - 逃逸提取结果
//       type - 类型
//       field_decl - 字段声明
//       write_location - 写入位置
// 输出：result - 逃逸证据结果（GC 管理）
ArrayDetectErrorCode generateEscapeEvidence (
  AD_FUNC_ARGS,
  EscapeExtractionResult * extraction,
  tree type,
  tree field_decl,
  location_t write_location,
  EscapeEvidenceResult * &result
);

// ============================================================================
// 组合接口：综合所有字段的逃逸信息
// ============================================================================

// 综合所有字段的逃逸信息（调用两层模块）
// 输入：detector - 包含逃逸收集结果的检测器
// 输出：evidence_results - 证据结果列表
//       total_synthesized - 总综合数量
ArrayDetectErrorCode synthesizeAllFieldEscapes (
  AD_FUNC_ARGS,
  array_detector::ArrayDetector &detector,
  vec<EscapeEvidenceResult*> * &evidence_results,
  unsigned int &total_synthesized
);

// ============================================================================
// 调试逃逸识别（唯一允许字符串匹配的场景）
// ============================================================================

// 判断是否为已知安全调试函数
// 仅在此函数中允许使用字符串模糊匹配
bool isKnownSafeDebugFunction (
  AD_FUNC_ARGS,
  char const * function_name
);

// 判断逃逸是否为安全调试逃逸
// 仅当逃逸为函数调用且函数为调试函数时返回 true
bool isSafeDebugEscape (
  AD_FUNC_ARGS,
  SourceUseInfo const * escape
);

// ============================================================================
// 调试输出
// ============================================================================

// 打印逃逸提取结果（调试用）
void printEscapeExtractionResult (
  EscapeExtractionResult const * result,
  FILE * output
);

// 打印逃逸证据结果（调试用）
void printEscapeEvidenceResult (
  EscapeEvidenceResult const * result,
  FILE * output
);

} // namespace array_detect_ns
