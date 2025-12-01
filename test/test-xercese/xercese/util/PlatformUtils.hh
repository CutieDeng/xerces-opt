#pragma once

#include "xercese/util/XMemory.hh"

namespace xercese
{
  struct XMLPlatformUtils {
    static MemoryManager *fgMemoryManager;
    void XMLPlatformUtils::Initialize (MemoryManager *memMgrOrNull);
  };
} // namespace xercese
