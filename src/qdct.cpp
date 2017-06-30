#include "qdct.h"

/* Core transformation
 *
 * given the residual matrix: R, the core matrix: W, is
 *   W = Cf x R x Cf^T
 */
void forward_dct4x4(const int mat_x[][4], int mat_z[][4]) {
  int mat_temp[4][4];
  int p0, p1, p2, p3, t0, t1, t2, t3;

	// Horizontal
  for (int i = 0; i < 4; i++) {
    p0 = mat_x[i][0];
    p1 = mat_x[i][1];
    p2 = mat_x[i][2];
    p3 = mat_x[i][3];

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    mat_temp[i][0] = t0 + t1;
    mat_temp[i][1] = (t3 << 1) + t2;
    mat_temp[i][2] = t0 - t1;
    mat_temp[i][3] = t3 - (t2 << 1);
  }

  // Vertical
  for (int i = 0; i < 4; i++) {
    p0 = mat_temp[0][i];
    p1 = mat_temp[1][i];
    p2 = mat_temp[2][i];
    p3 = mat_temp[3][i];

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    mat_z[0][i] = t0 + t1;
    mat_z[1][i] = t2 + (t3 << 1);
    mat_z[2][i] = t0 - t1;
    mat_z[3][i] = t3 - (t2 << 1);
  }
}

/* Inversed core transformation
 *
 * given the core matrix: W, the residual matrix: R is
 *   R = Ci x W x Ci^T
 */
void inverse_dct4x4(const int mat_x[][4], int mat_z[][4]) {
  int mat_temp[4][4];
  int p0, p1, p2, p3, t0, t1, t2, t3;

	// Horizontal
  for (int i = 0; i < 4; i++) {
    t0 = mat_x[i][0];
    t1 = mat_x[i][1];
    t2 = mat_x[i][2];
    t3 = mat_x[i][3];

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 = t1 + (t3 >> 1);

    mat_temp[i][0] = p0 + p3;
    mat_temp[i][1] = p1 + p2;
    mat_temp[i][2] = p1 - p2;
    mat_temp[i][3] = p0 - p3;
  }

  // Vertical
  for (int i = 0; i < 4; i++) {
    t0 = mat_temp[0][i];
    t1 = mat_temp[1][i];
    t2 = mat_temp[2][i];
    t3 = mat_temp[3][i];

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 = t1 + (t3 >> 1);

    mat_z[0][i] = p0 + p3;
    mat_z[1][i] = p1 + p2;
    mat_z[2][i] = p1 - p2;
    mat_z[3][i] = p0 - p3;
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++)
      // mat_z[i][j] = int(mat_z[i][j] / 64.0 + 0.5);
      mat_z[i][j] = (mat_z[i][j] + 32) >> 6;
  }
}

/* Quantization
 *
 * By formula:
 *   (0, 0),(2, 0),(0, 2),(2, 2): mf = mat_MF[QP % 6][0]
 *   (1, 1),(3, 1),(1, 3),(3, 3): mf = mat_MF[QP % 6][1]
 *   other positions:             mf = mat_MF[QP % 6][2]
 */
