#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>
#include <utility>

#include "block.h"
#include "intra.h"
#include "bitstream.h"

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

  bool is_I_PCM = false;

  bool coded_block_pattern_luma = false;
  std::array<bool, 4> coded_block_pattern_luma_4x4{{false, false, false, false}};
  bool coded_block_pattern_chroma_DC = false;
  bool coded_block_pattern_chroma_AC = false;

  Bitstream bitstream;

  static const std::array<int, 16> convert_table;

  MacroBlock(const int r, const int c): mb_row(r), mb_col(c) {}

  Block4x4 get_Y_4x4_block(int pos);
  Block4x4 get_Cr_4x4_block(int pos);
  Block4x4 get_Cb_4x4_block(int pos);

  Block4x4 get_Y_DC_block();
  Block4x4 get_Y_AC_block(int pos);

  Block2x2 get_Cr_DC_block();
  Block4x4 get_Cr_AC_block(int pos);

  Block2x2 get_Cb_DC_block();
  Block4x4 get_Cb_AC_block(int pos);
};

#endif
