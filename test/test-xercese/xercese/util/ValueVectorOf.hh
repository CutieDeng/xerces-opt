#pragma once

#include "xercese/util/XMemory.hh"
#include "xercese/framework/MemoryManager.hh"

namespace xercese {

template <typename TElem>
struct ValueVectorOf : XMemory {
  ValueVectorOf (unsigned maxElems, MemoryManager *mgr = XMLPlatformUtils::fgMemoryManager, bool toCallDestructor = false);
  ValueVectorOf (ValueVectorOf<TElem> const &toCopy);
  ~ValueVectorOf ();
  ValueVectorOf<TElem> &operator= (ValueVectorOf<TElem> const &toAssign);
  void addElement (TElem const &toAdd);
  void setElementAt (TElem const &toSet, unsigned setAt);
  void insertElementAt (TElem const &toInsert, unsigned insertAt);
  void removeElementAt (unsigned removeAt);
  void removeAllElements ();
  bool containsElement (TElem const &toCheck, unsigned startIndex = 0);
  TElem const &elementAt (unsigned getAt) const;
  TElem &elementAt (unsigned getAt);
  unsigned curCapacity () const;
  unsigned size () const;
  MemoryManager *getMemoryManager () const;
  void ensureExtraCapacity (unsigned amount);
  TElem const *rawData() const;
  private:
  bool fCallDestructor;
  unsigned fCurCount;
  unsigned fMaxCount;
  TElem *fELemList;
  MemoryManager *fMemoryManager;
};

}

#include "ValueVectorOf.cc"
