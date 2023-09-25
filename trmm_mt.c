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
#include <mpi.h>

/* Include benchmark-specific header. */
#include "trmm.h"
#include "aux.h"

#define QTD_T 2

/* Variable declaration/allocation. */
DATA_TYPE alpha;
DATA_TYPE **A;
DATA_TYPE **B;
DATA_TYPE *recv_array;
int m;
int n;
int rank;
int size;
int sendcount;

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void *kernel_trmm(void *t_id) {
    // printf("aoba");
    int i, j, k, offset;

    //BLAS parameters
    //SIDE   = 'L'
    //UPLO   = 'L'
    //TRANSA = 'T'
    //DIAG   = 'U'
    // => Form  B := alpha*A**T*B.
    // A is MxM
    // B is MxN
    for(i = 0; i < m; i++) {  // Linha
        for(j = rank + *(int*) t_id * size; j < n; j += size * QTD_T) {  // Coluna
            // printf("%d %d %d\n", *(int*) t_id, i, j);
            offset = j/size*m;

            for(k = i+1; k < m; k++) {  // Elementos
                recv_array[offset + i] += A[k][i] * recv_array[offset + k];
            }
            
            recv_array[offset + i] *= alpha;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int i;
    int a_size;
    int b_size;
    int *sendcounts;
    int *displs;
    int ts_ids[QTD_T];
    DATA_TYPE *send_array;
    pthread_t ts[QTD_T];

    m = M;
    n = N;
    a_size = m*m;
    b_size = m*n;
    send_array = (DATA_TYPE*) calloc(b_size, sizeof(DATA_TYPE));
    sendcounts = (int*) calloc(n, sizeof(int));
    displs = (int*) calloc(n, sizeof(int));
    for(i = 0; i < n; i++) {
        sendcounts[i % size]++;
    }
    for(i = 0; i < size; i++) {
        sendcounts[i] *= m;

        displs[i] = (i > 0) ? displs[i-1] + sendcounts[i-1] : 0;
    }

    recv_array = (DATA_TYPE*) calloc(sendcounts[rank], sizeof(DATA_TYPE));

    alloc_array_aux(&A, m, m);
    if(rank == 0) {
        alloc_array_aux(&B, m, n);
        init_array_aux(&alpha, A, B, m, n);
        flatten_array(send_array, B, m, n, size, sendcounts, displs);
    }

    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(i = 0; i < m; i++) {
        MPI_Bcast(A[i], m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    MPI_Scatterv(send_array, sendcounts, displs, MPI_DOUBLE, recv_array, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // print_array_aux(&recv_array, 1, sendcounts[rank]);

    for(i = 0; i < QTD_T; i++) {
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < QTD_T; i++) {
        pthread_join(ts[i], NULL);
    }

    // print_array_aux(&recv_array, 1, sendcounts[rank]);

    MPI_Gatherv(recv_array, sendcounts[rank], MPI_DOUBLE, send_array, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // print_array_aux(&recv_array, 1, div_size);

    if(rank == 0) {
        unflatten_array(send_array, B, m, n, size, sendcounts, displs);
        // print_array_aux(B, m, n);
        // checksum2(B, m, n);
    }

    free_array_aux(A, m);
    if(rank == 0) {
        free_array_aux(B, m);
    }
    free(send_array);
    free(recv_array);

    MPI_Finalize();

    return 0;
}
