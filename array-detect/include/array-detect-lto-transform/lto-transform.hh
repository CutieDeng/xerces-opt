#ifndef ARRAY_DETECT_LTO_TRANSFORM_HH
#define ARRAY_DETECT_LTO_TRANSFORM_HH

#include "gcc-common.hh"
#include "lto-summary.hh"

#include "hash-map.h"
#include "hash-traits.h"

namespace array_detect_ns {

// ============================================================================
// Owned field lookup using GCC hash_map
// ============================================================================

// Key type: (type_name, field_name) pair of strings
// Using pair_hash with nofree_string_hash for both strings
typedef pair_hash<nofree_string_hash, nofree_string_hash> TypeFieldPairHash;

// Value type: pointer to summary (may be nullptr for file-based entries)
typedef LtoUnifiedResultSummary* OwnedFieldValue;

// Hash map traits: key uses pair_hash, value is pointer (unbounded since
// pointer cannot represent empty/deleted - use value for these markers)
typedef simple_hashmap_traits<TypeFieldPairHash, OwnedFieldValue> OwnedFieldMapTraits;

// The hash map type: (type_name, field_name) -> LtoUnifiedResultSummary*
typedef hash_map<
  std::pair<const char*, const char*>,
  OwnedFieldValue,
  OwnedFieldMapTraits
> OwnedFieldMap;

// ============================================================================
// LTO Transform context
// ============================================================================

struct LtoTransformContext {
  // Lookup table: (type_name, field_name) -> owned info
  OwnedFieldMap* owned_fields;

  // Statistics
  unsigned int fields_transformed;
  unsigned int accesses_found;
  unsigned int accesses_transformed;
};

// ============================================================================
// API functions
// ============================================================================

// Initialize transform context from LTRANS summaries
// Returns true if there are owned fields to transform
bool initLtoTransformContext (LtoTransformContext* ctx);

// Clean up transform context
void deinitLtoTransformContext (LtoTransformContext* ctx);

// Check if a (type, field) pair is owned
// Returns the summary if owned, nullptr otherwise
LtoUnifiedResultSummary* lookupOwnedField (
  LtoTransformContext* ctx,
  char const* type_name,
  char const* field_name
);

// Check if field is owned (returns true even if summary is nullptr)
bool isFieldOwned (
  LtoTransformContext* ctx,
  char const* type_name,
  char const* field_name
);

// Transform a single function's GIMPLE to handle owned field accesses
// Returns number of transformations made
unsigned int transformFunctionForOwnedFields (
  LtoTransformContext* ctx,
  function* fn
);

// Entry point: transform all functions in LTRANS
// Called from function_transform callback
unsigned int runLtoTransform (function* fn);

// Debug: print owned field table
void printOwnedFieldTable (LtoTransformContext* ctx, FILE* out);

} // namespace array_detect_ns

#endif // ARRAY_DETECT_LTO_TRANSFORM_HH
