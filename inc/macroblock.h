#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>

#include "block.h"

#define BLOCKS_PER_MB 4+1+1

class MacroBlock {
public:
  int mb_row;
  int mb_col;
  std::array<Block, BLOCKS_PER_MB> block;

  MacroBlock(const int r, const int c): mb_row(r), mb_col(c) {}
  Block& operator[] (int x) {
    return block[x];
  }
};

#endif
