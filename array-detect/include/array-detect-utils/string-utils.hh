#pragma once

#include "prelude.hh"
#include "context.hh"
#include "gcc-ext-util.hh"

namespace array_detect_ns {

// ============================================================================
// Safe Name Accessors (consolidated from multiple files)
// ============================================================================

// Get type name safely, returns fallback string if unavailable
char const* safeGetTypeName (AD_FUNC_ARGS, tree type);

// Get field name safely, returns fallback string if unavailable
char const* safeGetFieldName (AD_FUNC_ARGS, tree field_decl);

// ============================================================================
// Racket String Utilities (consolidated from multiple files)
// ============================================================================

// Escape string for Racket datum format (handles quotes and backslashes)
// Uses context's escaped_string_buffer at specified half_offset (0 or 1)
void escapeRacketString (ArrayDetectContext& ctx, char const* input, size_t half_offset);

// Get pointer to escaped string at specified half_offset
char* getEscapedString (ArrayDetectContext& ctx, size_t half_offset);

// Ensure result buffer has required capacity, returns false on allocation failure
bool ensureResultBufferCapacity (ArrayDetectContext& ctx, size_t required);

} // namespace array_detect_ns