void forward_quantize4x4(const int mat_x[][4], int mat_z[][4], const int QP) {
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

      mat_z[i][j] = (abs(mat_x[i][j]) * mat_MF[QP % 6][k] + f) >> qbits;
      if (mat_x[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

void forward_DC_quantize4x4(const int mat_x[][4], int mat_z[][4], const int QP) {
  int qbits = 15 + floor(QP / 6);
  int f = (int)(pow(2.0, qbits) / 3.0);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mat_z[i][j] = (abs(mat_x[i][j]) * mat_MF[QP % 6][0] + 2 * f) >> (qbits + 1);
      if (mat_x[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

/* Rescaling (inversed quantization)
 *
 * By formula:
 *   (0, 0),(2, 0),(0, 2),(2, 2): mf = mat_V[QP % 6][0]
 *   (1, 1),(3, 1),(1, 3),(3, 3): mf = mat_V[QP % 6][1]
 *   other positions:             mf = mat_V[QP % 6][2]
 */
void inverse_quantize4x4(const int mat_x[][4], int mat_z[][4], const int QP) {
  int t = floor(QP / 6);
  int k;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      mat_z[i][j] = (mat_x[i][j] * mat_V[QP % 6][k]) << t;
    }
  }
}

/* Inverse Quantization for Luma DC
 */
void inverse_DC_quantize4x4(const int mat_x[][4], int mat_z[][4], const int QP) {
  int t = floor(QP / 6);
  int f = (int)pow(2.0, 1 - t);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (QP >= 12)
        mat_z[i][j] = (int)((mat_x[i][j] * mat_V[QP % 6][0]) * pow(2.0, t - 2));
      else
        mat_z[i][j] = (mat_x[i][j] * mat_V[QP % 6][0] + f) >> (2 - t);
    }
  }
}

void forward_quantize2x2(const int mat_x[][2], int mat_z[][2], const int QP) {
  int qbits = 15 + floor(QP / 6);
  int f = (int)(pow(2.0, qbits) / 3.0);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      mat_z[i][j] = (abs(mat_x[i][j]) * mat_MF[QP % 6][0] + 2 * f) >> (qbits + 1);
      if (mat_x[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

void inverse_quantize2x2(const int mat_x[][2], int mat_z[][2], const int QP) {
  int t = floor(QP / 6);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      if (QP >= 6)
        mat_z[i][j] = (int)((mat_x[i][j] * mat_V[QP % 6][0]) * pow(2.0, t - 1));
      else
        mat_z[i][j] = (mat_x[i][j] * mat_V[QP % 6][0]) >> 1;
    }
  }
}

/* Hadamard transformation on 4x4 block
 */
void forward_hadamard4x4(const int mat_x[][4], int mat_z[][4]) {
  int mat_temp[4][4];
  int p0, p1, p2, p3, t0, t1, t2, t3;

  // Horizontal
  for (int i = 0; i < 4; i++) {
    p0 = mat_x[i][0];
    p1 = mat_x[i][1];
    p2 = mat_x[i][2];
    p3 = mat_x[i][3];

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    mat_temp[i][0] = t0 + t1;
    mat_temp[i][1] = t3 + t2;
    mat_temp[i][2] = t0 - t1;
    mat_temp[i][3] = t3 - t2;
  }

  // Vertical
  for (int i = 0; i < 4; i++) {
    p0 = mat_temp[0][i];
    p1 = mat_temp[1][i];
    p2 = mat_temp[2][i];
    p3 = mat_temp[3][i];

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    mat_z[0][i] = (t0 + t1 + 1) >> 1;
    mat_z[1][i] = (t2 + t3 + 1) >> 1;
    mat_z[2][i] = (t0 - t1 + 1) >> 1;
    mat_z[3][i] = (t3 - t2 + 1) >> 1;
  }
}

/* Inversed hadamard transformation on 4x4 block
 */
void inverse_hadamard4x4(const int mat_x[][4], int mat_z[][4]) {
  int mat_temp[4][4];
  int p0, p1, p2, p3, t0, t1, t2, t3;

  // Horizontal
  for (int i = 0; i < 4; i++) {
    t0 = mat_x[i][0];
    t1 = mat_x[i][1];
    t2 = mat_x[i][2];
    t3 = mat_x[i][3];

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = t1 - t3;
    p3 = t1 + t3;

    mat_temp[i][0] = p0 + p3;
    mat_temp[i][1] = p1 + p2;
    mat_temp[i][2] = p1 - p2;
    mat_temp[i][3] = p0 - p3;
  }

  // Vertical
  for (int i = 0; i < 4; i++) {
    t0 = mat_temp[0][i];
    t1 = mat_temp[1][i];
    t2 = mat_temp[2][i];
    t3 = mat_temp[3][i];

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = t1 - t3;
    p3 = t1 + t3;

    mat_z[0][i] = p0 + p3;
    mat_z[1][i] = p1 + p2;
    mat_z[2][i] = p1 - p2;
    mat_z[3][i] = p0 - p3;
  }
}

void forward_hadamard2x2(const int mat_x[][2], int mat_z[][2]) {
  int p0, p1, p2, p3;

  p0 = mat_x[0][0] + mat_x[0][1];
  p1 = mat_x[0][0] - mat_x[0][1];
  p2 = mat_x[1][0] + mat_x[1][1];
  p3 = mat_x[1][0] - mat_x[1][1];

  mat_z[0][0] = p0 + p2;
  mat_z[0][1] = p1 + p3;
  mat_z[1][0] = p0 - p2;
  mat_z[1][1] = p1 - p3;
}

void inverse_hadamard2x2(const int mat_x[][2], int mat_z[][2]) {
  int t0, t1, t2, t3;

  t0 = mat_x[0][0] + mat_x[0][1];
  t1 = mat_x[0][0] - mat_x[0][1];
  t2 = mat_x[1][0] + mat_x[1][1];
  t3 = mat_x[1][0] - mat_x[1][1];

  mat_z[0][0] = t0 + t2;
  mat_z[0][1] = t1 + t3;
  mat_z[1][0] = t0 - t2;
  mat_z[1][1] = t1 - t3;
}

/* Quantized discrete cosine transformation
 *
 * The interface of forward or inverse QDCT, apply on each 4x4 block
 */
template <typename T>
inline void forward_qdct(T& block, const int BLOCK_SIZE, const int QP) {

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
      forward_dct4x4(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*BLOCK_SIZE+x] = mat_z[y][x];
      }
    }
  }

  int mat16[4][4], mat8[2][2];
  if (BLOCK_SIZE == 16) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        mat16[i][j] = block[i*4*BLOCK_SIZE + j*4];
      }
    }
    forward_hadamard4x4(mat16, mat_x);
    forward_DC_quantize4x4(mat_x, mat16, QP);
  }
  else { // BLOCK_SIZE = 8
    int mat_p[2][2];
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        mat8[i][j] = block[i*4*BLOCK_SIZE + j*4];
      }
    }
    forward_hadamard2x2(mat8, mat_p);
    forward_quantize2x2(mat_p, mat8, QP);
  }

  // Apply 4x4 quantization 16 times on 16x16 block
  for (int i = 0; i < BLOCK_SIZE*BLOCK_SIZE; i += BLOCK_SIZE*4) {
    for (int j = 0; j < BLOCK_SIZE; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*BLOCK_SIZE+x];
      }

      // Apply 4x4 core transform
      forward_quantize4x4(mat_x, mat_z, QP);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*BLOCK_SIZE+x] = mat_z[y][x];
      }
    }
  }

  if (BLOCK_SIZE == 16) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++)
        block[i*4*BLOCK_SIZE + j*4] = mat16[i][j];
    }
  }
  else { // BLOCK_SIZE = 8
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++)
        block[i*4*BLOCK_SIZE + j*4] = mat8[i][j];
    }
  }
}

