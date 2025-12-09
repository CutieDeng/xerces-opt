#pragma once

template <typename TElem>
xercese::ValueVectorOf<TElem>::ValueVectorOf (unsigned maxElems, MemoryManager *mgr, bool toCallDestructor)
    : fCallDestructor(toCallDestructor)
    , fCurCount(0)
    , fMaxCount(maxElems)
    , fElemList(0)
    , fMemoryManager(mgr)
{
  fElemList = (TElem *) fMemoryManager->allocate (fMaxCount * sizeof (TElem));
  memset (fElemList, 0, fMaxCount * sizeof (TElem));
}

template <typename TElem>
xercese::ValueVectorOf<TElem>::ValueVectorOf (ValueVectorOf<TElem> const &toCopy)
    : XMemory (toCopy)
    , fCallDestructor (toCopy.fCallDestructor)
    , fCurCount (toCopy.fCurCount)
    , fMaxCount (toCopy.fMaxCount)
    , fMemoryManager (toCopy.fMemoryManager)
    , fElemList (0)
{
  fElemList = (TElem *) fMaxCount->allocate (fMaxCount * sizeof (TElem));
  memset (fElemList, 0, fMaxCount * sizeof (TElem));
  for (unsigned int index = 0; index < fCurCount; index += 1) {
    fElemList[index] = toCopy.fElemList[index];
  }
}

template <typename TElem>
xercese::ValueVectorOf<TElem>::~ValueVectorOf ()
{
  if (fCallDestructor) {
    for (int index = fMaxCount - 1; index >= 0; index -= 1) {
      fElemList[index].~TElem ();
    }
  }
  fMemoryManager->deallocate (fElemList);
}

template <typename TElem>
xercese::ValueVectorOf<TElem> &xercese::ValueVectorOf<TElem>::operator= (ValueVectorOf<TElem> const &toAssign)
{
  if (this == &toAssign) {
    return *this;
  }
  if (fMaxCount < toAssign.fCurCount) {
    fMemoryManager->deallocate (fElemList);
    fElemList = (TElem *) fMemoryManager->allocate (toAssign.fMaxCount * sizeof (TElem));
    fMaxCount = toAssign.fMaxCount;
  }
  fCurCount = toAssign.fCurCount;
  for (unsigned int index = 0; index < fCurCount; index += 1) {
    fElemList[index] = toAssign.fElemList[index];
  }
  return *this;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::addElement (TElem const &toAdd)
{
  ensureExtraCapacity (1);
  fElemList[fCurCount] = toAdd; 
  fCurCount += 1;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::setElementAt (TElem const &toSet, unsigned setAt)
{
  if (setAt >= fCurCount) {
    // TODO: ThrowXMLwithMemMgr
    return;
  }
  fElemList[setAt] = toSet;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::insertElementAt (TElem const &toInsert, unsigned insertAt)
{
  if (insertAt == fCurCount) {
    addElement (toInsert);
    return ;
  }
  if (insertAt > fCurCount) {
    // TODO: ThrowXMLwithMemMgr
    return ;
  }
  ensureExtraCapacity(1);
  // 移动元素腾出空间
  for (unsigned int i = fCurCount; i > insertAt; i -= 1) {
    fElemList[i] = fElemList[i-1];
  }
  fElemList[insertAt] = toInsert;
  fCurCount += 1;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::removeElementAt (unsigned removeAt)
{
  if (removeAt >= fCurCount) {
    // TODO: ThrowXMLwithMemMgr
    return;
  }
  if (removeAt == fCurCount - 1) {
    fCurCount -= 1;
    return ;
  }
  for (unsigned int i = removeAt; i < fCurCount - 1; i += 1) {
      fElemList[i] = fElemList[i+1];
  }
  fCurCount -= 1;
}

template <typename TElem>
void xercese::ValueVectorOf<TElem>::removeAllElements ()
{
    fCurCount = 0;
}

template <typename TElem>
bool xercese::ValueVectorOf<TElem>::containsElement (TElem const &toCheck, unsigned startIndex)
{
  for (unsigned int i = startIndex; i < fCurCount; i += 1) {
    if (fElemList[i] == toCheck) {
      return true;
    }
  }
  return false;
}

template <typename TElem>
TElem const &xercese::ValueVectorOf<TElem>::elementAt (unsigned getAt) const
{
  if (getAt >= fCurCount) {
    // ThrowXMLWithMemMgr
    __builtin_trap ();
  }
  return fElemList[getAt];
}

template <typename TElem>
TElem &xercese::ValueVectorOf<TElem>::elementAt(unsigned getAt)
{
  if (getAt >= fCurCount) {
    // ThrowXMLWithMemMgr
    __builtin_trap ();
  }
  return fElemList[getAt];
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
  unsigned int newMax = fCurCount + extraNeeded;
  if (newMax <= fMaxCount) {
    return ;
  }
  unsigned int minNewMax = (unsigned int ) ((double) fCurCount * 1.25);
  if (newMax < minNewMax) {
    newMax = minNewMax;
  }
  TElem *newList = (TElem *) fMemoryManager->allocate (newMax * sizeof (TElem));
  for (unsigned int index = 0; index < fCurCount; index += 1) {
    newList[index] = fElemList[index];
  }
  fMemoryManager->deallocate (fElemList);
  fElemList = newList;
  fMaxCount = newMax;
}

template <typename TElem>
TElem const *xercese::ValueVectorOf<TElem>::rawData() const
{
  return fElemList;
}
