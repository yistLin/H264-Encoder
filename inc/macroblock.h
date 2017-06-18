#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>

#include "block.h"

#define BLOCKS_PER_MB 4+1+1

class MacroBlock {
public:
  int mb_row;
  int mb_col;
  int mb_index;
  Block16x16 Y;
  Block8x8 Cr;
  Block8x8 Cb;

  MacroBlock(const int r, const int c): mb_row(r), mb_col(c) {}
};

#endif

