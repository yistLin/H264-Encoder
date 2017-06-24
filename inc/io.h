#ifndef IO
#define IO

#include <fstream>
#include <cstdint>
#include <vector>
#include <cmath>

#include "log.h"
#include "vlc.h"
#include "nal.h"
#include "bitstream.h"

class RawFrame {
public:
  int width;
  int height;
  std::vector<int> Y;
  std::vector<int> Cb;
  std::vector<int> Cr;

  RawFrame(const int, const int);
};

class PadFrame {
public:
  int width;
  int height;
  int raw_width;
  int raw_height;
  std::vector<int> Y;
  std::vector<int> Cb;
  std::vector<int> Cr;

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
};

class Writer {
public:
  Writer(std::string);

  void write_sps(const int, const int, const int);
  void write_pps();
  void write_slice(const Bitstream&);

private:
  Log logger;
  std::fstream file;
  static std::uint8_t stopcode[4];

  Bitstream seq_parameter_set_rbsp(const int, const int, const int);
  Bitstream pic_parameter_set_rbsp();
  Bitstream slice_layer_without_partitioning_rbsp(const Bitstream&);
};

#endif // IO
