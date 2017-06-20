#include "qdct.h"

/* Matrix multiplication
 *
 * mat_mul(mat_a, mat_b, mat_c): mat_c = mat_a x mat_b
 */
inline void mat_mul(const int a[][4], const int b[][4], int c[][4]) {
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			c[i][j] = 0;
      for(int k = 0; k < 4; k++)
        c[i][j] += a[i][k] * b[k][j];
		}
	}
}

/* Core transformation on 4x4 block
 *
 * core_transform(mat_x, mat_z, QP): mat_x -> mat_z
 */
inline void core_transform(const int mat_x[][4], int mat_z[][4], const int QP) {

  /* Core transformation
   *
   * given the residual matrix: R, the core matrix: W, is
   *   W = Cf x R x Cf^T
   */
  static int mat_temp[4][4], mat_w[4][4];
	mat_mul(mat_Cf, mat_x, mat_temp);
  mat_mul(mat_temp, mat_Cf_T, mat_w);

  /* Quantization
   *
   * By formula:
   *   (0, 0),(2, 0),(0, 2),(2, 2): mf = mat_MF[QP % 6][0]
   *   (1, 1),(3, 1),(1, 3),(3, 3): mf = mat_MF[QP % 6][1]
   *   other positions:             mf = mat_MF[QP % 6][2]
   */
  int qbits = 15 + floor(QP / 6);
  int f = (int)(pow(2.0, qbits) / 3.0);
  int k;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      mat_z[i][j] = (abs(mat_w[i][j]) * mat_MF[QP % 6][k] + f) >> qbits;
      if (mat_w[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

/* Inversed core transformation on 4x4 block
 *
 * inv_core_transform(mat_x, mat_z, QP): mat_x -> mat_z
 */
inline void inv_core_transform(const int mat_x[][4], int mat_z[][4], const int QP) {

  /* Rescaling (inversed quantization)
   *
   * By formula:
   *   (0, 0),(2, 0),(0, 2),(2, 2): mf = mat_V[QP % 6][0]
   *   (1, 1),(3, 1),(1, 3),(3, 3): mf = mat_V[QP % 6][1]
   *   other positions:             mf = mat_V[QP % 6][2]
   */
  int t = floor(QP / 6);
  int f = (int)pow(2.0, t);
  int k;
  static int mat_w[4][4];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      mat_w[i][j] = mat_x[i][j] * mat_V[QP % 6][k] * f;
    }
  }

  /* Inversed core transformation
   *
   * given the core matrix: W, the residual matrix: R is
   *   R = Ci x W x Ci^T
   */
  static int mat_temp[4][4];
	mat_mul(mat_Ci, mat_w, mat_temp);
  mat_mul(mat_temp, mat_Ci_T, mat_z);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++)
      mat_z[i][j] = int(mat_z[i][j] / 256.0 + 0.5);
  }
}

/* Quantized discrete cosine transformation
 *
 * The interface of forward or inverse QDCT, apply on each 4x4 block
 */
template <typename T, typename Func>
inline void qdct(T& block, const int BLOCK_SIZE, Func transform_func, const int QP) {

  // source 4x4 block, target 4x4 block
  int mat_x[4][4], mat_z[4][4];

  // Apply 4x4 core transform 16 times on 16x16 block
  for (int i = 0; i < BLOCK_SIZE*BLOCK_SIZE; i += BLOCK_SIZE*4) {
    for (int j = 0; j < BLOCK_SIZE; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*BLOCK_SIZE+x];
      }

      // Apply 4x4 core transform
      transform_func(mat_x, mat_z, QP);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*BLOCK_SIZE+x] = mat_z[y][x];
      }
    }
  }
}

void qdct_luma16x16_intra(Block16x16& block) {
  qdct(block, 16, core_transform, LUMA_QP);
}
void qdct_chroma8x8_intra(Block8x8& block) {
  qdct(block, 8, core_transform, CHROMA_QP);
}
void inv_qdct_luma16x16_intra(Block16x16& block) {
  qdct(block, 16, inv_core_transform, LUMA_QP);
}
void inv_qdct_chroma8x8_intra(Block8x8& block) {
  qdct(block, 8, inv_core_transform, CHROMA_QP);
}
