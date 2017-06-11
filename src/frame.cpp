#include "frame.h"

Frame::Frame() : Frame::Frame(1) {}

Frame::Frame(const int _slices_per_frame) {
	// Set number of slices in a frame
	this->slices_per_frame = _slices_per_frame;
	this->slice.reserve(this->slices_per_frame);
}

std::size_t Frame::read(RawFrame rf) {
	this->width = rf.width;
	this->height = rf.height;

	// Each frame is divided into one or several slices
	for (int i = 0; i < this->slices_per_frame; i++) {

	}

	// Return number of blocks
	return 0;
}