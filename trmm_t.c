#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "aux.h"

double alpha;
double **A;
double **B;
int m;
int n;
int qtd_t;

void *kernel_trmm(void *t_id) {
    int i, j, k;

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
    int i;

    int ret = args_parse(argc, argv, "hs:t:", &m, &n, &qtd_t);

    if(ret <= 1) {
        exit(ret);
    }

    pthread_t *ts = (pthread_t*) malloc(qtd_t*sizeof(pthread_t));
    int *ts_ids = (int*) malloc(qtd_t*sizeof(int));

    alloc_array(&A, m, m);
    alloc_array(&B, m, n);
    init_arrays(&alpha, A, B, m, n);

    for(i = 0; i < qtd_t; i++) {
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < qtd_t; i++) {
        pthread_join(ts[i], NULL);
    }

    // print_array_aux(B, m, n);
    checksum(B, m, n);

    free_array(A, m);
    free_array(B, m);

    return 0;
}
