#include <stdio.h>

namespace bm {

struct BMPattern {
  int fShiftTableLen;
  char *fShiftTable;
  char *fPattern;
};

void init (BMPattern &bm, int patternLen, void (handle)(char )) {
  for (int i = 0; i < bm.fShiftTableLen; i += 1) {
    bm.fShiftTable[i] = patternLen;
  }
  for (int k = 0; k < patternLen; k += 1) {
    char ch = bm.fPattern[k];
    handle (ch); 
  }
}

} // namespace bm
