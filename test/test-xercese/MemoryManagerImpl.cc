#include "xercese/internal/MemoryManagerImpl.hh"
#include "xercese/util/OutOfMemoryException.hh"

namespace xercese
{
  void *MemoryManagerImpl::allocate (size_t size) {
    void *memptr;
    try {
      memptr = ::operator new (size);
    }
    catch (...) {
      throw OutOfMemoryException();
    }
    if (memptr == NULL) {
      throw OutOfMemoryException();
    }
    return memptr;
  }

  void MemoryManagerImpl::deallocate(void *p) {
    ::operator delete(p);
  }
} // namespace xercese
