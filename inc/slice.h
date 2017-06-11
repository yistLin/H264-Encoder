#ifndef SLICE
#define SLICE

#include <vector>

#include "macroblock.h"

class Slice {
public:
	std::vector<MacroBlock> macroblock;

	Slice();
};

#endif