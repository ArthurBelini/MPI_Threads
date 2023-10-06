#include "aux.h"

void kernel_trmm(double **A, double **B, double alpha, int m, int n) {
    int i, j, k;

    for(i = 0; i < m; i++) {  // Linhas de B
        for(j = 0; j < n; j++) {  // Colunas de B
            for(k = i + 1; k < m; k++) {  // Elementos da coluna j
                B[i][j] += A[k][i] * B[k][j];
            }

            B[i][j] *= alpha;
        }
    }
}

int main(int argc, char** argv) {
    // ### Declarações ###
    double **A, **B;
    double alpha;
    int m, n;

    // ### Passagem de argumentos ###
    int args_flag = args_parse(argc, argv, "hpcs:", &m, &n, NULL);

    if(args_flag <= 1) {  // Saída normal ou com erro
        exit(args_flag);
    }

    // ### Inicialização das matrizes ###
    alloc_array(&A, m, m);
    alloc_array(&B, m, n);
    
    init_arrays(&alpha, A, B, m, n);

    // ### Processamento principal ###
    kernel_trmm(A, B, alpha, m, n);

    run_tests(B, m, n, args_flag);  // Testes de resultado

    // ### Finalização ###
    free_array(A, m);
    free_array(B, m);

    return 0;
}
