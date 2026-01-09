#pragma once

// ============================================================================
// 向后兼容转发头文件
// ============================================================================
// 此头文件已拆分为 3 个独立模块：
// - ad-escaped-use: EscapedUseResult 定义 + extractEscapedUses()
// - ad-source-escape: SourceEscapeConclude 定义 + generateSourceEscapeConclude()
// - ad-field-escape: FieldEscapeConclude 定义 + summarizeFieldEscape()
// ============================================================================

#include "escaped-use.hh"
#include "source-escape.hh"
#include "field-escape.hh"
