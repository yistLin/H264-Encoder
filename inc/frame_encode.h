#ifndef FRAME_ENCODE
#define FRAME_ENCODE

#include <vector>
#include <experimental/optional>
#include <functional>
#include <tuple>

#include "block.h"
#include "macroblock.h"
#include "frame.h"
#include "intra.h"
#include "qdct.h"

void encode_I_frame(Frame&);
void encode_Y_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);
int encode_Y_intra16x16_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);
int encode_Y_intra4x4_block(int, MacroBlock&, MacroBlock&, std::vector<MacroBlock>&, Frame&);
void encode_Cr_Cb_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);
int encode_Cr_Cb_intra8x8_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);

#endif
