#pragma once

#include "xercese/util/XMemory.hh"

namespace xercese
{

struct XMLPlatformUtils
{
    static MemoryManager *fgMemoryManager; // 全局内存管理器指针

    // 初始化方法，设置全局内存管理器
    static void Initialize (MemoryManager *memMgrOrNull);

    // 终止方法，释放全局内存管理器资源
    static void Terminate();
};

} // namespace xercese
