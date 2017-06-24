#ifndef FRAME
#define FRAME

#include <vector>

#include "log.h"
#include "macroblock.h"

enum {
  MB_NEIGHBOR_UL,
  MB_NEIGHBOR_U,
  MB_NEIGHBOR_UR,
  MB_NEIGHBOR_L
};

enum {
  I_PICTURE,
  P_PICTURE
};

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

class Frame {
public:
  int type;
  int width;
  int height;
  int raw_width;
  int raw_height;
  int nb_mb_rows;
  int nb_mb_cols;
  std::vector<MacroBlock> mbs;

  Frame(const PadFrame&);
  int get_neighbor_index(const int, const int);
};

#endif
