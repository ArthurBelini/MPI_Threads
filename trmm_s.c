/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* trmm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "aux.h"

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_trmm(int m, int n, double alpha, double **A, double **B) {
  int i, j, k;

  //BLAS parameters
  //SIDE   = 'L'
  //UPLO   = 'L'
  //TRANSA = 'T'
  //DIAG   = 'U'
  // => Form  B := alpha*A**T*B.
  // A is MxM
  // B is MxN
  #pragma scop
    for (i = 0; i < m; i++)
      for (j = 0; j < n; j++) {
        for (k = i+1; k < m; k++)
          B[i][j] += A[k][i] * B[k][j];
        B[i][j] = alpha * B[i][j];
      }
  #pragma endscop
}

int main(int argc, char** argv) {
    double **A;
    double **B;
    double alpha;
    int m;
    int n;

    args_parse(argc, argv, "hs:", &m, &n, NULL);

    alloc_array(&A, m, m);
    alloc_array(&B, m, n);
    init_arrays(&alpha, A, B, m, n);

    /* Initialize array(s). */
    init_arrays(&alpha, A, B, m, n);
    // print_array_aux2(A, m, n);
    // print_array_aux2(m, m, POLYBENCH_ARRAY(A));
    // print_array_aux3(m, n, POLYBENCH_ARRAY(B));

    /* Run kernel. */
    kernel_trmm(m, n, alpha, A, B);
    // print_array(B, m, n);
    // checksum(B, m, n);

    /* Be clean. */
    free_array(A, m);
    free_array(B, m);

    return 0;
}
