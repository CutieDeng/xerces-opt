#ifndef ARRAY_DETECT_LTO_TRANSFORM_HH
#define ARRAY_DETECT_LTO_TRANSFORM_HH

#include "gcc-common.hh"
#include "lto-summary.hh"

#include <unordered_map>
#include <string>

namespace array_detect_ns {

// ============================================================================
// Owned field lookup table
// ============================================================================

// Key for (type_name, field_name) pair - using std::string for safe storage
struct TypeFieldKey {
  std::string type_name;
  std::string field_name;

  bool operator== (TypeFieldKey const& other) const {
    return type_name == other.type_name && field_name == other.field_name;
  }
};

struct TypeFieldKeyHasher {
  std::size_t operator() (TypeFieldKey const& key) const {
    std::size_t h1 = std::hash<std::string>{}(key.type_name);
    std::size_t h2 = std::hash<std::string>{}(key.field_name);
    return h1 ^ (h2 << 1);
  }
};

// ============================================================================
// LTO Transform context
// ============================================================================

struct LtoTransformContext {
  // Lookup table: (type_name, field_name) -> owned info
  std::unordered_map<TypeFieldKey, LtoUnifiedResultSummary*, TypeFieldKeyHasher>* owned_fields;

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
