#ifndef QDCT
#define QDCT

#include <cmath>
#include "block.h"

// Important definition

// MAX QP for luma is 51, MAX QP for chroma is 39
const int LUMA_QP = 30;
const int CHROMA_QP = 10;
const int mat_Cf[4][4] = {
  {1,  1,  1,  1},
  {2,  1, -1, -2},
  {1, -1, -1,  1},
  {1, -2,  2, -1}
};
const int mat_Cf_T[4][4] = {
  {1,  2,  1,  1},
  {1,  1, -1, -2},
  {1, -1, -1,  2},
  {1, -2,  1, -1}
};
const int mat_MF[6][3] = {
  {13107, 5243, 8066},
  {11916, 4660, 7490},
  {10082, 4194, 6554},
  {9362,  3647, 5825},
  {8192,  3355, 5243},
  {7282,  2893, 4559}
};
const int mat_Ci[4][4] = {
  {2,  2,  2,  1},
  {2,  1, -2, -2},
  {2, -1, -2,  2},
  {2, -2,  2, -1}
};
const int mat_Ci_T[4][4] = {
  {2,  2,  2,  2},
  {2,  1, -1, -2},
  {2, -2, -2,  2},
  {1, -2,  2, -1}
};
const int mat_V[6][3] = {
  {10, 16, 13},
  {11, 18, 14},
  {13, 20, 16},
  {14, 23, 18},
  {16, 25, 20},
  {18, 29, 23}
};

// Private part
inline void mat_mul(const int[][4], const int[][4], int[][4]);
inline void core_transform(const int[][4], int[][4], const int);
inline void inv_core_transform(const int[][4], int[][4], const int);

// Main QDCT function used as an expandable funciton
template <typename T, typename Func>
inline void qdct(T&, const int, Func, const int);

// Public interface
void qdct_luma16x16_intra(Block16x16&);
void qdct_chroma8x8_intra(Block8x8&);
void inv_qdct_luma16x16_intra(Block16x16&);
void inv_qdct_chroma8x8_intra(Block8x8&);

#endif // QDCT
