#include "gcc-plugin.h"
#include "plugin-version.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "cgraph.h"
#include "plugin.h"
#include "diagnostic.h"
#include "langhooks.h"
#include "context.h"
#include "gimple.h"
#include "stringpool.h"
#include "vec.h"
#include "hash-map.h"
#include "tree-iterator.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "tree-ssa.h"
#include "print-tree.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#define CUTIE_FUNCTION_BEGIN \
  { ::cutie_ns::CutieErrorCode ecode;

#define CUTIE_FUNCTION_RAW_END \
  return ecode; }

#define CUTIE_FUNCTION_END \
  cleanup:; CUTIE_FUNCTION_RAW_END

#define RET \
  do { goto cleanup; } while (0)

#define CUTIE_DEBUG_PRINT_RAW(file, fmt_msg, ...) \
  do { \
    fprintf(file, "[%s:%d] %s: ", __FILE__, __LINE__, __func__); \
    fprintf(file, fmt_msg, ##__VA_ARGS__); \
    fprintf(file, "\n"); \
  } while (0)

#define CUTIE_DEBUG_PRINT(fmt_msg, ...) \
  CUTIE_DEBUG_PRINT_RAW(ctx.debug_file, fmt_msg, ##__VA_ARGS__)

#define CUTIE_TRY_RAW(rst, brk_label, succ_debug, err_debug, dbg_msg, ...) \
  do { CutieErrorCode ecode1 = (rst); \
    if (ecode1 != OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

#define CUTIE_TRY_RAW2(rst, unmatch_label, brk_label, succ_debug, unmatch_debug, err_debug, dbg_msg, ...) \
  do { \
    CutieErrorCode ecode1 = (rst); \
    if (ecode1 == RECOVERABLE_ERROR) { \
      if (unmatch_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto unmatch_label; \
    } else if (ecode1 != OK) { \
      ecode = ecode1; \
      if (err_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
      goto brk_label; \
    } else { \
      if (succ_debug) { \
        CUTIE_DEBUG_PRINT(dbg_msg, #rst, ##__VA_ARGS__); \
      } \
    } \
  } while (0)

namespace cutie_ns {

enum CutieErrorCode : int64_t {
#define CUTIE_ERROR_DEF(e, d) e,
#include "cutie-state.txt"
#undef CUTIE_ERROR_DEF
};

char const *ERROR_DESCRIPTION[] = {
#define CUTIE_ERROR_DEF(e, d) d,
#include "cutie-state.txt"
#undef CUTIE_ERROR_DEF
};

struct CutieContext {
  FILE *debug_file;
  void (*debug_file_dtor)(FILE *);
} g_cutie_ctx;

static void closeWrap(FILE *f) {
  (void) fclose(f);
}

static void nothingWithFile(FILE *) {
}

CutieErrorCode initWithTmpFile(CutieContext &ctx) CUTIE_FUNCTION_BEGIN {
  ctx.debug_file = fopen("/tmp/array-detect.log", "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    ecode = RESOURCE_ERROR;
    RET;
  } else {
    ecode = OK;
    RET;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithNamedFile(CutieContext &ctx, char const *debug_file_path) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT_RAW (stderr, "set debug ostream -> %s\n", debug_file_path);
  ctx.debug_file = fopen(debug_file_path, "w");
  ctx.debug_file_dtor = closeWrap;
  if (ctx.debug_file == nullptr) {
    ecode = RESOURCE_ERROR;
    RET;
  } else {
    ecode = OK;
    RET;
  }
} CUTIE_FUNCTION_END

CutieErrorCode initWithStderr(CutieContext &ctx, char const *debug_file_path) CUTIE_FUNCTION_BEGIN {
  (void)debug_file_path; // 避免未使用参数警告
  ctx.debug_file = stderr;
  ctx.debug_file_dtor = nothingWithFile;
  if (ctx.debug_file == nullptr) {
    ecode = RESOURCE_ERROR;
    RET;
  } else {
    ecode = OK;
    RET;
  }
} CUTIE_FUNCTION_END

void deinit(CutieContext &ctx) {
  ctx.debug_file_dtor(ctx.debug_file);
}

} // namespace cutie_ns

// 前向声明
class ArrayDetector;

// 使用cutie_ns命名空间中的类型
typedef cutie_ns::CutieContext CutieContext;
typedef cutie_ns::CutieErrorCode CutieErrorCode;

// 前向声明函数（非命名空间内的函数）
static CutieErrorCode array_detect_execute(CutieContext &ctx);
static CutieErrorCode array_detect_analysis(CutieContext &ctx);
static CutieErrorCode trace_field_assignments(CutieContext &ctx, ArrayDetector* detector);

// ----------------------------------------------------------------------------
// 字段信息结构
// ----------------------------------------------------------------------------

struct FieldInfo {
  const char* field_name;         // 字段名称
  const char* containing_type;    // 包含该字段的类型名称
  tree field_decl;                // 字段声明（用于匹配）
  tree containing_type_tree;     // 包含该字段的类型树
  bool is_pointer;                // 是否是指针类型
  bool is_array_candidate;        // 是否是数组候选
  int source_count;               // 来源数量
  vec<const char*>* sources;     // 赋值来源（函数调用等）
  vec<const char*>* conflicting_assigns; // 冲突的赋值操作
};

// ----------------------------------------------------------------------------
// 数组检测器类
// ----------------------------------------------------------------------------

class ArrayDetector {
private:
  vec<FieldInfo*> m_fields; // 使用GCC框架的vec容器存储字段信息

public:
  ArrayDetector() {
    // 初始化检测器
    m_fields.create(0); // 提供初始大小参数
  }
  
  // 注意：根据code-style.rktd规范，禁用RAII机制，所以这里不使用析构函数
  // 改为提供显式的清理函数
  
  CutieErrorCode add_field(FieldInfo* field_info) CUTIE_FUNCTION_BEGIN {
    if (!field_info) {
      ecode = cutie_ns::OK;
      RET;
    }
    
    // 添加字段信息
    m_fields.safe_push(field_info);
    // 调试信息：输出字段添加情况
    fprintf(stderr, "[ArrayDetector] Added field: %s::%s (total: %u)\n", 
            field_info->containing_type, field_info->field_name, 
            (unsigned)m_fields.length());
    
    ecode = cutie_ns::OK;
    RET;
  } CUTIE_FUNCTION_END
  
  CutieErrorCode analyze_usage() CUTIE_FUNCTION_BEGIN {
    // 分析使用情况，判断是否是数组候选
    for (unsigned int i = 0; i < m_fields.length(); i++) {
      FieldInfo* field = m_fields[i];
      if (!field) continue;
      
      // 只有指针类型才可能是数组候选
      if (!field->is_pointer) {
        field->is_array_candidate = false;
        continue;
      }
      
      // 检查来源数量
      if (field->source_count == 0) {
        // 没有赋值来源，不是数组候选
        field->is_array_candidate = false;
      } else if (field->source_count == 1) {
        // 只有一个来源，检查是否是函数调用
        if (field->sources && field->sources->length() > 0) {
          const char* source = (*field->sources)[0];
          // 如果是函数调用（不是<other-expr>等），可能是数组候选
          if (strcmp(source, "<call-expr>") == 0 || 
              (strstr(source, "allocate") != NULL) ||
              (strcmp(source, "<unknown-call>") != 0 && strcmp(source, "<other-expr>") != 0)) {
            field->is_array_candidate = true;
          } else {
            field->is_array_candidate = false;
          }
        } else {
          field->is_array_candidate = false;
        }
      } else {
        // 多个来源，不是owned数组
        field->is_array_candidate = false;
      }
      
      // 如果有冲突赋值，肯定不是owned数组
      if (field->conflicting_assigns && field->conflicting_assigns->length() > 0) {
        field->is_array_candidate = false;
      }
    }
    
    ecode = cutie_ns::OK;
    RET;
  } CUTIE_FUNCTION_END
  
  CutieErrorCode cleanup() CUTIE_FUNCTION_BEGIN {
    // 显式清理资源，替代析构函数（遵循禁用RAII的规范）
    for (unsigned int i = 0; i < m_fields.length(); i++) {
      FieldInfo* field = m_fields[i];
      if (field) {
        if (field->sources) {
          field->sources->release();
          delete field->sources;
        }
        if (field->conflicting_assigns) {
          field->conflicting_assigns->release();
          delete field->conflicting_assigns;
        }
        // GCC的ggc_alloc分配的内存会自动管理，不需要显式释放
      }
    }
    m_fields.release();
    
    ecode = cutie_ns::OK;
    RET;
  } CUTIE_FUNCTION_END
  
  size_t get_field_count() const {
    return m_fields.length();
  }
  
  FieldInfo* get_field(size_t index) const {
    if (index < m_fields.length()) {
      return m_fields[index];
    }
    return NULL;
  }
};

// ----------------------------------------------------------------------------
// print_results 函数（需要在 ArrayDetector 定义之后）
// ----------------------------------------------------------------------------

namespace cutie_ns {

static CutieErrorCode print_results(CutieContext &ctx, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Printing results");
  
  // 统计信息
  size_t total_fields = detector->get_field_count();
  CUTIE_DEBUG_PRINT("Total fields in detector: %zu", total_fields);
  
  if (total_fields == 0) {
    ecode = OK;
    RET;
  }
  
  // 打开输出文件（写入模式，每次覆盖，因为每个编译单元独立分析）
  FILE* output_file = fopen("array-detect-results.txt", "w");
  if (!output_file) {
    CUTIE_DEBUG_PRINT("Failed to open output file");
    ecode = RESOURCE_ERROR;
    RET;
  }
  
  fprintf(output_file, "=== Array Detection Results ===\n\n");
  
  size_t array_candidates = 0;
  
  // 遍历所有字段，输出分析结果
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo* field = detector->get_field(i);
    if (!field) continue;
    
    fprintf(output_file, "Type: %s, Field: %s\n", field->containing_type, field->field_name);
    fprintf(output_file, "  - Is pointer: %s\n", field->is_pointer ? "yes" : "no");
    
    if (field->is_pointer) {
      fprintf(output_file, "  - Array candidate: %s\n", field->is_array_candidate ? "yes" : "no");
      fprintf(output_file, "  - Source count: %d\n", field->source_count);
      
      if (field->is_array_candidate) {
        array_candidates++;
        
        if (field->source_count == 1 && field->sources && field->sources->length() > 0) {
          fprintf(output_file, "  - Source: %s\n", (*field->sources)[0]);
          fprintf(output_file, "  - Potential owned array (single source)\n");
          fprintf(output_file, "  - DETECTED: This is likely an owned array member!\n");
        }
      } else {
        // 说明为什么不是数组候选
        if (field->source_count == 0) {
          fprintf(output_file, "  - Reason: No assignment sources found\n");
        } else if (field->source_count > 1) {
          fprintf(output_file, "  - Reason: Multiple sources detected (not owned array)\n");
          if (field->sources) {
            for (unsigned int j = 0; j < field->sources->length(); j++) {
              fprintf(output_file, "    Source %d: %s\n", j+1, (*field->sources)[j]);
            }
          }
        } else if (field->conflicting_assigns && field->conflicting_assigns->length() > 0) {
          fprintf(output_file, "  - Reason: Conflicting assignments detected\n");
          for (unsigned int j = 0; j < field->conflicting_assigns->length(); j++) {
            fprintf(output_file, "    Conflicting assign: %s\n", (*field->conflicting_assigns)[j]);
          }
        } else {
          fprintf(output_file, "  - Reason: Source is not a function call\n");
          if (field->sources && field->sources->length() > 0) {
            fprintf(output_file, "    Source: %s\n", (*field->sources)[0]);
          }
        }
      }
    } else {
      fprintf(output_file, "  - Array candidate: no (not a pointer)\n");
    }
    
    fprintf(output_file, "\n");
  }
  
  // 写入汇总信息
  fprintf(output_file, "--- Array Member Detection Results ---\n");
  fprintf(output_file, "Total fields analyzed: %zu\n", total_fields);
  fprintf(output_file, "Array candidates: %zu\n", array_candidates);
  
  // 按类型分组输出
  fprintf(output_file, "\n--- Results by Type ---\n");
  for (size_t i = 0; i < total_fields; i++) {
    FieldInfo* field = detector->get_field(i);
    if (!field) continue;
    
    fprintf(output_file, "\nType: %s\n", field->containing_type);
    fprintf(output_file, "  Field: %s - Is pointer: %s - Is array candidate: %s\n",
            field->field_name,
            field->is_pointer ? "Yes" : "No",
            field->is_array_candidate ? "Yes" : "No");
  }
  
  fclose(output_file);
  ecode = OK;
  RET;
} CUTIE_FUNCTION_END

} // namespace cutie_ns

// ----------------------------------------------------------------------------
// 辅助函数
// ----------------------------------------------------------------------------

  // 获取类型名称
static const char* get_type_name(tree type) {
  if (!type) return "<unknown>";
  
  if (TYPE_NAME(type)) {
    if (TREE_CODE(TYPE_NAME(type)) == IDENTIFIER_NODE) {
      return IDENTIFIER_POINTER(TYPE_NAME(type));
    } else if (TREE_CODE(TYPE_NAME(type)) == TYPE_DECL) {
      tree name = DECL_NAME(TYPE_NAME(type));
      if (name) {
        return IDENTIFIER_POINTER(name);
      }
    }
  }
  
  return "<unnamed>";
}

// 获取字段名称
static const char* get_field_name(tree field) {
  if (!field) return "<unknown>";
  if (DECL_NAME(field)) {
    return IDENTIFIER_POINTER(DECL_NAME(field));
  }
  return "<unnamed>";
}

// 检查类型是否是指针类型
static bool is_pointer_type(tree type) {
  if (!type) return false;
  return TREE_CODE(type) == POINTER_TYPE;
}

// 检查是否是字段访问（COMPONENT_REF）
static bool is_field_access(tree expr, tree* field_decl_out, tree* object_out) {
  if (!expr) return false;
  
  if (TREE_CODE(expr) == COMPONENT_REF) {
    *field_decl_out = TREE_OPERAND(expr, 1);
    *object_out = TREE_OPERAND(expr, 0);
    return true;
  }
  return false;
}

// 获取函数调用名称
static const char* get_call_name(gimple* stmt) {
  if (!stmt || gimple_code(stmt) != GIMPLE_CALL) {
    return NULL;
  }
  
  tree fndecl = gimple_call_fndecl(stmt);
  if (fndecl && DECL_NAME(fndecl)) {
    return IDENTIFIER_POINTER(DECL_NAME(fndecl));
  }
  
  // 如果是间接调用，尝试获取函数指针的名称
  tree fn = gimple_call_fn(stmt);
  if (fn && TREE_CODE(fn) == ADDR_EXPR) {
    tree decl = TREE_OPERAND(fn, 0);
    if (decl && DECL_NAME(decl)) {
      return IDENTIFIER_POINTER(DECL_NAME(decl));
    }
  }
  
  return "<unknown-call>";
}

// 检查是否是函数调用表达式（未使用，但保留以备将来使用）
// static bool is_function_call(tree expr) {
//   if (!expr) return false;
//   return TREE_CODE(expr) == CALL_EXPR;
// }

// 获取表达式的字符串表示（简化版）
static const char* expr_to_string(tree expr) {
  if (!expr) return "<null>";
  
  if (TREE_CODE(expr) == CALL_EXPR) {
    return "<call-expr>";
  } else if (TREE_CODE(expr) == ADDR_EXPR) {
    return "<addr-expr>";
  } else if (TREE_CODE(expr) == VAR_DECL && DECL_NAME(expr)) {
    return IDENTIFIER_POINTER(DECL_NAME(expr));
  } else if (TREE_CODE(expr) == COMPONENT_REF) {
    return "<component-ref>";
  }
  
  return "<other-expr>";
}

// ----------------------------------------------------------------------------
// 业务逻辑函数
// ----------------------------------------------------------------------------

// 辅助函数：处理一个类型，提取其字段
static CutieErrorCode process_type_fields(CutieContext &ctx, tree type, ArrayDetector* detector, hash_set<tree>* processed_types) CUTIE_FUNCTION_BEGIN {
  if (!type) {
    ecode = cutie_ns::OK;
    RET;
  }
  
  // 只处理结构体/类类型
  if (TREE_CODE(type) != RECORD_TYPE && TREE_CODE(type) != UNION_TYPE) {
    ecode = cutie_ns::OK;
    RET;
  }
  
  // 检查是否已处理过
  if (!processed_types->add(type)) {
    // 已处理过，跳过
    ecode = cutie_ns::OK;
    RET;
  }
  
  const char* type_name = get_type_name(type);
  CUTIE_DEBUG_PRINT("Processing type: %s", type_name);
  
  // 遍历字段
  tree field;
  for (field = TYPE_FIELDS(type); field; field = DECL_CHAIN(field)) {
    if (TREE_CODE(field) != FIELD_DECL) continue;
    
    const char* field_name = get_field_name(field);
    
    // 跳过虚函数表指针
    if (strstr(field_name, "_vptr") != NULL) {
      continue;
    }
    
    tree field_type = TREE_TYPE(field);
    bool is_ptr = is_pointer_type(field_type);
    
    CUTIE_DEBUG_PRINT("  Field: %s, is_pointer: %d", field_name, is_ptr ? 1 : 0);
    
    // 创建字段信息（使用GCC的内存分配）
    FieldInfo* field_info = (FieldInfo*)ggc_alloc<FieldInfo>();
    if (!field_info) {
      ecode = cutie_ns::MEMORY_ERROR;
      RET;
    }
    // 初始化字段
    memset(field_info, 0, sizeof(FieldInfo));
    
    field_info->field_name = field_name;
    field_info->containing_type = type_name;
    field_info->field_decl = field;
    field_info->containing_type_tree = type;
    field_info->is_pointer = is_ptr;
    field_info->is_array_candidate = false;
    field_info->source_count = 0;
    field_info->sources = new vec<const char*>();
    field_info->sources->create(0);
    field_info->conflicting_assigns = new vec<const char*>();
    field_info->conflicting_assigns->create(0);
    
    CutieErrorCode tmp_ecode = detector->add_field(field_info);
    if (tmp_ecode != cutie_ns::OK) {
      field_info->sources->release();
      delete field_info->sources;
      field_info->conflicting_assigns->release();
      delete field_info->conflicting_assigns;
      ecode = tmp_ecode;
      RET;
    }
  }
  
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

// 收集所有类型和字段
static CutieErrorCode collect_all_types_and_fields(CutieContext &ctx, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Collecting all types and fields");
  
  // 使用hash_set来避免重复处理同一类型
  hash_set<tree> processed_types;
  processed_types.create_ggc(0);
  
  // 方法：遍历所有函数，从函数体中的字段访问提取类型
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun();
    if (!fn) continue;
    
    // 遍历函数中的语句，查找字段访问
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        
        // 检查赋值语句中的类型
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          tree lhs = gimple_assign_lhs(stmt);
          
          // 检查是否是字段访问
          tree field_decl = NULL_TREE;
          tree object = NULL_TREE;
          if (is_field_access(lhs, &field_decl, &object)) {
            // 找到字段访问，获取包含类型
            tree object_type = TREE_TYPE(object);
            if (!object_type) continue;
            
            // 如果是引用类型，获取其基础类型
            if (TREE_CODE(object_type) == REFERENCE_TYPE) {
              object_type = TREE_TYPE(object_type);
            }
            
            // 如果是指针类型，获取其指向的类型
            if (TREE_CODE(object_type) == POINTER_TYPE) {
              object_type = TREE_TYPE(object_type);
            }
            
            tree containing_type = TYPE_MAIN_VARIANT(object_type);
            if (containing_type) {
              CutieErrorCode tmp_ecode = process_type_fields(ctx, containing_type, detector, &processed_types);
              if (tmp_ecode != cutie_ns::OK) {
                ecode = tmp_ecode;
                RET;
              }
            }
          }
        }
      }
    }
  }
  
  // hash_set使用GCC的垃圾回收，不需要显式释放
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

// 分析字段赋值
static CutieErrorCode analyze_field_assignments_in_functions(CutieContext &ctx, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  CUTIE_DEBUG_PRINT("Analyzing field assignments in functions");
  
  // 遍历所有函数
  struct cgraph_node* node;
  FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node) {
    function* fn = node->get_fun();
    if (!fn) continue;
    
    const char* func_name = node->name();
    CUTIE_DEBUG_PRINT("Analyzing function: %s", func_name);
    
    // 遍历函数中的所有基本块
    basic_block bb;
    FOR_EACH_BB_FN(bb, fn) {
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple* stmt = gsi_stmt(gsi);
        
        // 检查是否是赋值语句
        if (gimple_code(stmt) == GIMPLE_ASSIGN) {
          tree lhs = gimple_assign_lhs(stmt);
          tree rhs = gimple_assign_rhs1(stmt);
          
          // 检查左值是否是字段访问
          tree field_decl = NULL_TREE;
          tree object = NULL_TREE;
          if (is_field_access(lhs, &field_decl, &object)) {
            const char* field_name = get_field_name(field_decl);
            CUTIE_DEBUG_PRINT("Found field assignment: %s", field_name);
            
            // 查找对应的FieldInfo
            FieldInfo* field_info = NULL;
            for (size_t i = 0; i < detector->get_field_count(); i++) {
              FieldInfo* fi = detector->get_field(i);
              if (fi && fi->field_decl == field_decl) {
                field_info = fi;
                break;
              }
            }
            
            if (!field_info) {
              // 字段不在我们的列表中，尝试提取类型并添加
              tree object_type = TREE_TYPE(object);
              if (object_type) {
                // 如果是引用类型，获取其基础类型
                if (TREE_CODE(object_type) == REFERENCE_TYPE) {
                  object_type = TREE_TYPE(object_type);
                }
                // 如果是指针类型，获取其指向的类型
                if (TREE_CODE(object_type) == POINTER_TYPE) {
                  object_type = TREE_TYPE(object_type);
                }
                
                tree containing_type = TYPE_MAIN_VARIANT(object_type);
                if (containing_type) {
                  // 创建临时hash_set来处理类型
                  hash_set<tree> temp_processed;
                  temp_processed.create_ggc(0);
                  CutieErrorCode tmp_ecode = process_type_fields(ctx, containing_type, detector, &temp_processed);
                  if (tmp_ecode == cutie_ns::OK) {
                    // 重新查找字段信息
                    for (size_t i = 0; i < detector->get_field_count(); i++) {
                      FieldInfo* fi = detector->get_field(i);
                      if (fi && fi->field_decl == field_decl) {
                        field_info = fi;
                        break;
                      }
                    }
                  }
                }
              }
              
              if (!field_info) {
                // 仍然找不到，跳过
                continue;
              }
            }
            
            // 分析右值来源
            const char* source = NULL;
            bool is_call = false;
            
            // 检查是否是函数调用
            if (TREE_CODE(rhs) == CALL_EXPR) {
              // 直接调用表达式
              source = "<call-expr>";
              is_call = true;
            } else if (gimple_code(stmt) == GIMPLE_CALL) {
              // GIMPLE调用语句
              source = get_call_name(stmt);
              if (source) {
                is_call = true;
              }
            } else {
              // 其他类型的表达式
              source = expr_to_string(rhs);
              is_call = false;
            }
            
            if (source) {
              CUTIE_DEBUG_PRINT("  Source: %s (is_call: %d)", source, is_call ? 1 : 0);
              
              // 记录来源
              field_info->sources->safe_push(source);
              field_info->source_count++;
              
              // 如果不是函数调用，记录为冲突赋值
              if (!is_call) {
                field_info->conflicting_assigns->safe_push(source);
              }
            }
          }
        }
        // 检查是否是GIMPLE_CALL语句（可能是通过调用赋值）
        else if (gimple_code(stmt) == GIMPLE_CALL) {
          // 这里可以处理通过函数调用返回值的赋值
          // 简化处理：暂时跳过
        }
      }
    }
  }
  
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

static CutieErrorCode trace_field_assignments(CutieContext &ctx, ArrayDetector* detector) CUTIE_FUNCTION_BEGIN {
  // 追踪字段的赋值操作
  CUTIE_DEBUG_PRINT("Tracing field assignments");
  
  // 第一步：收集所有类型和字段
  CutieErrorCode tmp_ecode = collect_all_types_and_fields(ctx, detector);
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    RET;
  }
  
  // 第二步：分析字段赋值
  tmp_ecode = analyze_field_assignments_in_functions(ctx, detector);
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    RET;
  }
  
  // 第三步：分析使用情况，判断是否是数组候选
  tmp_ecode = detector->analyze_usage();
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    RET;
  }
  
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

// 已经在文件开头定义了这些类型别名

static CutieErrorCode array_detect_analysis(CutieContext &ctx) CUTIE_FUNCTION_BEGIN {
  // 重命名标签以避免与宏中的cleanup冲突
  CUTIE_DEBUG_PRINT("Starting array member detection analysis");
  
  // 创建数组检测器
  ArrayDetector detector;
  
  // 执行分析
  CutieErrorCode tmp_ecode = trace_field_assignments(ctx, &detector);
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    goto analysis_cleanup;
  }
  
  tmp_ecode = cutie_ns::print_results(ctx, &detector);
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    goto analysis_cleanup;
  }
  
  analysis_cleanup:
  // 使用显式清理函数替代析构函数
  detector.cleanup();
  
  CUTIE_DEBUG_PRINT("Array member detection analysis completed");
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

static CutieErrorCode array_detect_execute(CutieContext &ctx) CUTIE_FUNCTION_BEGIN {
  // 使用不同的标签名来避免与宏中的cleanup冲突
  CutieErrorCode tmp_ecode = cutie_ns::initWithStderr(ctx, "");
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    goto exec_cleanup;
  }
  
  tmp_ecode = array_detect_analysis(ctx);
  if (tmp_ecode != cutie_ns::OK) {
    ecode = tmp_ecode;
    goto exec_cleanup;
  }
  
  exec_cleanup:
  cutie_ns::deinit(ctx);
  ecode = cutie_ns::OK;
  RET;
} CUTIE_FUNCTION_END

