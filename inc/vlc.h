#ifndef VLC
#define VLC

#include <cmath>
#include <bitset>
#include <string>
#include <cstdint>

#include "bitstream.h"

Bitstream exp_golomb(const int);
Bitstream signed_exp_golomb(const int);

#endif