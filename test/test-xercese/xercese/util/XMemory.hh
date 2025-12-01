#pragma once

#include <stddef.h>

namespace xercese
{

struct MemoryManager;

struct XMemory {
  void *operator new (size_t size);  
  void *operator new (size_t size, char const *file, int line); 
  void operator delete (void *p, char const *file, int line);
  void *operator new (size_t size, MemoryManager *memMgr);
  void *operator new (size_t size, void *ptr);
  void operator delete (void *p);
  void operator delete (void *p, MemoryManager *memMgr);
  void operator delete (void *p, void *ptr);
  virtual ~XMemory() {}
};
  
} // namespace xercese
