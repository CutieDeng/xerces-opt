#pragma once

#include <stdint.h>

namespace xercese
{
  struct MemoryManager {
    virtual ~MemoryManager() {}
    virtual void *allocate(size_t size) = 0;
    virtual void deallocate(void *p) = 0;
    protected:
    MemoryManager() {}
    private:
    MemoryManager(MemoryManager const &);
    MemoryManager &operator=(MemoryManager const &);
  };
} // namespace xercese
