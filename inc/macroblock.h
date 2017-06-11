#ifndef MACROBLOCK
#define MACROBLOCK

#include <array>

#include "block.h"

#define BLOCKS_PER_MB 4+1+1

class MacroBlock {
public:
	std::array<Block, BLOCKS_PER_MB> block;
	
	MacroBlock();
};

#endif