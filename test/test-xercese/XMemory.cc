#include "xercese/util/XMemory.hh"
#include "xercese/util/PlatformUtils.hh"
#include "xercese/framework/MemoryManager.hh"

namespace xercese {

void *XMemory::operator new (size_t size) {
  size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation (sizeof (MemoryManager *));
  void *block = XMLPlatformUtils::fgMemoryManager->allocate (headerSize * size);
  *(MemoryManager**) block = XMLPlatformUtils::fgMemoryManager;
    return (void *)((char *)block + headerSize);
}

void *XMemory::operator new(size_t size, char const * /*file*/, int /*line*/) {
  return ::operator new (size);
}

void XMemory::operator delete(void *p, char const * /*file*/, int /*line*/) {
  operator delete (p);
}

void *XMemory::operator new(size_t size, MemoryManager *memMgr) {
  size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation (sizeof (MemoryManager *));
  void *block = XMLPlatformUtils::fgMemoryManager->allocate (headerSize * size);
  *(MemoryManager**) block = memMgr;
    return (void *)((char *)block + headerSize);
}

void *XMemory::operator new(size_t /*size*/, void *ptr) {
  return ptr;
}

void XMemory::operator delete(void *p) {
  if (p != nullptr) {
    size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(sizeof(MemoryManager*)); 
    void *block = (void *)((char *) p - headerSize);
    MemoryManager *mgr = *(MemoryManager**)block;
    mgr->deallocate(block);
  }
}

void XMemory::operator delete(void *p, MemoryManager * /*memMgr*/) {
  if (p != nullptr) {
    size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(sizeof(MemoryManager*)); 
    void *block = (void *)((char *) p - headerSize);
    MemoryManager *mgr = *(MemoryManager**)block;
    mgr->deallocate(block);
  }
}

void XMemory::operator delete(void * /*p*/, void * /*ptr*/) {}

} // namespace xercese
