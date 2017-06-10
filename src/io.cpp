#include "io.h"

RawFrame::RawFrame(const int w, const int h): width(w), height(h) {}

Reader::Reader(const char* filename, const int wid, const int hei) {
  this->width = wid;
  this->height = hei;

  // Open the file stream for raw video file
  this->file.open(filename, std::ios::in | std::ios::binary);
  if (!this->file.is_open()) {
    Log::log(Level::ERROR, "Cannot open file.");
    exit(1);
  }

  // Get file size
  std::size_t begin_pos = this->file.tellg();
  this->file.seekg(0, std::ios::end);
  std::size_t end_pos = this->file.tellg();
  this->fsize = end_pos - begin_pos;

  // Go back to the beginning of file
  this->file.clear();
  this->file.seekg(0, std::ios::beg);

  // Calculate number of frames
  // yuv420p contains 4 Y + 1 U + 1 V
  this->pixels_per_unit = (wid / 2) * (hei / 2);
  this->pixels_per_frame = this->pixels_per_unit * 6;
  this->nb_frames = this->fsize / this->pixels_per_frame;
}

RawFrame Reader::read_one_frame() {

}