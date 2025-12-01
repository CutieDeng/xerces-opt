#pragma once

template <typename TElem>
xercese::ValueVectorOf<TElem>::ValueVectorOf(unsigned maxElems, MemoryManager *mgr, bool toCallDestructor)
    : fCallDestructor(toCallDestructor)
    , fCurCount(0)
    , fMaxCount(maxElems)
    , fMemoryManager(mgr)
{
    if (fMemoryManager == nullptr) {
        fMemoryManager = XMLPlatformUtils::fgMemoryManager;
    }

    if (maxElems > 0) {
        fELemList = (TElem*)fMemoryManager->allocate(maxElems * sizeof(TElem));
    } else {
        fELemList = nullptr;
    }
}

template <typename TElem>
xercese::ValueVectorOf<TElem>::ValueVectorOf(ValueVectorOf<TElem> const &toCopy)
    : fCallDestructor(toCopy.fCallDestructor)
    , fCurCount(toCopy.fCurCount)
    , fMaxCount(toCopy.fMaxCount)
    , fMemoryManager(toCopy.fMemoryManager)
{
    if (fMaxCount > 0) {
        fELemList = (TElem*)fMemoryManager->allocate(fMaxCount * sizeof(TElem));
        for (unsigned i = 0; i < fCurCount; ++i) {
            new (&fELemList[i]) TElem(toCopy.fELemList[i]);
        }
    } else {
        fELemList = nullptr;
    }
}

template <typename TElem>
xercese::ValueVectorOf<TElem>::~ValueVectorOf()
{
    removeAllElements();
    if (fELemList != nullptr) {
        fMemoryManager->deallocate(fELemList);
        fELemList = nullptr;
    }
}

template <typename TElem>
xercese::ValueVectorOf<TElem> &xercese::ValueVectorOf<TElem>::operator=(ValueVectorOf<TElem> const &toAssign)
{
    if (this != &toAssign) {
        // 创建临时副本然后交换
        ValueVectorOf<TElem> temp(toAssign);

        // 交换成员变量
        std::swap(fCallDestructor, temp.fCallDestructor);
        std::swap(fCurCount, temp.fCurCount);
        std::swap(fMaxCount, temp.fMaxCount);
        std::swap(fELemList, temp.fELemList);
        std::swap(fMemoryManager, temp.fMemoryManager);
    }
    return *this;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::addElement(TElem const &toAdd)
{
    ensureExtraCapacity(1);
    new (&fELemList[fCurCount]) TElem(toAdd);
    fCurCount++;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::setElementAt(TElem const &toSet, unsigned setAt)
{
    if (setAt >= fCurCount) {
        // 索引越界
        return;
    }

    // 如果需要调用析构函数，先调用原对象的析构函数
    if (fCallDestructor) {
        fELemList[setAt].~TElem();
    }

    // 放置新对象
    new (&fELemList[setAt]) TElem(toSet);
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::insertElementAt(TElem const &toInsert, unsigned insertAt)
{
    if (insertAt > fCurCount) {
        insertAt = fCurCount;
    }

    ensureExtraCapacity(1);

    // 移动元素腾出空间
    for (unsigned i = fCurCount; i > insertAt; --i) {
        new (&fELemList[i]) TElem(fELemList[i-1]);
        if (fCallDestructor) {
            fELemList[i-1].~TElem();
        }
    }

    // 插入新元素
    new (&fELemList[insertAt]) TElem(toInsert);
    fCurCount++;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::removeElementAt(unsigned removeAt)
{
    if (removeAt >= fCurCount) {
        return;
    }

    // 调用析构函数
    if (fCallDestructor) {
        fELemList[removeAt].~TElem();
    }

    // 移动后续元素
    for (unsigned i = removeAt; i < fCurCount - 1; ++i) {
        if (fCallDestructor) {
            fELemList[i].~TElem();
        }
        new (&fELemList[i]) TElem(fELemList[i+1]);
    }

    fCurCount--;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::removeAllElements()
{
    if (fCallDestructor) {
        for (unsigned i = 0; i < fCurCount; ++i) {
            fELemList[i].~TElem();
        }
    }
    fCurCount = 0;
}

template <typename TElem>
bool xercese::ValueVectorOf<TElem>::containsElement(TElem const &toCheck, unsigned startIndex)
{
    for (unsigned i = startIndex; i < fCurCount; ++i) {
        if (fELemList[i] == toCheck) {
            return true;
        }
    }
    return false;
}

template <typename TElem>
TElem const &xercese::ValueVectorOf<TElem>::elementAt(unsigned getAt) const
{
    // 在实际应用中应该添加越界检查
    return fELemList[getAt];
}

template <typename TElem>
TElem &xercese::ValueVectorOf<TElem>::elementAt(unsigned getAt)
{
    // 在实际应用中应该添加越界检查
    return fELemList[getAt];
}

template <typename TElem>
unsigned xercese::ValueVectorOf<TElem>::curCapacity() const
{
    return fMaxCount;
}

template <typename TElem>
unsigned xercese::ValueVectorOf<TElem>::size() const
{
    return fCurCount;
}

template <typename TElem>
xercese::MemoryManager *xercese::ValueVectorOf<TElem>::getMemoryManager() const
{
    return fMemoryManager;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::ensureExtraCapacity(unsigned extraNeeded)
{
    if (fCurCount + extraNeeded > fMaxCount) {
        // 计算新容量，这里简单地翻倍，或者至少加上所需的额外空间
        unsigned newCapacity = fMaxCount * 2;
        if (newCapacity < fCurCount + extraNeeded) {
            newCapacity = fCurCount + extraNeeded;
        }

        // 分配新内存
        TElem *newList = (TElem*)fMemoryManager->allocate(newCapacity * sizeof(TElem));

        // 拷贝现有元素
        for (unsigned i = 0; i < fCurCount; ++i) {
            new (&newList[i]) TElem(fELemList[i]);
            if (fCallDestructor) {
                fELemList[i].~TElem();
            }
        }

        // 释放旧内存
        if (fELemList != nullptr) {
            fMemoryManager->deallocate(fELemList);
        }

        // 更新成员变量
        fELemList = newList;
        fMaxCount = newCapacity;
    }
}

template <typename TElem>
TElem const *xercese::ValueVectorOf<TElem>::rawData() const
{
    return fELemList;
}
