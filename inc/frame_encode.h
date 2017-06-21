#ifndef FRAME_ENCODE
#define FRAME_ENCODE

#include <vector>
#include <experimental/optional>
#include <functional>

#include "block.h"
#include "macroblock.h"
#include "frame.h"
#include "intra.h"
#include "qdct.h"

void encode_I_frame(Frame&);
void encode_Y_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);
void encode_Cr_Cb_block(MacroBlock&, std::vector<MacroBlock>&, Frame&);

#endif
