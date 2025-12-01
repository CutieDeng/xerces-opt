#pragma once

#include "xercese/framework/MemoryManager.hh"

namespace xercese
{
  struct MemoryManagerImpl : MemoryManager {
    MemoryManagerImpl() {}
    virtual ~MemoryManagerImpl() {}
    virtual void *allocate(size_t size);
    virtual void deallocate(void *p);
    private:
    MemoryManagerImpl(MemoryManagerImpl const &);
    MemoryManagerImpl &operator=(MemoryManagerImpl const &);
  };
} // namespace xercese
