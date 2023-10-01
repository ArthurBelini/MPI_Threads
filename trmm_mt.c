#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <mpi.h>

#include "aux.h"

double alpha;
double **A;
double **B;
double *recv_array;
int m;
int n;
int rank;
int size;
int sendcount;
int qtd_t;

void *kernel_trmm(void *t_id) {
    int i, j, k, offset;

    for(i = 0; i < m; i++) {  // Linha
        for(j = rank + *(int*) t_id * size; j < n; j += size * qtd_t) {  // Coluna
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

    if(rank == 0) {
        int ret = args_parse(argc, argv, "hs:t:", &m, &n, &qtd_t);

        if(ret <= 1) {
            MPI_Finalize();
            exit(ret);
        }
    }

    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&qtd_t, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int i;
    int a_size;
    int b_size;
    int *sendcounts;
    int *displs;
    double *send_array;

    pthread_t *ts = (pthread_t*) malloc(qtd_t*sizeof(pthread_t));
    int *ts_ids = (int*) malloc(qtd_t*sizeof(int));

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

    for(i = 0; i < qtd_t; i++) {
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < qtd_t; i++) {
        pthread_join(ts[i], NULL);
    }

    MPI_Gatherv(recv_array, sendcounts[rank], MPI_DOUBLE, send_array, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        unflatten_array(send_array, B, m, n, size, sendcounts, displs);
        // print_array_aux(B, m, n);
        checksum(B, m, n);
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
