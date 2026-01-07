#include "lto-transform.hh"

#include <cstring>

#include "context.hh"

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

// ============================================================================
// Key type alias for cleaner code
// ============================================================================

typedef std::pair<const char*, const char*> TypeFieldKey;

// ============================================================================
// Context initialization
// ============================================================================

bool initLtoTransformContext (LtoTransformContext* ctx) {
  if (!ctx) return false;

  FILE* debug_out = ::array_detect_ns::g_array_detect_ctx.debug_file;
  if (debug_out) fprintf (debug_out, "[initLtoTransformContext] ENTRY\n");

  // Create hash map
  ctx->owned_fields = new OwnedFieldMap ();
  ctx->fields_transformed = 0;
  ctx->accesses_found = 0;
  ctx->accesses_transformed = 0;

  unsigned int owned_count = 0;

  // Get aggregated summaries from LTO section
  // (populated by ipa_read_summary -> readArrayDetectLtoSummarySections)
  vec<LtoUnifiedResultSummary*, va_gc>* summaries = aggregateLtransSummaries ();
  if (summaries && !summaries->is_empty ()) {
    if (debug_out) fprintf (debug_out, "[initLtoTransformContext] Using LTO section summaries, count=%u\n",
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
      if (ctx->owned_fields->get (key) != nullptr) continue;

      // Insert with summary pointer
      ctx->owned_fields->put (key, s);
      owned_count++;

      if (debug_out) {
        fprintf (debug_out, "[initLtoTransformContext] Added owned field from LTO: %s::%s\n", tname, fname);
      }
    }
  }

  ctx->fields_transformed = owned_count;

  if (debug_out) {
    fprintf (debug_out, "[initLtoTransformContext] EXIT, owned_count=%u\n", owned_count);
  }

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

void printOwnedFieldTable (LtoTransformContext* ctx, FILE* out) {
  if (!ctx || !ctx->owned_fields || !out) return;

  fprintf (out, "=== Owned Field Table ===\n");
  fprintf (out, "Total owned fields: %u\n", ctx->fields_transformed);

  // Iterate hash_map using iterator
  for (auto iter = ctx->owned_fields->begin ();
       iter != ctx->owned_fields->end (); ++iter) {
    auto entry = *iter;
    fprintf (out, "  %s::%s\n",
             entry.first.first ? entry.first.first : "<null>",
             entry.first.second ? entry.first.second : "<null>");
  }

  fprintf (out, "=========================\n");
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
  LtoTransformContext* ctx;
  function* fn;
  unsigned int accesses_found;
  unsigned int accesses_to_transform;
  FILE* debug_out;
};

// Check a single tree expression for field access
static bool checkExprForOwnedAccess (
  tree expr,
  TransformWalkData* data,
  bool is_read
) {
  FieldAccessInfo info;
  if (!extractFieldAccess (expr, &info)) return false;

  data->accesses_found++;

  // Check if this field is owned
  bool is_owned = isFieldOwned (data->ctx, info.type_name, info.field_name);

  if (is_owned) {
    data->accesses_to_transform++;
    if (data->debug_out) {
      fprintf (data->debug_out, "  [OWNED ACCESS] %s::%s (%s)\n",
               info.type_name, info.field_name,
               is_read ? "read" : "write");
    }
    return true;
  }

  return false;
}

// Analyze a GIMPLE statement for owned field accesses
static void analyzeStmtForOwnedAccess (gimple* stmt, TransformWalkData* data) {
  if (!stmt) return;

  switch (gimple_code (stmt)) {
    case GIMPLE_ASSIGN: {
      // Check LHS (write)
      tree lhs = gimple_assign_lhs (stmt);
      checkExprForOwnedAccess (lhs, data, false);

      // Check RHS operands (read)
      for (unsigned i = 1; i < gimple_num_ops (stmt); i++) {
        tree op = gimple_op (stmt, i);
        if (op) checkExprForOwnedAccess (op, data, true);
      }
      break;
    }

    case GIMPLE_CALL: {
      // Check arguments (usually reads, but could be writes for out params)
      for (unsigned i = 0; i < gimple_call_num_args (stmt); i++) {
        tree arg = gimple_call_arg (stmt, i);
        checkExprForOwnedAccess (arg, data, true);
      }
      // Check LHS if present
      tree lhs = gimple_call_lhs (stmt);
      if (lhs) checkExprForOwnedAccess (lhs, data, false);
      break;
    }

    case GIMPLE_RETURN: {
      greturn* ret = as_a<greturn*> (stmt);
      tree val = gimple_return_retval (ret);
      if (val) checkExprForOwnedAccess (val, data, true);
      break;
    }

    case GIMPLE_COND: {
      gcond* cond = as_a<gcond*> (stmt);
      checkExprForOwnedAccess (gimple_cond_lhs (cond), data, true);
      checkExprForOwnedAccess (gimple_cond_rhs (cond), data, true);
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
  LtoTransformContext* ctx,
  function* fn
) {
  if (!ctx || !fn) return 0;

  FILE* debug_out = ::array_detect_ns::g_array_detect_ctx.debug_file;

  TransformWalkData walk_data;
  walk_data.ctx = ctx;
  walk_data.fn = fn;
  walk_data.accesses_found = 0;
  walk_data.accesses_to_transform = 0;
  walk_data.debug_out = debug_out;

  // Get function name for debugging
  char const* fn_name = function_name (fn);

  if (debug_out) {
    fprintf (debug_out, "[transformFunction] Analyzing function: %s\n",
             fn_name ? fn_name : "<anonymous>");
  }

  // Traverse all basic blocks
  basic_block bb;
  FOR_EACH_BB_FN (bb, fn) {
    for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
         !gsi_end_p (gsi); gsi_next (&gsi)) {
      gimple* stmt = gsi_stmt (gsi);
      analyzeStmtForOwnedAccess (stmt, &walk_data);
    }
  }

  if (debug_out) {
    fprintf (debug_out, "[transformFunction] %s: %u accesses found, %u owned\n",
             fn_name ? fn_name : "<anon>",
             walk_data.accesses_found,
             walk_data.accesses_to_transform);
  }

  ctx->accesses_found += walk_data.accesses_found;
  ctx->accesses_transformed += walk_data.accesses_to_transform;

  // TODO: Actual code transformation will be added here
  // For now, we just identify the accesses

  return walk_data.accesses_to_transform;
}

// ============================================================================
// Entry point for function_transform callback
// ============================================================================

unsigned int runLtoTransform (function* fn) {
  // Initialize global context on first call
  if (!g_transform_ctx) {
    g_transform_ctx = new LtoTransformContext ();
    if (!initLtoTransformContext (g_transform_ctx)) {
      // No owned fields to transform
      delete g_transform_ctx;
      g_transform_ctx = nullptr;
      return 0;
    }

    // Debug: print owned field table
    FILE* df = ::array_detect_ns::g_array_detect_ctx.debug_file;
    if (df) {
      fprintf (df, "\n[runLtoTransform] LTO Transform initialized\n");
      printOwnedFieldTable (g_transform_ctx, df);
    }
  }

  if (!g_transform_ctx) return 0;

  return transformFunctionForOwnedFields (g_transform_ctx, fn);
}

} // namespace array_detect_ns
