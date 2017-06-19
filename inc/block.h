#ifndef BLOCK
#define BLOCK

#include <array>

#define PIXELS_PER_BLOCK 8*8

using Block8x8 = std::array<int, 8*8>;
using Block16x16 = std::array<int, 16*16>;

#endif
