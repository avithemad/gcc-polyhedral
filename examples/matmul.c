#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define M 2048
#define N 2048
#define K 2048
#define alpha 1
#define beta 1

#pragma declarations
double A[M][K];
double B[K][N];
double C[M][N];
#pragma enddeclarations


void matmul() {
    int i, j, k;
#pragma scop
  for (i = 0; i < M; i++)
    for (j = 0; j < N; j++)
      for (k = 0; k < K; k++)
        C[i][j] = beta * C[i][j] + alpha * A[i][k] * B[k][j];
#pragma endscop
}

