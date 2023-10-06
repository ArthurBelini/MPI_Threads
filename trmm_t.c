#include <pthread.h>

#include "aux.h"

double **A;
double **B;
double alpha;
int m;
int n;
int qtd_t;

void *kernel_trmm(void *t_id) {
    int i, j, k;

    for(int i = 0; i < m; i++) {  // Linhas de B
        for(int j = *((int *) t_id); j < n; j += qtd_t) {  // Colunas de B
            for(int k = i + 1; k < m; k++) {  // Elementos da coluna j
                B[i][j] += A[k][i] * B[k][j];
            }

            B[i][j] = alpha * B[i][j];
        }
    }
}

int main(int argc, char** argv) {
    // ### Passagem de argumentos ###
    int args_flag = args_parse(argc, argv, "hpcs:t:", &m, &n, &qtd_t);

    if(args_flag <= 1) {  // Saída normal ou com erro
        exit(args_flag);
    }

    // ### Declarações de Threads e seus Ids ###
    pthread_t *ts = (pthread_t*) malloc(qtd_t * sizeof(pthread_t));
    int *ts_ids = (int*) malloc(qtd_t * sizeof(int));

    // ### Inicialização das matrizes ###
    alloc_array(&A, m, m);
    alloc_array(&B, m, n);

    init_arrays(&alpha, A, B, m, n);

    // ### Processamento principal ###
    int i;

    for(i = 0; i < qtd_t; i++) {  // Criação dos fluxos
        ts_ids[i] = i;

        pthread_create(&ts[i], NULL, &kernel_trmm, &ts_ids[i]);
    }

    for(i = 0; i < qtd_t; i++) {  // Unir os fluxos
        pthread_join(ts[i], NULL);
    }

    run_tests(B, m, n, args_flag);  // Testes de resultado

    // ### Finalização ###
    free_array(A, m);
    free_array(B, m);

    return 0;
}
