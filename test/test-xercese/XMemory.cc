#include "xercese/util/Memory.hh"
#include "xercese/util/PlatformUtils.hh"
#include "xercese/framework/MemoryManager.hh"

namespace xercese {

inline void *XMemory::operator new (size_t size) {
  size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation (sizeof (MemoryManager *));
  void *block = XMLPlatformUtils::fgMemoryManager->allocate (headerSize * size);
  *(MemoryManager**) block = XMLPlatformUtils::fgMemoryManager;
    return (void *)((char *)block + headerSize);
}

inline void *XMemory::operator new(size_t size, char const *file, int line) {
  return ::operator new (size);
}

inline void XMemory::operator delete(void *p, char const *file, int line) {
  operator delete (p);
}

inline void *XMemory::operator new(size_t size, MemoryManager *memMgr) {
  size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation (sizeof (MemoryManager *));
  void *block = XMLPlatformUtils::fgMemoryManager->allocate (headerSize * size);
  *(MemoryManager**) block = memMgr;
    return (void *)((char *)block + headerSize);
}

inline void *XMemory::operator new(size_t _size, void *ptr) {
  return ptr;
}

inline void XMemory::operator delete(void *p) {
  if (p != nullptr) {
    size_t headerSize = XMLPlatformUtils::alignPointerForNewBlcokAllocation(sizeof(MemoryManager*)); 
    void *block = (void *)((char *) p - headerSize);
    MemoryManager *mgr = *(MemoryManager**)block;
    mgr->deallocate(block);
  }
}

inline void XMemory::operator delete(void *p, MemoryManager *memMgr) {
  if (p != nullptr) {
    size_t headerSize = XMLPlatformUtils::alignPointerForNewBlcokAllocation(sizeof(MemoryManager*)); 
    void *block = (void *)((char *) p - headerSize);
    MemoryManager *mgr = *(MemoryManager**)block;
    mgr->deallocate(block);
  }
}

inline void XMemory::operator delete(void *p, void *ptr) {}

} // namespace xercese
