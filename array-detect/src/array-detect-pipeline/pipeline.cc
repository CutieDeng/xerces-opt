#include "pipeline.hh"
#include "field-write-collector.hh"
#include "write-operation-trace.hh"
#include "source-use-analysis.hh"
#include "array-detector.hh"
#include "info-print.hh"

namespace array_detect_ns {

// ============================================================================
// Pipeline 实现：简单暴力地调用下层模块
// ============================================================================

ArrayDetectErrorCode runArrayDetectionPipeline (
  AD_FUNC_ARGS,
  ArrayDetector &detector
) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("=== Starting Array Detection Pipeline ===");
  
  // 第一步：提取字段信息
  // 遍历所有函数，提取类型和字段信息，填充 detector.m_type_field_writes
  AD_DEBUG_PRINT ("Step 1: Extracting field information");
  AD_TRY (collectTypesAndFields (detector, AD_ARGS));
  
  // 第二步：分析信息
  // 追踪字段赋值，分析字段的赋值来源，判断是否为 owned 数组
  AD_DEBUG_PRINT ("Step 2: Analyzing field assignments");
  AD_TRY (traceFieldAssignments (detector, AD_ARGS));

  // 第三步：源操作数使用分析
  // 分析 write-operation 的源操作数使用流和逃逸情况
  AD_DEBUG_PRINT ("Step 3: Analyzing source operand uses and escapes");

  // 获取逃逸规则配置
  SourceUseEscapeRules escape_rules = getDefaultEscapeRules();

  // 统计信息
  unsigned int total_analyzed = 0;
  unsigned int total_escaped = 0;

  // 遍历所有 write-operation 结果，分析源操作数使用
  for (auto it = detector.m_type_field_writes.begin();
       it != detector.m_type_field_writes.end();
       ++it) {
    FieldWriteSourceInfo* write_info = it->second;
    if (!write_info) continue;

    // 执行源操作数使用分析
    SourceUseAnalysisResult* use_result =
      analyzeFromWriteSourceInfo(write_info, escape_rules);

    if (use_result) {
      total_analyzed++;
      if (use_result->has_escape) {
        total_escaped++;
      }

      // 调试输出（可选）
      if (ctx.debug_file && use_result->has_escape) {
        AD_DEBUG_PRINT ("  Field %s::%s source escapes via %s (%u uses, %u escapes)",
                        it->first.type_name,
                        it->first.field_name,
                        getEscapeKindString(use_result->dominant_escape_kind),
                        use_result->total_use_count,
                        use_result->escape_count);
      }
    }
  }

  AD_DEBUG_PRINT ("Source use analysis complete: %u fields analyzed, %u with escapes",
                  total_analyzed, total_escaped);

  // 第四步：输出信息
  // 生成并输出分析报告到文件
  AD_DEBUG_PRINT ("Step 4: Printing results");
  AD_TRY (printResults (AD_ARGS, detector));
  
  AD_DEBUG_PRINT ("=== Array Detection Pipeline Completed ===");
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 顶层入口：创建检测器并执行分析
// ============================================================================

ArrayDetectErrorCode runArrayDetectorAnalysis (AD_FUNC_ARGS) AD_FUNCTION_BEGIN {
  AD_DEBUG_PRINT ("Starting array member detection analysis");
  ArrayDetector detector;
  // 延迟初始化：在使用前分配 vec 指针
  AD_TRY (init (detector, AD_ARGS));
  // 直接调用 pipeline 执行完整流程
  AD_TRY_LABEL (runArrayDetectionPipeline (AD_ARGS, detector), analysis_cleanup);
  analysis_cleanup:
  // 清理资源
  deinit (detector, AD_ARGS);
  AD_DEBUG_PRINT ("Array member detection analysis completed");
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detect_ns

