// ----------------------------------------------------------------------------
// 字段信息结构
// ----------------------------------------------------------------------------

// 单个赋值操作的详细信息
struct AssignmentDetail {
  const char* source;              // 赋值来源
  bool is_call;                    // 是否是函数调用
  int tree_code;                   // 右值表达式的树代码
  const char* location_info;       // 位置信息（文件名:行号，如果可用）
  const char* rhs_description;     // 右值表达式描述
};

// 函数级别的赋值信息
struct FunctionAssignment {
  const char* function_name;      // 函数名称（用于显示）
  tree function_decl;              // 函数声明（唯一标识）
  const char* function_id;        // 函数唯一标识字符串（基于decl和位置）
  int assignment_count;            // 该函数中该字段的赋值次数
  vec<const char*>* sources;       // 该函数中的赋值来源（简化版）
  vec<AssignmentDetail*>* assignment_details; // 详细的赋值信息
};

struct FieldInfo {
  const char* field_name;         // 字段名称
  const char* containing_type;    // 包含该字段的类型名称
  tree field_decl;                // 字段声明（用于匹配）
  tree containing_type_tree;     // 包含该字段的类型树
  bool is_pointer;                // 是否是指针类型
  bool is_array_candidate;        // 是否是数组候选
  int source_count;               // 总来源数量（所有函数）
  vec<const char*>* sources;       // 所有赋值来源（函数调用等）
  vec<const char*>* conflicting_assigns; // 冲突的赋值操作
  vec<FunctionAssignment*>* function_assignments; // 按函数分组的赋值信息
};
