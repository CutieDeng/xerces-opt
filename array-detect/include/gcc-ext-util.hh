#include "gcc-common.hh"

#include "prelude.hh"
#include "state.hh"
#include "context.hh"
#include "array-detector.hh"

namespace gcc_ext_util {

using namespace ::cutie_ns;

CutieErrorCode get_type_name (CUTIE_FUNC_ARGS, tree type, char const *&result);
CutieErrorCode analyze_gimple_assignment (CUTIE_FUNC_ARGS, gimple* stmt, ArrayDetector* detector, char const* func_name, tree func_decl);
CutieErrorCode process_type_fields(CUTIE_FUNC_ARGS, tree type, ArrayDetector* detector, hash_set<tree>* processed_types);
CutieErrorCode is_pointer_type(CUTIE_FUNC_ARGS, tree type, bool &result);
// 检查是否是字段访问（COMPONENT_REF）
CutieErrorCode is_field_access(CUTIE_FUNC_ARGS, tree expr, tree* field_decl_out, tree* object_out, bool &result);

// 安全字符串复制函数，防止缓冲区溢出
void safe_string_copy(char* dest, size_t dest_size, const char* src);

// 辅助函数声明
void get_source_location_string(location_t loc, char* buffer, size_t buffer_size);
void get_source_line_content(location_t loc, char* buffer, size_t buffer_size);

} // namespace gcc_ext_util
