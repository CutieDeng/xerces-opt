#pragma once

// ============================================================================
// ad-field-write-capacity 模块
// ============================================================================
// 分析字段写入中的 malloc size 参数，寻找关联整数字段
//
// 数据流：
//   field-write-info, write-original-source, type -> (listof malloc-capacity-evidence)
//   (field, (listof field-write-info)) -> (mapof integer-field (listof malloc-capacity-evidence))
//
// 场景：分析 ptr = malloc(a) 中 a 被写入了哪些整数字段
// 例如：obj.d = malloc(a); obj.a = a; obj.b = a;
//       -> 产生两个证据：(d, a) 和 (d, b)
// ============================================================================

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "field-write.hh"
#include "write-source.hh"
#include "field-wrapper.hh"

namespace array_detect_ns {

using namespace ::array_detector;
using namespace ::field_analysis;

// ============================================================================
// 置信度枚举
// ============================================================================

enum MallocEvidenceConfidence {
  MALLOC_CONF_CERTAIN,    // 确定：直接引用字段
  MALLOC_CONF_PROBABLE,   // 可能：间接引用或部分匹配
  MALLOC_CONF_WEAK        // 弱：推测性关联
};

// ============================================================================
// malloc 容量证据
// ============================================================================
// (malloc-capacity-evidence
//   pointer_field    : tree                   ; 指针字段
//   integer_field    : tree                   ; 关联的整数字段
//   write_info       : field-write-info       ; 写入信息
//   malloc_call      : gimple                 ; malloc 调用语句
//   size_expr        : tree                   ; size 表达式
//   confidence       : (or 'certain 'probable 'weak))

struct MallocCapacityEvidence {
  // === 关联信息 ===
  tree pointer_field;               // 指针字段 (FIELD_DECL)
  tree integer_field;               // 关联的整数字段 (FIELD_DECL)
  tree containing_type;             // 所属类型

  // === 来源信息 ===
  FieldWriteInfo* write_info;       // 字段写入信息
  WriteOriginalSource* write_source; // 写入来源

  // === malloc 调用信息 ===
  gimple* malloc_call;              // malloc/calloc/realloc 调用语句
  char const* alloc_func_name;      // 分配函数名
  tree size_expr;                   // size 表达式
  tree element_count_expr;          // 元素数量表达式（如果可分离）

  // === 置信度 ===
  MallocEvidenceConfidence confidence;

  // === 位置信息 ===
  location_t location;
  char const* description;
};

// ============================================================================
// 函数声明
// ============================================================================

// 分析单个字段写入的 malloc 容量关联
// 输入：write_info, write_source
// 输出：(listof MallocCapacityEvidence*) - 可能关联多个整数字段
ArrayDetectErrorCode analyzeMallocCapacity (
  AD_FUNC_ARGS,
  FieldWriteInfo* write_info,
  WriteOriginalSource* write_source,
  tree containing_type,
  vec<MallocCapacityEvidence*, va_gc>** results
);

// 收集所有 malloc 容量证据
// 输入：TypeFieldAnalysisData (包含所有写入)
// 输出：填充 tfad->malloc_evidences
ArrayDetectErrorCode collectAllMallocEvidences (
  AD_FUNC_ARGS,
  Wrapper_FieldEscapeConclude_OwnershipConclude* tfad
);

// 查找 size 表达式关联的所有整数字段
// 方式1：表达式直接引用字段（如 obj.count）
// 方式2：表达式的值被写入到整数字段（如 a 被写入 obj.a 和 obj.b）
ArrayDetectErrorCode traceSizeToIntegerFields (
  AD_FUNC_ARGS,
  tree size_expr,
  tree containing_type,
  vec<tree, va_gc>** out_integer_fields
);

// 检查 gimple 语句是否为 malloc/calloc/realloc 调用
bool isMallocLikeCall (gimple* stmt, char const** out_func_name);

// 从 malloc 调用中提取 size 表达式
tree extractMallocSizeExpr (gimple* call_stmt, char const* func_name);

// 打印 malloc 容量证据
void printMallocCapacityEvidence (
  AD_FUNC_ARGS,
  FILE* out,
  MallocCapacityEvidence* evidence
);

// 打印所有 malloc 容量证据（从 hashmap）
void printAllMallocEvidences (
  AD_FUNC_ARGS,
  FILE* out,
  MallocEvidencesByIntegerFieldMap* evidences_map
);

// 获取置信度名称
char const* mallocConfidenceToString (MallocEvidenceConfidence conf);

} // namespace array_detect_ns