inline void forward_qdct4x4(Block4x4 block, const int QP) {

  // source 4x4 block, target 4x4 block
  int mat_x[4][4], mat_z[4][4];

  // Copy into 4x4 matrix
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++)
      mat_x[y][x] = block[y*4+x];
  }

  // Apply 4x4 core transform
  forward_dct4x4(mat_x, mat_z);
  forward_quantize4x4(mat_z, mat_x, QP);

  // Write back from 4x4 matrix
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++)
      block[y*4+x] = mat_x[y][x];
  }
}

inline void inverse_qdct4x4(Block4x4 block, const int QP) {

  // source 4x4 block, target 4x4 block
  int mat_x[4][4], mat_z[4][4];

  // Copy into 4x4 matrix
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++)
      mat_x[y][x] = block[y*4+x];
  }

  // Apply 4x4 core transform
  inverse_quantize4x4(mat_x, mat_z, QP);
  inverse_dct4x4(mat_z, mat_x);

  // Write back from 4x4 matrix
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++)
      block[y*4+x] = mat_x[y][x];
  }
}

/* Inversed quantized discrete cosine transformation
 *
 * The interface of forward or inverse QDCT, apply on each 4x4 block
 */
template <typename T>
inline void inverse_qdct(T& block, const int BLOCK_SIZE, const int QP) {

  // source 4x4 block, target 4x4 block
  int mat_x[4][4], mat_z[4][4];

  int mat16[4][4], mat8[2][2];
  if (BLOCK_SIZE == 16) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        mat16[i][j] = block[i*4*BLOCK_SIZE + j*4];
      }
    }

    inverse_hadamard4x4(mat16, mat_z);
    inverse_DC_quantize4x4(mat_z, mat16, QP);
  }
  else { // BLOCK_SIZE = 8
    int mat_p[2][2];
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        mat8[i][j] = block[i*4*BLOCK_SIZE + j*4];
      }
    }
    inverse_hadamard2x2(mat8, mat_p);
    inverse_quantize2x2(mat_p, mat8, QP);
  }

  // Apply 4x4 core transform 16 times on 16x16 block
  for (int i = 0; i < BLOCK_SIZE*BLOCK_SIZE; i += BLOCK_SIZE*4) {
    for (int j = 0; j < BLOCK_SIZE; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*BLOCK_SIZE+x];
      }

      // Apply 4x4 core transform
      inverse_quantize4x4(mat_x, mat_z, QP);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*BLOCK_SIZE+x] = mat_z[y][x];
      }
    }
  }

  if (BLOCK_SIZE == 16) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++)
        block[i*4*BLOCK_SIZE + j*4] = mat16[i][j];
    }
  }
  else { // BLOCK_SIZE = 8
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++)
        block[i*4*BLOCK_SIZE + j*4] = mat8[i][j];
    }
  }

  // Apply 4x4 quantization 16 times on 16x16 block
  for (int i = 0; i < BLOCK_SIZE*BLOCK_SIZE; i += BLOCK_SIZE*4) {
    for (int j = 0; j < BLOCK_SIZE; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*BLOCK_SIZE+x];
      }

      // Apply 4x4 core transform
      inverse_dct4x4(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*BLOCK_SIZE+x] = mat_z[y][x];
      }
    }
  }
}

void qdct_luma16x16_intra(Block16x16& block) {
  forward_qdct(block, 16, LUMA_QP);
}
void qdct_chroma8x8_intra(Block8x8& block) {
  forward_qdct(block, 8, CHROMA_QP);
}
void qdct_luma4x4_intra(Block4x4 block) {
  forward_qdct4x4(block, LUMA_QP);
}
void inv_qdct_luma16x16_intra(Block16x16& block) {
  inverse_qdct(block, 16, LUMA_QP);
}
void inv_qdct_chroma8x8_intra(Block8x8& block) {
  inverse_qdct(block, 8, CHROMA_QP);
}
void inv_qdct_luma4x4_intra(Block4x4 block) {
  inverse_qdct4x4(block, LUMA_QP);
}
