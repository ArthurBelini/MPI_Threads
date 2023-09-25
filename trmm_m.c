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
#include <mpi.h>

/* Include benchmark-specific header. */
#include "trmm.h"
#include "aux.h"

/* Variable declaration/allocation. */
DATA_TYPE alpha;
DATA_TYPE **A;
DATA_TYPE **B;
int m;
int n;

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void kernel_trmm(DATA_TYPE *recv_array, int rank, int size, int sendcount) {
    int i, j, k, offset;

    //BLAS parameters
    //SIDE   = 'L'
    //UPLO   = 'L'
    //TRANSA = 'T'
    //DIAG   = 'U'
    // => Form  B := alpha*A**T*B.
    // A is MxM
    // B is MxN
    // print_array_aux(A, m, m);
    // print_array_aux(&recv_array, 1, div_size);
    for(i = 0; i < m; i++) {  // Linha
        for(j = rank; j < n; j+=size) {  // Coluna
            // printf("%d\n", j);
            offset = j/size*m;

            for(k = i+1; k < m; k++) {  // Elementos
                // printf("%d %d\n", j, k);
                // printf("%d\n", div_offset + i);
                // printf("%f %f\n", A[k][i], recv_array[div_offset + k]);
                recv_array[offset + i] += A[k][i] * recv_array[offset + k];
            }
            
            recv_array[offset + i] *= alpha;
        }
    }
    // print_array_aux(&recv_array, 1, div_size);
    // printf("\n\n\n");
}

int main(int argc, char** argv) {
    int i, a_size, b_size, rank, size, x;
    DATA_TYPE *send_array;
    int *sendcounts;
    int *displs;
    DATA_TYPE *recv_array;
    // DATA_TYPE *send_a;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Retrieve problem size. */
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
    // print_array_aux4(&sendcounts, 1, size);
    for(i = 0; i < size; i++) {
        sendcounts[i] *= m;

        displs[i] = (i > 0) ? displs[i-1] + sendcounts[i-1] : 0;
    }

    // print_array_aux4(&sendcounts, 1, size);
    // print_array_aux4(&displs, 1, size);

    recv_array = (DATA_TYPE*) calloc(sendcounts[rank], sizeof(DATA_TYPE));

    alloc_array_aux(&A, m, m);
    // printf("%ld %ld\n", sizeof(A), sizeof(A[0][0]));
    if(rank == 0) {
        alloc_array_aux(&B, m, n);
        // printf("%d\n", (int) sizeof(B));
        init_array_aux(&alpha, A, B, m, n);
        flatten_array(send_array, B, m, n, size, sendcounts, displs);
        // print_array_aux(A, m, m);
        // print_array_aux(B, m, n);
        // print_array_aux(&send_array, 1, b_size);
    }

    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(i = 0; i < m; i++) {
        MPI_Bcast(A[i], m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    // unflatten_array(send_a, A, m, m, div_size, size);

    // print_array_aux(A, m, m);

    MPI_Scatterv(send_array, sendcounts, displs, MPI_DOUBLE, recv_array, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // print_array_aux(&recv_array, 1, sendcounts[rank]);

    kernel_trmm(recv_array, rank, size, sendcounts[rank]);

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
