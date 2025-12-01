#include "XMemory.hh"
#include "PlatformUtils.hh"
#include "../framework/MemoryManager.hh"

namespace xercese {

inline void *XMemory::operator new(size_t size) {
    return XMLPlatformUtils::fgMemoryManager->allocate(size);
}

inline void *XMemory::operator new(size_t size, char const *file, int line) {
    return XMLPlatformUtils::fgMemoryManager->allocate(size);
}

inline void XMemory::operator delete(void *p, char const *file, int line) {
    if (p) {
        XMLPlatformUtils::fgMemoryManager->deallocate(p);
    }
}

inline void *XMemory::operator new(size_t size, MemoryManager *memMgr) {
    return memMgr->allocate(size);
}

inline void *XMemory::operator new(size_t size, void *ptr) {
    return ptr;
}

inline void XMemory::operator delete(void *p) {
    if (p) {
        XMLPlatformUtils::fgMemoryManager->deallocate(p);
    }
}

inline void XMemory::operator delete(void *p, MemoryManager *memMgr) {
    if (p) {
        memMgr->deallocate(p);
    }
}

inline void XMemory::operator delete(void *p, void *ptr) {
    // 定位new的delete操作符，通常不需要释放内存
}

} // namespace xercese