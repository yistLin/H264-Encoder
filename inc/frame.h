#ifndef FRAME
#define FRAME

#include <vector>

#include "io.h"
#include "slice.h"

class Frame {
public:
	int width;
	int height;
	int slices_per_frame;
	std::vector<Slice> slice;

	Frame();
	Frame(const int);
	std::size_t read(RawFrame);
};

#endif