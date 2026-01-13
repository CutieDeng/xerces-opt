#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"
#include "info.hh"
#include "array-detect-context-gcc-interface.hh"
#include "field-write.hh"
#include "type-field-hash.hh"
#include "field-wrapper.hh"

// ============================================================================
// 前置声明：各分析模块的结果类型
// ============================================================================
// 数据流关系（三层 Wrapper 结构）：
// - 层级2: Wrapper_SourceUse_EscapedUse
// - 层级1: Wrapper_WriteInfo_WriteSource_SourceEscapeConclude_TransferInfo
// - 层级3: Wrapper_FieldEscapeConclude_TransferStats (per field)
// ============================================================================

namespace array_detect_ns {
  // 前置声明：各分析模块的结果类型
  struct FieldWriteInfo;
  struct SourceUseInfo;
  struct EscapedUseResult;
  struct SourceEscapeConclude;
  struct FieldEscapeConclude;
  struct TransferInfo;
  struct TransferStats;
}

namespace array_detector {

  struct WriteOriginalSource;  // 写入原始来源

  using namespace ::array_detect_ns;
  using namespace ::field_analysis;

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

// 类型-字段键：用于 hash_map 的复合键
struct TypeFieldKey {
  tree type;        // 类型（TYPE_MAIN_VARIANT）
  tree field_decl;  // 字段声明（FIELD_DECL）
};

// 哈希和比较函数声明
size_t hashTypeFieldKey (TypeFieldKey const *key);
bool equalTypeFieldKey (TypeFieldKey const *key1, TypeFieldKey const *key2);

} // namespace array_detector

// 在全局命名空间中特化 default_hash_traits
#include "type-field-hash-traits.hh"

// 定义 hash_map trait 类型
#include "type-field-hashmap-traits.hh"

namespace array_detector {

struct ArrayDetector {

  // 旧的数据结构（保留用于兼容，后续可移除）
  vec<FieldInfo*>* m_fields;

  // 新的数据结构：使用 hash_map 按 (type, field) 存储
  // TypeFieldAnalysisData = field_analysis::Wrapper_FieldEscapeConclude_TransferStats
  hash_map<TypeFieldKey, TypeFieldAnalysisData*, TypeFieldHashMapTraits>* m_type_field_writes;

};

}
