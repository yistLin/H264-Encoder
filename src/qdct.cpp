#include "qdct.h"

void mat_mul(const int a[4][4], const int b[4][4], int c[4][4]) {
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			c[i][j] = 0;
      for(int k = 0; k < 4; k++)
        c[i][j] += a[i][k] * b[k][j];
		}
	}
}

void compute_core(int mat_x[4][4]) {
	int mat_temp[4][4];
	mat_mul(mat_Cf, mat_x, mat_temp);
  mat_mul(mat_temp, mat_Cf_T, mat_x);
}

void inv_compute_core(int mat_x[4][4]) {
	int mat_temp[4][4];
	mat_mul(mat_Ci, mat_x, mat_temp);
  mat_mul(mat_temp, mat_Ci_T, mat_x);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++)
      mat_x[i][j] = int(mat_x[i][j] / 256.0 + 0.5);
  }
}

void core_transform(int mat_x[4][4], int mat_z[4][4]) {
  int qbits = 15 + floor(QP / 6);
  int f = (int)(pow(2.0, qbits) / 3.0);
  int mf;
  int k;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      /* By formula
       *
       * (0, 0), (2, 0), (0, 2), (2, 2) mf = mat_MF[QP % 6][0]
       * (1, 1), (3, 1), (1, 3), (3, 3) mf = mat_MF[QP % 6][1]
       * other positions                mf = mat_MF[QP % 6][2]
       */
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      mf = mat_MF[QP % 6][k];
      mat_z[i][j] = (abs(mat_x[i][j]) * mf + f) >> qbits;

      if (mat_x[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

void inv_core_transform(int mat_x[4][4], int mat_z[4][4]) {
  int t = floor(QP / 6);
  int f = (int)pow(2.0, t);
  int k;
  int v;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      v = mat_V[QP % 6][k];
      mat_z[i][j] = mat_x[i][j] * v * f;
    }
  }
}

/* Quantized Discrete Cosine Transform
 *
 * The interface of QDCT, to split a whole macroblock into
 * several 4x4 block and transform it
 */
void QDCT(MacroBlock& mb) {
}

/* Inverse Quantized Discrete Cosine Transform
 *
 * The interface for doing inversed way of QDCT on
 * whole macroblock
 */
void IQDCT(MacroBlock& mb) {

}

