#include "io.h"
#include <iostream>

RawFrame::RawFrame(const int w, const int h): width(w), height(h) {}

PadFrame::PadFrame(const int w, const int h) {
  this->raw_width = w;
  this->raw_height = h;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = w % 16) > 0) ? w + 16 - pd : w;
  this->height = ((pd = h % 16) > 0) ? h + 16 - pd : h;
}

PadFrame::PadFrame(const RawFrame& rf) {
  this->raw_width = rf.width;
  this->raw_height = rf.height;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = rf.width % 16) > 0) ? rf.width + 16 - pd : rf.width;
  this->height = ((pd = rf.height % 16) > 0) ? rf.height + 16 - pd : rf.height;

  // Reserve the pixel vectors
  int pixels_per_unit = this->width * this->height;
  this->Y.reserve(pixels_per_unit);
  this->Cr.reserve(pixels_per_unit);
  this->Cb.reserve(pixels_per_unit);

  // Set up pointers for insertion
  auto y_ptr = rf.Y.begin();
  auto cr_ptr = rf.Cr.begin();
  auto cb_ptr = rf.Cb.begin();
  auto pad_y_ptr = this->Y.begin();
  auto pad_cr_ptr = this->Cr.begin();
  auto pad_cb_ptr = this->Cb.begin();

  // Insert into vectors
  for (int i = 0; i < rf.height; i++) {
    // Insert original pixels
    this->Y.insert(pad_y_ptr, y_ptr, y_ptr + rf.width);
    this->Cr.insert(pad_cr_ptr, cr_ptr, cr_ptr + rf.width);
    this->Cb.insert(pad_cb_ptr, cb_ptr, cb_ptr + rf.width);

    // Insert padding pixels (additional width)
    this->Y.insert(pad_y_ptr + rf.width, this->width - rf.width, (std::uint8_t)0);
    this->Cr.insert(pad_cr_ptr + rf.width, this->width - rf.width, (std::uint8_t)0);
    this->Cb.insert(pad_cb_ptr + rf.width, this->width - rf.width, (std::uint8_t)0);

    // Move pointers
    y_ptr += rf.width;
    cr_ptr += rf.width;
    cb_ptr += rf.width;
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }

  // Insert padding pixels (additional height)
  for (int i = rf.height; i < this->height; i++) {
    this->Y.insert(pad_y_ptr, this->width, (std::uint8_t)0);
    this->Cr.insert(pad_cr_ptr, this->width, (std::uint8_t)0);
    this->Cb.insert(pad_cb_ptr, this->width, (std::uint8_t)0);

    // Move pointers
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }
}

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

void Reader::convert_rgb_to_ycrcb(unsigned char* rgb_pixel, double& y, double& cr, double& cb) {
  double b, g, r;

  // Convert from char to RGB value
  r = (double)rgb_pixel[0];
  g = (double)rgb_pixel[1];
  b = (double)rgb_pixel[2];

  // Convert from RGB to YCrCb
  y = this->clip(0.299 * r + 0.587 * g + 0.114 * b);
  cb = this->clip(0.564 * (b - y));
  cr = this->clip(0.713 * (r - y));
}

RawFrame Reader::read_one_frame() {
  unsigned char rgb_pixel[3];
  double y, cb, cr;

  // Reserve frame-sized bytes
  RawFrame rf(this->width, this->height);
  rf.Y.reserve(this->pixels_per_unit);
  rf.Cb.reserve(this->pixels_per_unit);
  rf.Cr.reserve(this->pixels_per_unit);

  for (int i = 0; i < this->pixels_per_unit; i++) {
    // Read 3 bytes as 1 pixel
    this->file.read((char*)rgb_pixel, 3);

    this->convert_rgb_to_ycrcb(rgb_pixel, y, cr, cb);

    // Fill to pixel array
    rf.Y[i] = (std::uint8_t)y;
    rf.Cb[i] = (std::uint8_t)cb;
    rf.Cr[i] = (std::uint8_t)cr;
  }

  return rf;
}

PadFrame Reader::get_padded_frame() {
  unsigned char rgb_pixel[3];
  double y, cb, cr;
  PadFrame pf(this->width, this->height);

  // Reserve the pixel vectors
  int pixels_per_unit = pf.width * pf.height;
  pf.Y.reserve(pixels_per_unit);
  pf.Cr.reserve(pixels_per_unit);
  pf.Cb.reserve(pixels_per_unit);

  std::fill(pf.Y.begin(), pf.Y.end(), (std::uint8_t)0);
  std::fill(pf.Cr.begin(), pf.Cr.end(), (std::uint8_t)0);
  std::fill(pf.Cb.begin(), pf.Cb.end(), (std::uint8_t)0);

  for (int i = 0; i < pf.raw_height; i++) {
    for (int j = 0; j < pf.raw_width; j++) {
      // Read 3 bytes as 1 pixel
      this->file.read((char*)rgb_pixel, 3);

      this->convert_rgb_to_ycrcb(rgb_pixel, y, cr, cb);

      // Fill to pixel array
      pf.Y[i*pf.width+j] = (std::uint8_t)y;
      pf.Cb[i*pf.width+j] = (std::uint8_t)cb;
      pf.Cr[i*pf.width+j] = (std::uint8_t)cr;
    }
  }

  return pf;
}
