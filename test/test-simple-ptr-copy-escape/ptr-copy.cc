#include "ptr-copy.hh"

void *getAnonymousName ();

int copyName (PtrCopyHuman &lhs, PtrCopyHuman &rhs) {
  lhs.name = rhs.name;
  return 0;
}

int copyAge (PtrCopyHuman &lhs, PtrCopyHuman &rhs) {
  lhs.age = rhs.age;
  return 1;
}

int initHuman (PtrCopyHuman &self) {
  self.name = getAnonymousName ();
  return 2;
}

int escapeSymmetric (PtrCopyHuman &self, PtrCopyHuman &rhs) {
  void *p = getAnonymousName ();
  self.name = p;
  rhs.name = p;
  return 3;
}

int escapeMem (PtrCopyHuman &self, void *&out) {
  void *p = getAnonymousName ();
  self.name = p;
  out = p;
  return 4;
}
