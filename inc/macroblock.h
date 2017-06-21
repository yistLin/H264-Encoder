#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>
#include <utility>

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
  std::array<Intra4x4Mode, 16> intra4x4_Y_mode;
  IntraChromaMode intra_Cr_Cb_mode;

  static const std::array<int, 16> convert_table;

  MacroBlock(const int r, const int c): mb_row(r), mb_col(c) {}

  /*
   *  0  1  4  5
   *  2  3  6  7
   *  8  9  12 13
   *  10 11 14 15
   */
  Block4x4& get_Y_4x4_block(int pos) {
    pos = convert_table[pos];
    int origin = (pos / 4) * 64 + (pos % 4) * 4;
    static Block4x4 block(Y[origin], Y[origin + 1], Y[origin + 2], Y[origin + 3],
                          Y[origin + 16], Y[origin + 17], Y[origin + 18], Y[origin + 19],
                          Y[origin + 32], Y[origin + 33], Y[origin + 34], Y[origin + 35],
                          Y[origin + 48], Y[origin + 49], Y[origin + 50], Y[origin + 51]);
    return block;
  }
};

#endif
