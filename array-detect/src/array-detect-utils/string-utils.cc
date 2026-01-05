#include "string-utils.hh"

namespace array_detect_ns {

// ============================================================================
// Safe Name Accessors
// ============================================================================

char const* safeGetTypeName (AD_FUNC_ARGS, tree type) {
  (void)ctx; (void)gcc_ctx;

  if (!type) {
    return "<null-type>";
  }

  tree type_id = TYPE_IDENTIFIER (type);
  if (!type_id) {
    return "<anonymous-type>";
  }

  char const* id_ptr = IDENTIFIER_POINTER (type_id);
  if (!id_ptr) {
    return "<unnamed-type>";
  }

  return identifier_to_locale (id_ptr);
}

char const* safeGetFieldName (AD_FUNC_ARGS, tree field_decl) {
  (void)ctx; (void)gcc_ctx;

  if (!field_decl) {
    return "<null-field>";
  }

  tree decl_name = DECL_NAME (field_decl);
  if (!decl_name) {
    return "<anonymous-field>";
  }

  char const* id_ptr = IDENTIFIER_POINTER (decl_name);
  if (!id_ptr) {
    return "<unnamed-field>";
  }

  return identifier_to_locale (id_ptr);
}

// ============================================================================
// Racket String Utilities
// ============================================================================

void escapeRacketString (
  ArrayDetectContext& ctx,
  char const* input,
  size_t half_offset
) {
  size_t half_size = ctx.escaped_string_buffer_size / 2;
  char* output = ctx.escaped_string_buffer + (half_offset * half_size);
  size_t output_size = half_size;

  size_t j = 0;
  for (size_t i = 0; input[i] != '\0' && j < output_size - 1; i++) {
    char c = input[i];
    if (c == '"' || c == '\\') {
      if (j + 2 >= output_size) break;
      output[j++] = '\\';
      output[j++] = c;
    } else {
      output[j++] = c;
    }
  }
  output[j] = '\0';
}

char* getEscapedString (ArrayDetectContext& ctx, size_t half_offset) {
  size_t half_size = ctx.escaped_string_buffer_size / 2;
  return ctx.escaped_string_buffer + (half_offset * half_size);
}

bool ensureResultBufferCapacity (ArrayDetectContext& ctx, size_t required) {
  if (ctx.result_datum_buffer_capacity >= required) {
    return true;
  }

  size_t new_capacity = ctx.result_datum_buffer_capacity * 2;
  while (new_capacity < required) {
    new_capacity *= 2;
  }

  char* new_buffer = (char*) ggc_realloc (ctx.result_datum_buffer, new_capacity);
  if (!new_buffer) {
    return false;
  }

  ctx.result_datum_buffer = new_buffer;
  ctx.result_datum_buffer_capacity = new_capacity;
  return true;
}

} // namespace array_detect_ns
