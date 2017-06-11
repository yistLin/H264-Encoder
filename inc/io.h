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

class Reader {
private:
  std::fstream file;
  Log logger;
  std::size_t get_file_size();

public:
  std::size_t file_size;
  int width;
  int height;
  int pixels_per_unit;
  int pixels_per_frame;
  int nb_frames;
  Reader(const char*, const int, const int);

  RawFrame read_one_frame();
  double clip(const double&);
};

#endif // IO
