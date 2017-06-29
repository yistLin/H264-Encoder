#ifndef IO
#define IO

#include <fstream>
#include <cstdint>
#include <vector>
#include <cmath>

#include "log.h"
#include "vlc.h"
#include "nal.h"
#include "qdct.h"
#include "frame.h"
#include "bitstream.h"

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
  void write_slice(const int, Frame&);

private:
  Log logger;
  std::fstream file;
  static std::uint8_t stopcode[4];
  unsigned int log2_max_frame_num;
  unsigned int log2_max_pic_order_cnt_lsb;

  Bitstream seq_parameter_set_rbsp(const int, const int, const int);
  Bitstream pic_parameter_set_rbsp();
  Bitstream write_slice_data(Frame&, Bitstream&);
  Bitstream mb_pred(MacroBlock&, Frame&);
  Bitstream slice_layer_without_partitioning_rbsp(const int, Frame&);
  Bitstream slice_header(const int);
};

#endif // IO
