#ifndef VLC
#define VLC

#include <cmath>
#include <bitset>
#include <string>
#include <cstdint>

#include "block.h"
#include "bitstream.h"

Bitstream ue(const unsigned int);
Bitstream se(const int);

std::pair<Bitstream, int> cavlc_block2x2(Block2x2, const int);
std::pair<Bitstream, int> cavlc_block4x4(Block4x4, const int);

#endif
