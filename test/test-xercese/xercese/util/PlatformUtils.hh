#pragma once

#include "xercese/util/XMemory.hh"

namespace xercese
{

struct XMLPlatformUtils
{
    static MemoryManager *fgMemoryManager;

    static void Initialize (MemoryManager *memMgrOrNull);

    static void Terminate ();

    inline static size_t alignPointerForNewBlockAllocation (size_t ptrSize);
};

size_t XMLPlatformUtils::alignPointerForNewBlockAllocation (size_t ptrSize) {
  size_t alignment = sizeof (void *) >= sizeof (double) ? sizeof (void *) : sizeof (double);
  size_t current = ptrSize % alignment;
  return current == 0 ? ptrSize : (ptrSize + alignment - current);
}

} // namespace xercese
