#ifndef IO
#define IO

#include <fstream>
#include <vector>
#include <cstdint>

#include "log.h"

class RawFrame {
public:
  int width;
  int height;
  std::vector<std::uint8_t> Y;
  std::vector<std::uint8_t> Cb;
  std::vector<std::uint8_t> Cr;

  RawFrame(const int, const int);
};

class PadFrame {
public:
  int width;
  int height;
  int raw_width;
  int raw_height;
  std::vector<std::uint8_t> Y;
  std::vector<std::uint8_t> Cb;
  std::vector<std::uint8_t> Cr;

  PadFrame(const int, const int);
  PadFrame(const RawFrame&);
};

class Reader {
private:
  Log logger;
  std::fstream file;
  std::size_t get_file_size();
  void convert_rgb_to_ycrcb(unsigned char*, double&, double&, double&);

public:
  std::size_t file_size;
  int width;
  int height;
  int pixels_per_unit;
  int pixels_per_frame;
  int nb_frames;

  Reader(std::string, const int, const int);
  RawFrame read_one_frame();
  PadFrame get_padded_frame();
  double clip(const double&);
};

#endif // IO
