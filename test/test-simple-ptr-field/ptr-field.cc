#include "ptr-field.hh"

struct Allocator;

void *createPtr();
int *createIntPtr(Allocator &);

extern Allocator *g_allocator;

int fuzz (PtrFieldFuzz &f) {
  f.k = createPtr ();  
  return 0;
}

int fuzz2 (PtrFieldFuzz &f) {
  f.x = createIntPtr (*g_allocator);
  return 0;
}
