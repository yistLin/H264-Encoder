#ifndef FRAME_ENCODE
#define FRAME_ENCODE

#include <vector>
#include <experimental/optional>
#include <functional>

#include "block.h"
#include "macroblock.h"
#include "frame.h"
#include "intra.h"

void encode_I_frame(Frame&);
void encode_Y_block(MacroBlock&, std::vector<Block16x16>&, Frame&);

#endif
