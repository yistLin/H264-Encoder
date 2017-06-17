#ifndef FRAME
#define FRAME

#include <vector>

#include "io.h"
#include "macroblock.h"

enum picture_type {
  I_PICTURE,
  P_PICTURE
};

class Frame {
public:
  int type;
  int width;
  int height;
  int raw_width;
  int raw_height;
  std::vector<MacroBlock> mbs;

  Frame(const PadFrame&);
};

#endif
