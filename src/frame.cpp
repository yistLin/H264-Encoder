#include "frame.h"

Frame::Frame(const PadFrame& pf) {
  int nb_cols = pf.width / 16;
  int nb_rows = pf.height / 16;
  int nb_mbs = nb_cols * nb_rows;

  for (int i = 0; i < pf.height; i += 16) {
    for (int j = 0; j < pf.width; j += 16) {

    }
  }
}
