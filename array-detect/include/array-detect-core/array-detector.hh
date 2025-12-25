#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"
#include "analysis-data.hh"
#include "type-field-hash.hh"

// ============================================================================
// 前置声明：各分析模块的结果类型
// ============================================================================
// 这些类型的完整定义在各自的模块中，这里只做前置声明
// 避免循环依赖，保持模块解耦

namespace array_detect_ns {
  struct FieldWriteCapture;       // 字段写入捕获（field-write-collector）
  struct SourceUseAnalysisResult; // 源使用分析结果（source-escape-collection）
  struct EscapeSynthesisResult;   // 逃逸综合结果（escape-synthesizer）
  struct OwnershipAnalysisResult; // 所有权分析结果（escape-synthesizer）
}

namespace array_detector {

  struct FieldSourceInfo;         // 字段源信息（write-operation-trace）

using namespace ::array_detect_ns;

class ArrayDetector;

// 检查字段的所有赋值是否来自同一源（用于判断数组候选）
bool checkAllAssignmentsFromSameSource (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field, char const ** out_unique_source);

// 分析字段的使用模式
ArrayDetectErrorCode analyzeFieldUsage (ArrayDetector &self, AD_FUNC_ARGS);

// 清理检测器资源
void deinit (ArrayDetector &self, AD_FUNC_ARGS);

// 添加字段到检测器
ArrayDetectErrorCode addField (ArrayDetector &self, AD_FUNC_ARGS, FieldInfo * field_info);

// 获取检测器中的字段数量
ArrayDetectErrorCode getFieldCount (ArrayDetector const &self, AD_FUNC_ARGS, size_t * out_count);
// 通过索引获取字段信息
ArrayDetectErrorCode getField (ArrayDetector const &self, AD_FUNC_ARGS, size_t index, FieldInfo** out_field);

// 初始化检测器（非延迟，直接分配并创建容器，不做空指针检查）
ArrayDetectErrorCode init (ArrayDetector &self, AD_FUNC_ARGS);

}

namespace array_detector {

// ============================================================================
// 统一结果记录结构：单个字段写入操作的完整分析链
// ============================================================================
// 将原来通过 aux 链表连接的异构数据，拍平为清晰的并列字段
// 每个分析阶段填充对应的结果指针，实现模块解耦和类型安全

struct FieldWriteAnalysisRecord {
  // === 核心写入信息（必需，由 field-write-collector 生成）===
  FieldWriteCapture* write_capture;           // 字段写入捕获：基本的 IR 信息
                                               // 包含：语句、位置、类型、字段、赋值表达式等

  // === 源操作数分析（可选，由 write-operation-trace 生成）===
  FieldSourceInfo* source_info;               // 字段源信息：源操作数的来源分析
                                               // 包含：函数调用、字段访问、常量、计算等

  // === 逃逸分析（可选，由 source-escape-collection 生成）===
  SourceUseAnalysisResult* escape_analysis;   // 源使用分析结果：源操作数的逃逸分析
                                               // 包含：所有使用、逃逸位置、逃逸类型等

  // === 逃逸综合（可选，由 escape-synthesizer 生成）===
  EscapeSynthesisResult* escape_synthesis;    // 逃逸综合结果：逃逸的分类和统计
                                               // 包含：逃逸类别位图、详细统计等

  // === 元数据 ===
  void* reserved;                              // 保留字段，供未来扩展使用
};

// ============================================================================
// 类型字段分析数据：单个 (type, field) 的完整分析结果
// ============================================================================
// 包含该字段的所有写入操作记录，以及字段级别的综合分析结果

struct TypeFieldAnalysisData {
  // === 标识信息 ===
  tree type;                                   // 类型（TYPE_MAIN_VARIANT）
  tree field_decl;                             // 字段声明（FIELD_DECL）

  // === 写入操作分析记录列表 ===
  vec<FieldWriteAnalysisRecord*>* write_analysis_records;
                                               // 该字段的所有写入操作的完整分析记录

  // === 字段级别综合分析（可选）===
  OwnershipAnalysisResult* ownership_analysis; // 所有权分析结果：字段是否支持 owned 指针
                                               // 基于所有写入操作的逃逸综合结果生成

  // === 元数据 ===
  void* reserved;                              // 保留字段，供未来扩展使用
};

// ============================================================================
// 向后兼容别名（逐步废弃）
// ============================================================================
// 为了平滑迁移，保留旧名称作为别名，后续版本将移除
typedef TypeFieldAnalysisData TypeFieldWriteOps;

// 类型-字段键：用于 hash_map 的复合键（简单结构体）
// 注意：定义也在 type-field-hash-traits.hh 中，避免循环依赖
struct TypeFieldKey {
  tree type;        // 类型（TYPE_MAIN_VARIANT，GCC 内部管理）
  tree field_decl;  // 字段声明（FIELD_DECL，GCC 内部管理）
};

// 哈希和比较函数声明（实现在 type-field-hash.cc 中）
// 使用普通函数，不使用成员函数或 C++ 特性
size_t hashTypeFieldKey (TypeFieldKey const *key);
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2);

} // namespace array_detector

// 在全局命名空间中特化 default_hash_traits（必须在 array_detector 命名空间之后）
// 此时 TypeFieldKey 已经完整定义
#include "type-field-hash-traits.hh"

// 定义 hash_map trait 类型（必须在 default_hash_traits 特化之后）
#include "type-field-hashmap-traits.hh"

namespace array_detector {

struct ArrayDetector {

  // 旧的数据结构（保留用于兼容，后续可移除）
  vec<FieldInfo*>* m_fields; // 使用指针类型，延迟初始化
  
  // 新的数据结构：使用 hash_map 按 type -> field 存储写入操作记录
  // 使用 hash_map 提供 O (1) 的查找性能，而不是 vec 的 O (n) 线性查找
  // 使用 TypeFieldHashMapTraits 提供 hash 和 equal 函数
  // hash_map 支持迭代器遍历（begin/end），可以直接遍历所有条目
  hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits>* m_type_field_writes; // 使用指针类型，延迟初始化

};

}

