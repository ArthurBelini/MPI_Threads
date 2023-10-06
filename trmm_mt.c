#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <mpi.h>

#include "aux.h"

double **A;
double **B;
double alpha;
double *recv_array; // Parte de B de cada processo
int m;
int n;
int rank;
int size;
int sendcount;
int qtd_t;

void *kernel_trmm(void *t_id) {
    int i, j, k;
    int offset;

    for(i = 0; i < m; i++) {  // Linhas de B
        for(j = rank + *((int*) t_id) * size; j < n; j += size * qtd_t) {  // Colunas de B
            offset = (j / size) * m;  // Posição inicial de elementos dessa iteração em recv_array

            for(k = i + 1; k < m; k++) {  // Elementos da coluna j
                recv_array[offset + i] += A[k][i] * recv_array[offset + k];
            }
            
            recv_array[offset + i] *= alpha;
        }
    }
}

int main(int argc, char** argv) {
    // ### Declarações ###
    int args_flag;

    // ### Inicialização do MPI ###
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ### Passagem de argumentos ###
    if(rank == 0) {
        args_flag = args_parse(argc, argv, "hpcs:t:", &m, &n, &qtd_t);

        if(args_flag <= 1) {  // Saída normal ou com erro
            MPI_Finalize();

            exit(args_flag);
        }
    }

    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Compartilhamentos de m e n de 0 para todos
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&qtd_t, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // ### Declarações pós inicialização ###
    int i;

    pthread_t *ts = (pthread_t*) malloc(qtd_t * sizeof(pthread_t));
    int *ts_ids = (int*) malloc(qtd_t * sizeof(int));

    double *send_array = (double*) malloc((m * n) * sizeof(double));  // B transformado em 1D
    int *sendcounts = (int*) malloc(size * sizeof(int));  // Quantidades de elementos que cada processo receberá
    int *displs = (int*) malloc(size * sizeof(int));  // Posições iniciais em send_array de cada processo

    for(i = 0; i < size; i++) {
        sendcounts[i] = (n / size) * m;
        sendcounts[i] += (i < (n % size)) ? m : 0;

        displs[i] = (i > 0) ? displs[i - 1] + sendcounts[i - 1] : 0;
    }

    recv_array = (double*) malloc(sendcounts[rank] * sizeof(double));  // Parte de B de cada processo

    // ### Inicialização das matrizes ###
    alloc_array(&A, m, m);  // Matriz A comum entre todos processos
    if(rank == 0) {  // Processo 0 inicia os valores
        alloc_array(&B, m, n);
        init_arrays(&alpha, A, B, m, n);
        flatten_array(send_array, B, m, n, size, displs);
    }

    // ### Compartilhamento de valores e partes de B de 0 para todos ###
    for(i = 0; i < m; i++) {
        MPI_Bcast(A[i], m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatterv(send_array, sendcounts, displs, MPI_DOUBLE, recv_array, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // ### Processamento principal ###
    for(i = 0; i < qtd_t; i++) {  // Criação dos fluxos
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < qtd_t; i++) {  // Unir os fluxos
        pthread_join(ts[i], NULL);
    }

    // ### Processo inverso de reunião dos dados de todos para 0 ###
    MPI_Gatherv(recv_array, sendcounts[rank], MPI_DOUBLE, send_array, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        unflatten_array(send_array, B, m, n, size, sendcounts, displs);

        run_tests(B, m, n, args_flag);  // Testes de resultado
    }

    // ### Finalização ###
    free_array(A, m);
    if(rank == 0) {
        free_array(B, m);
    }
    free(send_array);
    free(recv_array);

    MPI_Finalize();

    return 0;
}
