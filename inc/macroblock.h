#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>

#include "block.h"
#include "intra.h"

#define BLOCKS_PER_MB 4+1+1

class MacroBlock {
public:
  int mb_row;
  int mb_col;
  int mb_index;
  Block16x16 Y;
  Block8x8 Cr;
  Block8x8 Cb;

  bool is_intra16x16;
  Intra16x16Mode intra16x16_Y_mode;


  MacroBlock(const int r, const int c): mb_row(r), mb_col(c) {}
};

#endif
