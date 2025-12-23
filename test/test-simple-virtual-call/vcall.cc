#include "vcall.hh"

int fuzz (Fuzz &f, Buz &b) {
  b.x = (int *) f.buz ();
  return 1;
}

int quiz (Fuzz &f, Fuzz &f2, Buz &b) {
  f.dumb ();
  b.y = f2.buz (); 
}
