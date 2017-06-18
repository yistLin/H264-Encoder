#include "frame.h"

/* Initialize Frame(I-Picture) with a PadFrame
 *
 * Only I-Picture can be initialized with a padded frame, since there is no dependency
 * between I-Picture and other Pictures.
 */
Frame::Frame(const PadFrame& pf): type(I_PICTURE), width(pf.width), height(pf.height), raw_width(pf.raw_width), raw_height(pf.raw_height) {

  // Basic unit: number of columns, number of rows, number of macroblocks
  int nb_cols = pf.width / 16;
  int nb_rows = pf.height / 16;
  int nb_mbs = nb_cols * nb_rows;
  int cnt_mbs = 0;

  // Reserve the capacity of vector
  this->mbs.reserve(nb_mbs);

  for (int y = 0; y < nb_rows; y++) {
    for (int x = 0; x < nb_cols; x++) {
      // Initialize macroblock with row and column address
      MacroBlock mb(y, x);

      // The cnt_mbs-th macroblocks of this frame
      mb.mb_index = cnt_mbs++;

      // Upper left corner of block Y
      auto ul_itr = pf.Y.begin() + y * 16 * pf.width + x * 16;
      for (int i = 0; i < 256; i += 16) {
        for (int j = 0; j < 16; j++)
          mb.Y[i + j] = *(ul_itr++); // Y is 16x16
        ul_itr = ul_itr - 16 + pf.width;
      }

      // Upper left corner of block Cr
      ul_itr = pf.Cr.begin() + y * 16 * pf.width + x * 16;
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++) {
          mb.Cr[i + j] = *ul_itr; // Cr is 8x8 and down-sampled
          ul_itr += 2;
        }
        ul_itr = ul_itr - 16 + 2 * pf.width;
      }

      // Insert into Cb block
      ul_itr = pf.Cb.begin() + y * 16 * pf.width + x * 16;
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++) {
          mb.Cb[i + j] = *ul_itr; // Cb is 8x8 and down-sampled
          ul_itr += 2;
        }
        ul_itr = ul_itr - 16 + 2 * pf.width;
      }

      // Push into the vector of macroblocks
      this->mbs.push_back(mb);
    }
  }

  // Set arguments of frame
  this->nb_mb_rows = nb_rows;
  this->nb_mb_cols = nb_cols;
}

int Frame::get_neighbor_index(const int curr_index, const int neighbor_type) {
  int neighbor_index = 0;

  switch(neighbor_type) {
    case MB_NEIGHBOR_UL: // Upper left neighbor
      if (curr_index % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - this->nb_mb_cols - 1;
      break;

    case MB_NEIGHBOR_U: // Upper neighbor
      neighbor_index = curr_index - this->nb_mb_cols;
      break;

    case MB_NEIGHBOR_UR: // Upper right neighbor
      if ((curr_index + 1) % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - this->nb_mb_cols + 1;
      break;

    case MB_NEIGHBOR_L: // Left neighbor
      if (curr_index % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - 1;
      break;

    default:
      neighbor_index = -1;
      break;
  }

  // If neighbor doesn't exist, return -1
  if (neighbor_index < 0)
    neighbor_index = -1;
  return neighbor_index;
}
