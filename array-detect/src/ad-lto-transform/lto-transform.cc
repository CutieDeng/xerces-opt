#include "lto-transform.hh"

#include <stdlib.h>
#include <cstring>

#include "array-detect-context-gcc.hh"
#include "prelude.hh"
#include "context-init.hh"
#include "context.hh"
#include "pipeline.hh"

// GCC headers for GIMPLE traversal
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-pass.h"
#include "cgraph.h"
#include "gimple-walk.h"

namespace array_detect_ns {

// ============================================================================
// Global transform context (initialized once per LTRANS)
// ============================================================================

static LtoTransformContext* g_transform_ctx = nullptr;
static bool g_ltrans_analysis_done = false;
static bool g_ltrans_aggregated_written = false;

static void ensureLtransSummariesFromAnalysis (AD_FUNC_ARGS) {
  (void) gcc_ctx;
  if (hasLtransLtoSummaries ()) return;
  if (g_ltrans_analysis_done) return;
  g_ltrans_analysis_done = true;

  AD_DEBUG_PRINT ("[ensureLtransSummariesFromAnalysis] fallback analysis");

  // NOTE: LTO summary conversion from UnifiedFieldAnalysisResult has been removed.
  // The result-aggregator module was deleted. Fallback analysis for LTRANS is disabled.
  // To re-enable, implement conversion from the new capacity analysis modules.
  AD_DEBUG_PRINT ("[ensureLtransSummariesFromAnalysis] fallback analysis disabled - result-aggregator module removed");
}

// ============================================================================
// Key type alias for cleaner code
// ============================================================================

typedef std::pair<const char*, const char*> TypeFieldKey;

// ============================================================================
// Context initialization
// ============================================================================

bool initLtoTransformContext (AD_FUNC_ARGS, LtoTransformContext* transform_ctx) {
  if (!transform_ctx) return false;

  AD_DEBUG_PRINT ("[initLtoTransformContext] ENTRY");

  // Create hash map
  transform_ctx->owned_fields = new OwnedFieldMap ();
  transform_ctx->fields_transformed = 0;
  transform_ctx->accesses_found = 0;
  transform_ctx->accesses_transformed = 0;

  unsigned int owned_count = 0;

  // Load summaries from LTO sections (populated by ipa_read_summary).
  ::array_detect_ns::readArrayDetectLtoSummarySections (AD_ARGS);
  bool loaded_from_section = hasLtransLtoSummaries ();
  // NOTE: aggregateLtransSummaries was removed along with result-aggregator module.
  // Using getLtransLtoSummaries instead.
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = getLtransLtoSummaries ();
  if (!summaries || summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[initLtoTransformContext] WARNING: LTO summary blob missing; falling back to LTRANS analysis");
    ensureLtransSummariesFromAnalysis (AD_ARGS);
    summaries = getLtransLtoSummaries ();
    if (!summaries || summaries->is_empty ()) {
      summaries = getWpaLtoSummaries ();
    }
  }
  if (summaries && !summaries->is_empty ()) {
    AD_DEBUG_PRINT ("[initLtoTransformContext] Using %s summaries, count=%u",
                    loaded_from_section ? "LTO section" : "analysis",
                    summaries->length ());

    for (unsigned i = 0; i < summaries->length (); i++) {
      LtoUnifiedResultSummary* s = (*summaries)[i];
      if (!s) continue;

      // Only include owned=yes fields
      if (s->owned_verdict != OWNED_YES) continue;

      char const* tname = s->type_name ? s->type_name : "";
      char const* fname = s->ptr_field_name ? s->ptr_field_name : "";

      TypeFieldKey key (tname, fname);

      // Check if already exists
      if (transform_ctx->owned_fields->get (key) != nullptr) continue;

      // Insert with summary pointer
      transform_ctx->owned_fields->put (key, s);
      owned_count++;

      AD_DEBUG_PRINT ("[initLtoTransformContext] Added owned field from LTO: %s::%s", tname, fname);
    }
  }

  transform_ctx->fields_transformed = owned_count;

  AD_DEBUG_PRINT ("[initLtoTransformContext] EXIT, owned_count=%u", owned_count);

  return owned_count > 0;
}

void deinitLtoTransformContext (LtoTransformContext* ctx) {
  if (!ctx) return;
  if (ctx->owned_fields) {
    delete ctx->owned_fields;
    ctx->owned_fields = nullptr;
  }
}

LtoUnifiedResultSummary* lookupOwnedField (
  LtoTransformContext* ctx,
  char const* type_name,
  char const* field_name
) {
  if (!ctx || !ctx->owned_fields) return nullptr;
  if (!type_name || !field_name) return nullptr;

  TypeFieldKey key (type_name, field_name);
  OwnedFieldValue* val = ctx->owned_fields->get (key);
  return val ? *val : nullptr;
}

bool isFieldOwned (
  LtoTransformContext* ctx,
  char const* type_name,
  char const* field_name
) {
  if (!ctx || !ctx->owned_fields) return false;
  if (!type_name || !field_name) return false;

  TypeFieldKey key (type_name, field_name);
  return ctx->owned_fields->get (key) != nullptr;
}

void printOwnedFieldTable (AD_FUNC_ARGS, LtoTransformContext* transform_ctx) {
  (void) gcc_ctx;
  if (!transform_ctx || !transform_ctx->owned_fields) return;

  AD_DEBUG_PRINT ("=== Owned Field Table ===");
  AD_DEBUG_PRINT ("Total owned fields: %u", transform_ctx->fields_transformed);

  // Iterate hash_map using iterator
  for (auto iter = transform_ctx->owned_fields->begin ();
       iter != transform_ctx->owned_fields->end (); ++iter) {
    auto entry = *iter;
    AD_DEBUG_PRINT ("  %s::%s",
                    entry.first.first ? entry.first.first : "<null>",
                    entry.first.second ? entry.first.second : "<null>");
  }

  AD_DEBUG_PRINT ("=========================");
}

// ============================================================================
// Helper: Extract type name from tree
// ============================================================================

static char const* getTypeName (tree type) {
  if (!type) return nullptr;

  // Get the main variant for consistent naming
  type = TYPE_MAIN_VARIANT (type);

  // Try TYPE_NAME
  tree type_name = TYPE_NAME (type);
  if (type_name) {
    if (TREE_CODE (type_name) == IDENTIFIER_NODE) {
      return IDENTIFIER_POINTER (type_name);
    }
    if (TREE_CODE (type_name) == TYPE_DECL && DECL_NAME (type_name)) {
      return IDENTIFIER_POINTER (DECL_NAME (type_name));
    }
  }

  // For anonymous types, try getting the tag
  if (TREE_CODE (type) == RECORD_TYPE || TREE_CODE (type) == UNION_TYPE) {
    tree tag = TYPE_IDENTIFIER (type);
    if (tag) {
      return IDENTIFIER_POINTER (tag);
    }
  }

  return nullptr;
}

// ============================================================================
// Helper: Extract field name from FIELD_DECL
// ============================================================================

static char const* getFieldName (tree field) {
  if (!field) return nullptr;
  if (TREE_CODE (field) != FIELD_DECL) return nullptr;
  if (!DECL_NAME (field)) return nullptr;
  return IDENTIFIER_POINTER (DECL_NAME (field));
}

// ============================================================================
// Helper: Check if expression is a field access and extract info
// ============================================================================

struct FieldAccessInfo {
  tree base_object;       // The object being accessed
  tree record_type;       // The RECORD_TYPE of the object
  tree field_decl;        // The FIELD_DECL being accessed
  char const* type_name;
  char const* field_name;
  bool is_read;           // true = read, false = write
};

static bool extractFieldAccess (tree expr, FieldAccessInfo* info) {
  if (!expr || !info) return false;

  memset (info, 0, sizeof (*info));

  // Handle COMPONENT_REF: base.field
  if (TREE_CODE (expr) == COMPONENT_REF) {
    info->base_object = TREE_OPERAND (expr, 0);
    info->field_decl = TREE_OPERAND (expr, 1);

    if (TREE_CODE (info->field_decl) != FIELD_DECL) return false;

    // Get the record type from the base object
    tree base_type = TREE_TYPE (info->base_object);
    if (POINTER_TYPE_P (base_type)) {
      base_type = TREE_TYPE (base_type);
    }
    info->record_type = TYPE_MAIN_VARIANT (base_type);

    info->type_name = getTypeName (info->record_type);
    info->field_name = getFieldName (info->field_decl);

    return info->type_name && info->field_name;
  }

  // Handle MEM_REF with COMPONENT_REF inside (common in optimized code)
  if (TREE_CODE (expr) == MEM_REF) {
    tree inner = TREE_OPERAND (expr, 0);
    // Unwrap ADDR_EXPR
    if (TREE_CODE (inner) == ADDR_EXPR) {
      inner = TREE_OPERAND (inner, 0);
    }
    if (TREE_CODE (inner) == COMPONENT_REF) {
      return extractFieldAccess (inner, info);
    }
  }

  return false;
}

// ============================================================================
// GIMPLE statement analysis: find field accesses
// ============================================================================

struct TransformWalkData {
  LtoTransformContext* transform_ctx;
  function* fn;
  unsigned int accesses_found;
  unsigned int accesses_to_transform;
};

// Check a single tree expression for field access
static bool checkExprForOwnedAccess (
  AD_FUNC_ARGS,
  tree expr,
  TransformWalkData* data,
  bool is_read
) {
  (void) gcc_ctx;
  FieldAccessInfo info;
  if (!extractFieldAccess (expr, &info)) return false;

  data->accesses_found++;

  // Check if this field is owned
  bool is_owned = isFieldOwned (data->transform_ctx, info.type_name, info.field_name);

  if (is_owned) {
    data->accesses_to_transform++;
    AD_DEBUG_PRINT ("  [OWNED ACCESS] %s::%s (%s)",
                    info.type_name, info.field_name,
                    is_read ? "read" : "write");
    return true;
  }

  return false;
}

// Analyze a GIMPLE statement for owned field accesses
static void analyzeStmtForOwnedAccess (AD_FUNC_ARGS, gimple* stmt, TransformWalkData* data) {
  (void) gcc_ctx;
  if (!stmt) return;

  switch (gimple_code (stmt)) {
    case GIMPLE_ASSIGN: {
      // Check LHS (write)
      tree lhs = gimple_assign_lhs (stmt);
      checkExprForOwnedAccess (AD_ARGS, lhs, data, false);

      // Check RHS operands (read)
      for (unsigned i = 1; i < gimple_num_ops (stmt); i++) {
        tree op = gimple_op (stmt, i);
        if (op) checkExprForOwnedAccess (AD_ARGS, op, data, true);
      }
      break;
    }

    case GIMPLE_CALL: {
      // Check arguments (usually reads, but could be writes for out params)
      for (unsigned i = 0; i < gimple_call_num_args (stmt); i++) {
        tree arg = gimple_call_arg (stmt, i);
        checkExprForOwnedAccess (AD_ARGS, arg, data, true);
      }
      // Check LHS if present
      tree lhs = gimple_call_lhs (stmt);
      if (lhs) checkExprForOwnedAccess (AD_ARGS, lhs, data, false);
      break;
    }

    case GIMPLE_RETURN: {
      greturn* ret = as_a<greturn*> (stmt);
      tree val = gimple_return_retval (ret);
      if (val) checkExprForOwnedAccess (AD_ARGS, val, data, true);
      break;
    }

    case GIMPLE_COND: {
      gcond* cond = as_a<gcond*> (stmt);
      checkExprForOwnedAccess (AD_ARGS, gimple_cond_lhs (cond), data, true);
      checkExprForOwnedAccess (AD_ARGS, gimple_cond_rhs (cond), data, true);
      break;
    }

    default:
      break;
  }
}

// ============================================================================
// Transform a function
// ============================================================================

unsigned int transformFunctionForOwnedFields (
  AD_FUNC_ARGS,
  LtoTransformContext* transform_ctx,
  function* fn
) {
  (void) gcc_ctx;
  if (!transform_ctx || !fn) return 0;

  TransformWalkData walk_data;
  walk_data.transform_ctx = transform_ctx;
  walk_data.fn = fn;
  walk_data.accesses_found = 0;
  walk_data.accesses_to_transform = 0;

  // Get function name for debugging (avoid function_name symbol on newer GCC).
  char const* fn_name = nullptr;
  if (fn && fn->decl && DECL_NAME (fn->decl)) {
    fn_name = IDENTIFIER_POINTER (DECL_NAME (fn->decl));
  }

  AD_DEBUG_PRINT ("[transformFunction] Analyzing function: %s",
                  fn_name ? fn_name : "<anonymous>");

  // Traverse all basic blocks
  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
         !gsi_end_p (gsi); gsi_next (&gsi)) {
      gimple* stmt = gsi_stmt (gsi);
      analyzeStmtForOwnedAccess (AD_ARGS, stmt, &walk_data);
    }
  }

  AD_DEBUG_PRINT ("[transformFunction] %s: %u accesses found, %u owned",
                  fn_name ? fn_name : "<anon>",
                  walk_data.accesses_found,
                  walk_data.accesses_to_transform);

  transform_ctx->accesses_found += walk_data.accesses_found;
  transform_ctx->accesses_transformed += walk_data.accesses_to_transform;

  // TODO: Actual code transformation will be added here
  // For now, we just identify the accesses

  return walk_data.accesses_to_transform;
}

// ============================================================================
// Entry point for function_transform callback
// ============================================================================

unsigned int runLtoTransform (AD_FUNC_ARGS, function* fn) {
  // Initialize global context on first call
  if (!g_transform_ctx) {
    g_transform_ctx = new LtoTransformContext ();
    if (!initLtoTransformContext (AD_ARGS, g_transform_ctx)) {
      // No owned fields to transform
      delete g_transform_ctx;
      g_transform_ctx = nullptr;
      return 0;
    }

    // Debug: print owned field table
    AD_DEBUG_PRINT ("[runLtoTransform] LTO Transform initialized");
    printOwnedFieldTable (AD_ARGS, g_transform_ctx);

    if (!g_ltrans_aggregated_written) {
      char const* aggregated_file = getenv ("AD_AGGREGATED_FILE");
      if (aggregated_file) {
        // NOTE: writeLtransAggregatedResults was removed along with result-aggregator module.
        // To re-enable, implement based on new capacity analysis modules.
        (void) aggregated_file;
      }
      g_ltrans_aggregated_written = true;
    }
  }

  if (!g_transform_ctx) return 0;

  return transformFunctionForOwnedFields (AD_ARGS, g_transform_ctx, fn);
}

} // namespace array_detect_ns
