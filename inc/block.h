#ifndef BLOCK
#define BLOCK

#include <array>
#include <cstdint>

#define PIXELS_PER_BLOCK 8*8

typedef std::array<std::uint8_t, PIXELS_PER_BLOCK> Block;

#endif