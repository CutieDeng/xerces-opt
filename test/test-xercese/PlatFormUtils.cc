#include <stdlib.h>

#include "xercese/util/PlatformUtils.hh"
#include "xercese/framework/MemoryManager.hh"
#include "xercese/internal/MemoryManagerImpl.hh"

namespace xercese
{
  MemoryManager *XMLPlatformUtils::fgMemoryManager = nullptr;

  void XMLPlatformUtils::Initialize (MemoryManager *memMgrOrNull) {
    if (fgMemoryManager == nullptr) {
      if (memMgrOrNull != nullptr) {
        fgMemoryManager = memMgrOrNull;
      } else {
        fgMemoryManager = new MemoryManagerImpl();
      }
    }
  }

  void XMLPlatformUtils::Terminate() {
    delete fgMemoryManager;
  }

} // namespace xercese
