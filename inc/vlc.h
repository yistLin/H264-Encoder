#ifndef VLC
#define VLC

#include <cmath>
#include <bitset>
#include <string>
#include <cstdint>

#include "block.h"
#include "bitstream.h"

Bitstream exp_golomb(const int);
Bitstream signed_exp_golomb(const int);

Bitstream cavlc_block4x4(Block4x4, const int);

#endif
