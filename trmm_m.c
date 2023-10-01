/**MN
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

#include "aux.h"

double alpha;
double **A;
double **B;
int m;
int n;

void kernel_trmm(double *recv_array, int rank, int size, int sendcount) {
    int i, j, k, offset;

    for(i = 0; i < m; i++) {  // Linha
        for(j = rank; j < n; j+=size) {  // Coluna
            // printf("%d\n", j);
            offset = j/size*m;

            for(k = i+1; k < m; k++) {  // Elementos
                recv_array[offset + i] += A[k][i] * recv_array[offset + k];
            }
            
            recv_array[offset + i] *= alpha;
        }
    }
}

int main(int argc, char** argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0) {
        int ret;

        if((ret = args_parse(argc, argv, "hs:", &m, &n, NULL)) <= 1) {
            MPI_Finalize();
            exit(ret);
        }
    }

    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int i;
    int a_size; 
    int b_size;
    double *send_array;
    double *recv_array;
    int *sendcounts;
    int *displs;

    a_size = m*m;
    b_size = m*n;
    send_array = (double*) calloc(b_size, sizeof(double));
    sendcounts = (int*) calloc(size, sizeof(int));
    displs = (int*) calloc(size, sizeof(int));
    for(i = 0; i < n; i++) {
        sendcounts[i % size]++;
    }
    for(i = 0; i < size; i++) {
        sendcounts[i] *= m;

        displs[i] = (i > 0) ? displs[i-1] + sendcounts[i-1] : 0;
    }

    recv_array = (double*) calloc(sendcounts[rank], sizeof(double));

    alloc_array(&A, m, m);
    if(rank == 0) {
        alloc_array(&B, m, n);
        init_arrays(&alpha, A, B, m, n);
        flatten_array(send_array, B, m, n, size, sendcounts, displs);
    }

    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(i = 0; i < m; i++) {
        MPI_Bcast(A[i], m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    MPI_Scatterv(send_array, sendcounts, displs, MPI_DOUBLE, recv_array, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    kernel_trmm(recv_array, rank, size, sendcounts[rank]);

    MPI_Gatherv(recv_array, sendcounts[rank], MPI_DOUBLE, send_array, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        unflatten_array(send_array, B, m, n, size, sendcounts, displs);
        // print_array_aux(B, m, n);
        // checksum(B, m, n);
    }

    free_array(A, m);
    if(rank == 0) {
        free_array(B, m);
    }
    free(send_array);
    free(recv_array);

    MPI_Finalize();

    return 0;
}
