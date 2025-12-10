#include "gcc-common.hh"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "array-detector.hh"
#include "array-detect-context-gcc-interface.hh"

namespace gcc_ext_util {

using namespace ::array_detect_ns;

ArrayDetectErrorCode get_type_name (AD_FUNC_ARGS, tree type, char const *&result);
ArrayDetectErrorCode analyze_gimple_assignment (AD_FUNC_ARGS, gimple* stmt, ArrayDetector &detector, char const* func_name, tree func_decl);
ArrayDetectErrorCode process_type_fields(AD_FUNC_ARGS, tree type, ArrayDetector &detector, hash_set<tree>* processed_types);
ArrayDetectErrorCode is_pointer_type(AD_FUNC_ARGS, tree type, bool &result);
// 检查是否是字段访问（COMPONENT_REF）
ArrayDetectErrorCode is_field_access(AD_FUNC_ARGS, tree expr, tree* field_decl_out, tree* object_out, bool &result);

// 安全字符串复制函数，防止缓冲区溢出
void safe_string_copy(char* dest, size_t dest_size, const char* src);

// 辅助函数声明
ArrayDetectErrorCode get_source_location_string(AD_FUNC_ARGS, location_t loc, char* buffer, size_t buffer_size);
void get_source_line_content(location_t loc, char* buffer, size_t buffer_size);

} // namespace gcc_ext_util
