#include "field-dataflow-analysis.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// 构建字段数据流图
ArrayDetectErrorCode buildDataFlowGraph(
  AD_FUNC_ARGS,
  tree field_decl,
  function* fn,
  vec<WriteOperation*> const &field_writes,
  DataFlowGraph &graph
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  // 初始化图
  memset(&graph, 0, sizeof(DataFlowGraph));
  graph.field_decl = field_decl;
  graph.fn = fn;
  graph.nodes = ggc_alloc<vec<DataFlowNode*>>();
  graph.nodes->create(0);
  graph.edges = ggc_alloc<vec<DataFlowEdge*>>();
  graph.edges->create(0);
  
  if (!field_decl || !fn) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("Building data flow graph for field");
  
  // 创建定义节点（从写入操作）
  for (unsigned int i = 0; i < field_writes.length(); i++) {
    WriteOperation* write_op = field_writes[i];
    if (!write_op || !write_op->stmt) {
      continue;
    }
    
    DataFlowNode* def_node = ggc_alloc<DataFlowNode>();
    memset(def_node, 0, sizeof(DataFlowNode));
    def_node->type = DEF_FIELD_WRITE;
    def_node->stmt = write_op->stmt;
    def_node->field_decl = field_decl;
    def_node->location = write_op->location;
    
    // 设置描述
    if (write_op->is_from_call) {
      def_node->type = DEF_FIELD_CALL;
      def_node->description = ggc_strdup("field write from call");
    } else {
      def_node->description = ggc_strdup("field write");
    }
    
    // 设置 SSA 变量（如果有）
    if (write_op->rhs_value && TREE_CODE(write_op->rhs_value) == SSA_NAME) {
      def_node->ssa_var = write_op->rhs_value;
    }
    
    graph.nodes->safe_push(def_node);
  }
  
  AD_DEBUG_PRINT("  Created %u definition nodes", graph.nodes->length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 前向数据流分析（从定义到使用）
ArrayDetectErrorCode forwardDataFlowAnalysis(
  AD_FUNC_ARGS,
  DataFlowGraph const &graph,
  vec<DataFlowNode*> &use_nodes
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  use_nodes.create(0);
  
  if (!graph.nodes || graph.nodes->length() == 0) {
    AD_RETURNE(OK);
  }
  
  AD_DEBUG_PRINT("Performing forward data flow analysis");
  
  // 遍历所有定义节点
  for (unsigned int i = 0; i < graph.nodes->length(); i++) {
    DataFlowNode* def_node = (*graph.nodes)[i];
    if (!def_node || (def_node->type != DEF_FIELD_WRITE && def_node->type != DEF_FIELD_CALL)) {
      continue;
    }
    
    // 如果定义节点有 SSA 变量，追踪其使用
    if (def_node->ssa_var) {
      
      // 遍历函数中的所有语句，查找字段读取
      basic_block bb;
      FOR_EACH_BB_FN(bb, graph.fn) {
        gimple_stmt_iterator gsi;
        for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
          gimple* stmt = gsi_stmt(gsi);
          
          // 检查是否为字段读取
          if (gimple_code(stmt) == GIMPLE_ASSIGN) {
            tree rhs = gimple_assign_rhs1(stmt);
            
            // 检查右值是否使用了字段
            tree field_decl_check = NULL_TREE;
            tree object = NULL_TREE;
            bool is_field_access;
            AD_TRY(gcc_ext_util::is_field_access(AD_ARGS, rhs, &field_decl_check, &object, is_field_access));
            
            if (is_field_access && field_decl_check == graph.field_decl) {
              // 找到字段读取使用
              DataFlowNode* use_node = ggc_alloc<DataFlowNode>();
              memset(use_node, 0, sizeof(DataFlowNode));
              use_node->type = USE_FIELD_READ;
              use_node->stmt = stmt;
              use_node->field_decl = graph.field_decl;
              use_node->location = gimple_location(stmt);
              use_node->description = ggc_strdup("field read");
              
              use_nodes.safe_push(use_node);
              
              // 创建边
              DataFlowEdge* edge = ggc_alloc<DataFlowEdge>();
              memset(edge, 0, sizeof(DataFlowEdge));
              edge->from = def_node;
              edge->to = use_node;
              edge->type = EDGE_DEF_USE;
              edge->description = ggc_strdup("def-use");
              
              // 注意：这里需要修改 graph，但 graph 是 const，需要调整设计
              // 暂时跳过边的添加
            }
          }
        }
      }
    }
  }
  
  AD_DEBUG_PRINT("  Found %u use nodes", use_nodes.length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 后向数据流分析（从使用到定义）
ArrayDetectErrorCode backwardDataFlowAnalysis(
  AD_FUNC_ARGS,
  DataFlowGraph const &graph,
  DataFlowNode* use_node,
  vec<DataFlowNode*> &def_nodes
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  def_nodes.create(0);
  
  if (!use_node || !graph.nodes) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("Performing backward data flow analysis");
  
  // 找到所有定义节点
  for (unsigned int i = 0; i < graph.nodes->length(); i++) {
    DataFlowNode* node = (*graph.nodes)[i];
    if (node && (node->type == DEF_FIELD_WRITE || node->type == DEF_FIELD_CALL)) {
      // 简单实现：如果定义节点在同一个函数中，且位置在使用节点之前
      if (node->field_decl == graph.field_decl) {
        def_nodes.safe_push(node);
      }
    }
  }
  
  AD_DEBUG_PRINT("  Found %u definition nodes", def_nodes.length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 追踪字段值的来源
ArrayDetectErrorCode traceValueSource(
  AD_FUNC_ARGS,
  tree field_decl,
  WriteOperation* write_op,
  vec<ValueSource*> &sources
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  sources.create(0);
  
  if (!field_decl || !write_op) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("Tracing value source for field write");
  
  tree current_value = write_op->rhs_value;
  int depth = 0;
  const int MAX_DEPTH = 10;  // 防止无限循环
  
  while (current_value && depth < MAX_DEPTH) {
    depth++;
    
    if (TREE_CODE(current_value) == SSA_NAME) {
      gimple* def_stmt = SSA_NAME_DEF_STMT(current_value);
      
      if (!def_stmt) {
        break;
      }
      
      if (gimple_code(def_stmt) == GIMPLE_CALL) {
        // 函数调用来源
        ValueSource* source = ggc_alloc<ValueSource>();
        memset(source, 0, sizeof(ValueSource));
        source->type = SOURCE_CALL;
        source->stmt = def_stmt;
        source->value = current_value;
        source->description = ggc_strdup("function call");
        sources.safe_push(source);
        
        // 继续追踪返回值
        tree lhs = gimple_call_lhs(def_stmt);
        if (lhs && TREE_CODE(lhs) == SSA_NAME) {
          current_value = lhs;
          continue;
        }
        break;
      } else if (gimple_code(def_stmt) == GIMPLE_ASSIGN) {
        // 赋值来源
        tree rhs = gimple_assign_rhs1(def_stmt);
        
        if (TREE_CODE(rhs) == SSA_NAME) {
          current_value = rhs;  // 继续追踪
          continue;
        } else if (TREE_CODE(rhs) == INTEGER_CST) {
          ValueSource* source = ggc_alloc<ValueSource>();
          memset(source, 0, sizeof(ValueSource));
          source->type = SOURCE_CONSTANT;
          source->stmt = def_stmt;
          source->value = rhs;
          source->description = ggc_strdup("constant");
          sources.safe_push(source);
          break;
        } else {
          ValueSource* source = ggc_alloc<ValueSource>();
          memset(source, 0, sizeof(ValueSource));
          source->type = SOURCE_EXPRESSION;
          source->stmt = def_stmt;
          source->value = rhs;
          source->description = ggc_strdup("expression");
          sources.safe_push(source);
          break;
        }
      } else if (gimple_code(def_stmt) == GIMPLE_PHI) {
        // PHI 节点
        ValueSource* source = ggc_alloc<ValueSource>();
        memset(source, 0, sizeof(ValueSource));
        source->type = SOURCE_PHI;
        source->stmt = def_stmt;
        source->value = current_value;
        source->description = ggc_strdup("phi node");
        sources.safe_push(source);
        break;
      } else {
        break;
      }
    } else if (TREE_CODE(current_value) == INTEGER_CST) {
      ValueSource* source = ggc_alloc<ValueSource>();
      memset(source, 0, sizeof(ValueSource));
      source->type = SOURCE_CONSTANT;
      source->stmt = NULL;
      source->value = current_value;
      source->description = ggc_strdup("constant");
      sources.safe_push(source);
      break;
    } else {
      ValueSource* source = ggc_alloc<ValueSource>();
      memset(source, 0, sizeof(ValueSource));
      source->type = SOURCE_EXPRESSION;
      source->stmt = NULL;
      source->value = current_value;
      source->description = ggc_strdup("expression");
      sources.safe_push(source);
      break;
    }
  }
  
  AD_DEBUG_PRINT("  Traced %u sources (depth: %d)", sources.length(), depth);
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 追踪字段值的去向
ArrayDetectErrorCode traceValueSink(
  AD_FUNC_ARGS,
  tree field_decl,
  gimple* read_stmt,
  vec<ValueSink*> &sinks
) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  
  sinks.create(0);
  
  if (!field_decl || !read_stmt) {
    AD_RETURNE(INVALID_ARGUMENT);
  }
  
  AD_DEBUG_PRINT("Tracing value sink for field read");
  
  // 简单实现：查找字段的所有后续写入操作
  // 更完整的实现需要追踪 SSA 使用链
  
  // 这里暂时返回空列表，需要更复杂的实现
  // 可以使用 GCC 的 SSA 使用链 API
  
  AD_DEBUG_PRINT("  Traced %u sinks", sinks.length());
  
  AD_RETURNE(OK);
} AD_FUNCTION_END

} // namespace array_detect_ns
