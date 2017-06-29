#include "macroblock.h"

/*
 *  0  1  4  5
 *  2  3  6  7
 *  8  9  12 13
 *  10 11 14 15
 */
const std::array<int, 16> MacroBlock::convert_table = {{0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15}};

Block4x4 MacroBlock::get_Y_4x4_block(int pos) {
  pos = convert_table[pos];
  int origin = (pos / 4) * 64 + (pos % 4) * 4;
  Block4x4 block(Y[origin], Y[origin + 1], Y[origin + 2], Y[origin + 3],
                 Y[origin + 16], Y[origin + 17], Y[origin + 18], Y[origin + 19],
                 Y[origin + 32], Y[origin + 33], Y[origin + 34], Y[origin + 35],
                 Y[origin + 48], Y[origin + 49], Y[origin + 50], Y[origin + 51]);
  return block;
}

Block4x4 MacroBlock::get_Cr_4x4_block(int pos) {
  int origin = (pos / 2) * 32 + (pos % 2) * 4;
  Block4x4 block(Cr[origin], Cr[origin + 1], Cr[origin + 2], Cr[origin + 3],
                 Cr[origin + 8], Cr[origin + 9], Cr[origin + 10], Cr[origin + 11],
                 Cr[origin + 16], Cr[origin + 17], Cr[origin + 18], Cr[origin + 19],
                 Cr[origin + 24], Cr[origin + 25], Cr[origin + 26], Cr[origin + 27]);
  return block;
}

Block4x4 MacroBlock::get_Cb_4x4_block(int pos) {
  int origin = (pos / 2) * 32 + (pos % 2) * 4;
  Block4x4 block(Cb[origin], Cb[origin + 1], Cb[origin + 2], Cb[origin + 3],
                 Cb[origin + 8], Cb[origin + 9], Cb[origin + 10], Cb[origin + 11],
                 Cb[origin + 16], Cb[origin + 17], Cb[origin + 18], Cb[origin + 19],
                 Cb[origin + 24], Cb[origin + 25], Cb[origin + 26], Cb[origin + 27]);
  return block;
}

Block4x4 MacroBlock::get_Y_DC_block() {
  Block4x4 block(Y[0], Y[4], Y[8], Y[12],
                 Y[64], Y[68], Y[72], Y[76],
                 Y[128], Y[132], Y[136], Y[140],
                 Y[192], Y[196], Y[200], Y[204]);
  return block;
}

Block4x4 MacroBlock::get_Y_AC_block(int pos) {
  pos = convert_table[pos];
  int origin = (pos / 4) * 64 + (pos % 4) * 4;
  static int zero = 0;
  Block4x4 block(zero, Y[origin + 1], Y[origin + 2], Y[origin + 3],
                 Y[origin + 16], Y[origin + 17], Y[origin + 18], Y[origin + 19],
                 Y[origin + 32], Y[origin + 33], Y[origin + 34], Y[origin + 35],
                 Y[origin + 48], Y[origin + 49], Y[origin + 50], Y[origin + 51]);
  return block;
}

Block2x2 MacroBlock::get_Cr_DC_block() {
  Block2x2 block(Cr[0], Cr[4], Cr[32], Cr[36]);
  return block;
}

Block4x4 MacroBlock::get_Cr_AC_block(int pos) {
  int origin = (pos / 2) * 32 + (pos % 2) * 4;
  static int zero = 0;
  Block4x4 block(zero, Cr[origin + 1], Cr[origin + 2], Cr[origin + 3],
                 Cr[origin + 8], Cr[origin + 9], Cr[origin + 10], Cr[origin + 11],
                 Cr[origin + 16], Cr[origin + 17], Cr[origin + 18], Cr[origin + 19],
                 Cr[origin + 24], Cr[origin + 25], Cr[origin + 26], Cr[origin + 27]);
  return block;
}

Block2x2 MacroBlock::get_Cb_DC_block() {
  Block2x2 block(Cb[0], Cb[4], Cb[32], Cb[36]);
  return block;
}

Block4x4 MacroBlock::get_Cb_AC_block(int pos) {
  int origin = (pos / 2) * 32 + (pos % 2) * 4;
  static int zero = 0;
  Block4x4 block(zero, Cb[origin + 1], Cb[origin + 2], Cb[origin + 3],
                 Cb[origin + 8], Cb[origin + 9], Cb[origin + 10], Cb[origin + 11],
                 Cb[origin + 16], Cb[origin + 17], Cb[origin + 18], Cb[origin + 19],
                 Cb[origin + 24], Cb[origin + 25], Cb[origin + 26], Cb[origin + 27]);
  return block;
}