// ----------------------------------------------------------------------------
// Pass 注册结构
// ----------------------------------------------------------------------------

namespace {

const pass_data array_detect_pass_data = {
  .type = IPA_PASS,
  .name = "array-detect-wpa-pass",
  .optinfo_flags = OPTGROUP_NONE,
  .tv_id = TV_NONE,
  .properties_required = 0,
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0,
};

class pass_array_detect : public ipa_opt_pass_d {
 public:
  pass_array_detect(gcc::context* ctxt)
      : ipa_opt_pass_d(array_detect_pass_data, ctxt,
                       NULL,  // generate_summary
                       NULL,  // write_summary
                       NULL,  // read_summary
                       NULL,  // write_optimization_summary
                       NULL,  // read_optimization_summary
                       NULL,  // stmt_fixup
                       0,     // function_transform_todo_flags_start
                       NULL,  // function_transform
                       NULL)  // variable_transform
  {}

  opt_pass* clone() override { return new pass_array_detect(g); }

  unsigned int execute(function*) override {
    // Regular IPA passes in WPA mode call execute() with NULL function
    array_detect_execute(cutie_ns::g_cutie_ctx);
    return 0; // IPA passes return 0 for success
  }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// 插件初始化 - 仅在非独立模式下定义
// ----------------------------------------------------------------------------

int plugin_init(struct plugin_name_args* plugin_info,
                struct plugin_gcc_version* version) {
  // 版本检查
  if (!plugin_default_version_check(version, &gcc_version)) {
    return 1;
  }

  struct register_pass_info pass_info;
  pass_info.pass = new pass_array_detect(g);
  pass_info.reference_pass_name = "cdtor";  // 在此 pass 之后插入
  pass_info.ref_pass_instance_number = 1;         // 符合规则
  pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback(plugin_info->base_name,
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,
                    &pass_info);

  return 0;
}

int plugin_is_GPL_compatible = 1;
