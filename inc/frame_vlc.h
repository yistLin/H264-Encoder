#ifndef FRAME_VLC
#define FRAME_VLC

#include <array>
#include <vector>
#include <tuple>

#include "frame.h"
#include "io.h"
#include "macroblock.h"
#include "vlc.h"

void vlc_frame(Frame&, Writer&);
void vlc_Y_DC(MacroBlock&, std::vector<std::array<int, 17>>&, Frame&);
void vlc_Y(int, MacroBlock&, std::vector<std::array<int, 17>>&, Frame&);
void vlc_Cb_DC(MacroBlock&);
void vlc_Cr_DC(MacroBlock&);
void vlc_Cb_AC(int, MacroBlock&, std::vector<std::array<int, 4>>&, Frame&);
void vlc_Cr_AC(int, MacroBlock&, std::vector<std::array<int, 4>>&, Frame&);

#endif
