#ifndef ARRAY_DETECT_LTO_TRANSFORM_HH
#define ARRAY_DETECT_LTO_TRANSFORM_HH

#include "prelude.hh"
#include "context.hh"
#include "gcc-common.hh"
#include "lto-summary.hh"

#include "hash-map.h"
#include "hash-traits.h"

// ============================================================================
// GCC Version Compatibility Layer
// ============================================================================
// pair_hash, nofree_string_hash, simple_hashmap_traits were introduced in
// GCC 13+. For older versions (GCC 12), we provide a fallback using vec<>.
//
// Detection: Check if pair_hash exists by testing GCC version
// GCC 13+ should have these types in hash-traits.h
// ============================================================================

// GCC version check: __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__
#define AD_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)

#if AD_GCC_VERSION >= 130000
// GCC 13+: Use native pair_hash and hash_map
#define AD_USE_GCC_HASH_MAP 1
#else
// GCC 12 and earlier: Use vec<> fallback
#define AD_USE_GCC_HASH_MAP 0
#endif

namespace array_detect_ns {

// ============================================================================
// Owned field lookup
// ============================================================================

// Value type: pointer to summary (may be nullptr for file-based entries)
typedef LtoUnifiedResultSummary* OwnedFieldValue;

#if AD_USE_GCC_HASH_MAP

// GCC 13+: Use pair_hash with nofree_string_hash
typedef pair_hash<nofree_string_hash, nofree_string_hash> TypeFieldPairHash;
typedef simple_hashmap_traits<TypeFieldPairHash, OwnedFieldValue> OwnedFieldMapTraits;
typedef hash_map<
  std::pair<const char*, const char*>,
  OwnedFieldValue,
  OwnedFieldMapTraits
> OwnedFieldMap;

#else

// GCC 12 fallback: Use vec<> with linear search
// This is less efficient but compatible with older GCC versions
struct OwnedFieldEntry {
  char const* type_name;
  char const* field_name;
  OwnedFieldValue value;
};

// Wrapper class to provide hash_map-like interface over vec<>
class OwnedFieldMap {
  vec<OwnedFieldEntry, va_gc>* entries_;

public:
  OwnedFieldMap () : entries_ (nullptr) {}
  ~OwnedFieldMap () { /* GC-managed, no manual free */ }

  // put: insert or update entry
  void put (std::pair<const char*, const char*> key, OwnedFieldValue val) {
    // Check if exists
    if (entries_) {
      for (unsigned i = 0; i < entries_->length (); i++) {
        OwnedFieldEntry& e = (*entries_)[i];
        if (e.type_name && key.first && strcmp (e.type_name, key.first) == 0 &&
            e.field_name && key.second && strcmp (e.field_name, key.second) == 0) {
          e.value = val;
          return;
        }
      }
    }
    // Insert new
    if (!entries_) vec_alloc (entries_, 16);
    OwnedFieldEntry e = { key.first, key.second, val };
    vec_safe_push (entries_, e);
  }

  // get: lookup by key, returns pointer to value or nullptr
  OwnedFieldValue* get (std::pair<const char*, const char*> key) {
    if (!entries_) return nullptr;
    for (unsigned i = 0; i < entries_->length (); i++) {
      OwnedFieldEntry& e = (*entries_)[i];
      if (e.type_name && key.first && strcmp (e.type_name, key.first) == 0 &&
          e.field_name && key.second && strcmp (e.field_name, key.second) == 0) {
        return &e.value;
      }
    }
    return nullptr;
  }

  // Iterator support for debug printing
  struct iterator {
    vec<OwnedFieldEntry, va_gc>* vec_;
    unsigned idx_;
    iterator (vec<OwnedFieldEntry, va_gc>* v, unsigned i) : vec_ (v), idx_ (i) {}
    bool operator!= (const iterator& o) const { return idx_ != o.idx_; }
    iterator& operator++ () { ++idx_; return *this; }
    std::pair<std::pair<const char*, const char*>, OwnedFieldValue> operator* () const {
      OwnedFieldEntry& e = (*vec_)[idx_];
      return { { e.type_name, e.field_name }, e.value };
    }
  };
  iterator begin () { return iterator (entries_, 0); }
  iterator end () { return iterator (entries_, entries_ ? entries_->length () : 0); }
};

#endif // AD_USE_GCC_HASH_MAP

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
bool initLtoTransformContext (AD_FUNC_ARGS, LtoTransformContext* transform_ctx);

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
  AD_FUNC_ARGS,
  LtoTransformContext* transform_ctx,
  function* fn
);

// Entry point: transform all functions in LTRANS
// Called from function_transform callback
unsigned int runLtoTransform (AD_FUNC_ARGS, function* fn);

// Debug: print owned field table
void printOwnedFieldTable (AD_FUNC_ARGS, LtoTransformContext* transform_ctx);

} // namespace array_detect_ns

#endif // ARRAY_DETECT_LTO_TRANSFORM_HH
