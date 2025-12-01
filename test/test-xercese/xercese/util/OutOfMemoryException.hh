#pragma once

#include "xercese/util/XMemory.hh"

namespace xercese
{

struct OutOfMemoryException : public XMemory
{
    OutOfMemoryException();
    ~OutOfMemoryException();
    OutOfMemoryException(OutOfMemoryException const&);
    OutOfMemoryException& operator=(OutOfMemoryException const&);
};

// 内联实现所有方法
inline OutOfMemoryException::OutOfMemoryException() : XMemory() {}

inline OutOfMemoryException::~OutOfMemoryException() {}

inline OutOfMemoryException::OutOfMemoryException(OutOfMemoryException const& other) : XMemory(other) {}

inline OutOfMemoryException& OutOfMemoryException::operator=(OutOfMemoryException const& _other)
{
    return *this;
}

} // namespace xercese

