#ifndef BLOCK
#define BLOCK

#include <array>

#define PIXELS_PER_BLOCK 8*8

typedef std::array<int, 8*8> Block8x8;
typedef std::array<int, 16*16> Block16x16;

#endif
