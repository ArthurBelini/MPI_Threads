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
#include <pthread.h>

/* Include benchmark-specific header. */
#include "trmm.h"
#include "aux.h"

/* Variable declaration/allocation. */
DATA_TYPE alpha;
DATA_TYPE **A;
DATA_TYPE **B;
int m;
int n;
int qtd_t;

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void *kernel_trmm(void *t_id) {
    int i, j, k;

    //BLAS parameters
    //SIDE   = 'L'
    //UPLO   = 'L'
    //TRANSA = 'T'
    //DIAG   = 'U'
    // => Form  B := alpha*A**T*B.
    // A is MxM
    // B is MxN
    for(int i = 0; i < m; i++) {  // Linha
        for(int j = *(int *)t_id; j < n; j+=qtd_t) {  // Coluna
            for(int k = i+1; k < m; k++) {  // Elementos
                B[i][j] += A[k][i] * B[k][j];
            }

            B[i][j] = alpha * B[i][j];
        }
    }
}

int main(int argc, char** argv) {
    args_parse(argc, argv, "hs:t:", &m, &n, &qtd_t);

    pthread_t *ts = (pthread_t*) malloc(qtd_t*sizeof(pthread_t));
    int *ts_ids = (int*) malloc(qtd_t*sizeof(int));
    int i;

    alloc_array_aux(&A, m, m);
    alloc_array_aux(&B, m, n);
    init_array_aux(&alpha, A, B, m, n);
    // print_array_aux(A, m, m);
    // print_array_aux(B, m, n);

    /* Run kernel. */
    for(i = 0; i < qtd_t; i++) {
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < qtd_t; i++) {
        pthread_join(ts[i], NULL);
    }

    // print_array_aux(B, m, n);
    checksum2(B, m, n);

    /* Be clean. */
    free_array_aux(A, m);
    free_array_aux(B, m);

    return 0;
}
