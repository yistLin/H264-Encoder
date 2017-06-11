#include "io.h"
#include <iostream>

RawFrame::RawFrame(const int w, const int h): width(w), height(h) {}

Reader::Reader(const char* filename, const int wid, const int hei) {
  // Initialize logger
  this->logger = Log("Reader");

  this->width = wid;
  this->height = hei;

  // Open the file stream for raw video file
  this->file.open(filename, std::ios::in | std::ios::binary);
  if (!this->file.is_open()) {
    this->logger.log(Level::ERROR, "Cannot open file.");
    exit(1);
  }

  this->file_size = this->get_file_size();

  // Calculate number of frames
  this->pixels_per_unit = wid * hei;
  this->pixels_per_frame = this->pixels_per_unit * 3;
  this->nb_frames = this->file_size / this->pixels_per_frame;
}

std::size_t Reader::get_file_size() {
  // Get file size
  std::size_t begin_pos = this->file.tellg();
  this->file.seekg(0, std::ios::end);
  std::size_t end_pos = this->file.tellg();

  // Go back to the beginning of file
  this->file.clear();
  this->file.seekg(0, std::ios::beg);

  return end_pos - begin_pos;
}

double Reader::clip(const double& val) {
  if (val < 0.0)
    return 0.0;
  else if (val > 255.0)
    return 255.0;
  else
    return val;
}

RawFrame Reader::read_one_frame() {
  unsigned char rgb_pixel[3];
  double b, g, r;
  double y, cb, cr;

  // Reserve frame-sized bytes
  RawFrame rf(this->width, this->height);
  rf.Y.reserve(this->pixels_per_unit);
  rf.Cb.reserve(this->pixels_per_unit);
  rf.Cr.reserve(this->pixels_per_unit);

  for (int i = 0; i < this->pixels_per_unit; i++) {
    // Read 3 bytes as 1 pixel
    this->file.read((char*)rgb_pixel, 3);

    // Convert from char to RGB value
    r = (double)rgb_pixel[0];
    g = (double)rgb_pixel[1];
    b = (double)rgb_pixel[2];

    // Convert from RGB to YCrCb
    y = this->clip(0.299 * r + 0.587 * g + 0.114 * b);
    cb = this->clip(0.564 * (b - y));
    cr = this->clip(0.713 * (r - y));

    // Fill to pixel array
    rf.Y[i] = (std::uint8_t)y;
    rf.Cb[i] = (std::uint8_t)cb;
    rf.Cr[i] = (std::uint8_t)cr;
  }

  return rf;
}
