#include "gcc-common.hh"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "array-detect-context-gcc-interface.hh"

// Forward declaration for ArrayDetector (必须在 gcc_ext_util 命名空间外部)
namespace array_detector {
  class ArrayDetector;
}

namespace gcc_ext_util {

using namespace ::array_detect_ns;

ArrayDetectErrorCode get_type_name (AD_FUNC_ARGS, tree type, char const *&result);
ArrayDetectErrorCode analyze_gimple_assignment (AD_FUNC_ARGS, gimple * stmt, ::array_detector::ArrayDetector &detector, char const * func_name, tree func_decl);
ArrayDetectErrorCode process_type_fields (AD_FUNC_ARGS, tree type, ::array_detector::ArrayDetector &detector, hash_set<tree>* processed_types);
ArrayDetectErrorCode is_pointer_type (AD_FUNC_ARGS, tree type, bool &result);
// 检查是否是字段访问（COMPONENT_REF）
ArrayDetectErrorCode is_field_access (AD_FUNC_ARGS, tree expr, tree * field_decl_out, tree * object_out, bool &result);

// 安全字符串复制函数，防止缓冲区溢出
void safe_string_copy (char * dest, size_t dest_size, char const * src);

// 辅助函数声明
ArrayDetectErrorCode get_source_location_string (AD_FUNC_ARGS, location_t loc, char * buffer, size_t buffer_size);
void get_source_line_content (location_t loc, char * buffer, size_t buffer_size);

// 调试信息增强：打印字段写入捕获信息
// 包括：类型名（含命名空间）、字段名、字段类型名
ArrayDetectErrorCode logFieldWriteCapture (AD_FUNC_ARGS, tree containing_type, tree field_decl);

// 子函数：获取类型名（含命名空间），返回格式化的字符串
// 返回：成功返回 OK，result 指向格式化的类型名字符串（GCC 内部管理，无需释放）
ArrayDetectErrorCode formatTypeNameWithNamespace (AD_FUNC_ARGS, tree type, char const *&result);

// 从 GCC type tree 提取模板参数信息
// 优先使用 cp-tree.h API，不可用时回退到 demangling
// out_base_name: 基础类型名（不含模板参数）
// out_template_args: 模板参数列表（可能为 NULL）
ArrayDetectErrorCode extractTemplateArgsFromType (
  AD_FUNC_ARGS,
  tree type,
  char const** out_base_name,
  vec<char const*, va_gc>** out_template_args
);

// 子函数：获取字段名
// 返回：成功返回 OK，result 指向字段名字符串（GCC 内部管理，无需释放）
ArrayDetectErrorCode getFieldName (AD_FUNC_ARGS, tree field_decl, char const *&result);

// 子函数：获取字段类型名（处理指针）
// 返回：成功返回 OK，result 指向格式化的字段类型名字符串（GCC 内部管理，无需释放）
ArrayDetectErrorCode formatFieldTypeName (AD_FUNC_ARGS, tree field_type, char const *&result);

// 检测是否是编译器生成的字段（如虚表指针 vptr）
// 使用 DECL_ARTIFICIAL 检测，不依赖字段名硬编码
// 返回：成功返回 OK，result 为 true 表示是编译器生成的字段
ArrayDetectErrorCode isCompilerGeneratedField (AD_FUNC_ARGS, tree field_decl, bool &result);

} // namespace gcc_ext_util
