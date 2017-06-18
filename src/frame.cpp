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

  // Reserve the capacity of vector
  this->mbs.reserve(nb_mbs);

  for (int y = 0; y < nb_rows; y++) {
    for (int x = 0; x < nb_cols; x++) {
      // Initialize macroblock with row and column address
      MacroBlock mb(y, x);

      // The iterator for upper left corner of a macroblock
      auto ul_itr = pf.Y.begin() + y * 16 * pf.width + x * 16;

      /* Shift the position for Y0, Y1, Y2, Y3
       * -----------
       * | Y0 | Y1 |
       * |---------|
       * | Y2 | Y3 |
       * -----------
       */
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++)
          mb[0][i + j] = *(ul_itr++); // Y0
        for (int j = 0; j < 8; j++)
          mb[1][i + j] = *(ul_itr++); // Y1
        ul_itr = ul_itr - 16 + pf.width;
      }
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++)
          mb[2][i + j] = *(ul_itr++); // Y2
        for (int j = 0; j < 8; j++)
          mb[3][i + j] = *(ul_itr++); // Y3
        ul_itr = ul_itr - 16 + pf.width;
      }

      // Insert into Cr block
      ul_itr = pf.Cr.begin() + y * 16 * pf.width + x * 16;
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++) {
          mb[4][i + j] = *ul_itr; // Cr
          ul_itr += 2;
        }
        ul_itr = ul_itr - 16 + 2 * pf.width;
      }

      // Insert into Cb block
      ul_itr = pf.Cb.begin() + y * 16 * pf.width + x * 16;
      for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; j++) {
          mb[5][i + j] = *ul_itr; // Cb
          ul_itr += 2;
        }
        ul_itr = ul_itr - 16 + 2 * pf.width;
      }

      this->mbs.push_back(mb);
    }
  }
}
