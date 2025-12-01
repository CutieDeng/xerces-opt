#pragma once

#include "xercese/util/XMemory.hh"

namespace xercese
{
  struct OutOfMemoryException : XMemory {
    OutOfMemoryException ();
    ~OutOfMemoryException ();
    OutOfMemoryException (OutOfMemoryException const &toCopy);
    OutOfMemoryException &operator = (OutOfMemoryException const &toAssign);
  };
} // namespace xercese

