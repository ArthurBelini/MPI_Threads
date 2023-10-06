#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#ifndef AUX_H
#define AUX_H

// Função auxiliar de args_parse
// Verifica se string pode ser convertida para um número inteiro
int is_integer(const char *str) {
    char *str_aux;

    strtol(str, &str_aux, 10);  // Tenta converter

    if (*str_aux != '\0' || str_aux == str) {  // Casos inválidos
        return 0;
    }

    return 1;
}

// Flags de retorno: 0 - saída sem erro, 1 - saída com erro,
// 2 - sem erro, 3 - print de resultado, 4, checksum do resultado,
// 5 - os dois últimos
// Essa função recebe os argumentos dos códigos trmm_s, trmm_t, trmm_m e trmm_mt 
int args_parse(int argc, char** argv, char *opts, int *m, int *n, int *qtd_t) {
    int opt;

    int ret = 2;  // Inicia como retorno normal, sem flags
    char size = '\0';

    if(qtd_t != NULL) {
        *qtd_t = '\0';
    }

    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
            case 'h':
                if(qtd_t != NULL) {
                    printf("Usage: %s -s <size> -t <qtd_t> -h -p -c\n", argv[0]);
                } else {
                    printf("Usage: %s -s <size> -h -p -c\n", argv[0]);
                }

                printf("Options:\n");
                printf("  -s <size>     Size ('s', 'm', or 'l')\n");

                if(qtd_t != NULL) {
                    printf("  -t <qtd_t>    Qtd_t (> 0)\n");
                }

                printf("  -h            Show this help message\n");
                printf("  -p            Print the result\n");
                printf("  -c            Print the checksum\n");

                return 0;

            case 'p':
                ret++;  // Altera a flag de retorno
                break;

            case 'c':
                ret += 2;  // Altera a flag de retorno
                break;

            case 's':
                size = optarg[0];

                switch(size) {
                    case 't':  // Tamanho de testes
                        *m = 10;
                        *n = 15;
                        
                        break;

                    case 's':  // Tamanho pequeno
                        *m = 3000;
                        *n = 4000;

                        break;

                    case 'm':  // Tamanho médio
                        *m = 4000;
                        *n = 4500;

                        break;

                    case 'l':  // Tamanho grande
                        *m = 4500;
                        *n = 5000;

                        break;

                    default:
                        fprintf(stderr, "Invalid size value. Use 's', 'm', or 'l'.\n");

                        return 1;
                }
                break;

            case 't':
                if(!is_integer(optarg) || (*qtd_t = atoi(optarg)) <= 0) {
                    fprintf(stderr, "Invalid -t value. Must be an integer > 0.\n");

                    return 1;
                }
                
                break;

            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);

                return 1;

            case '?':
                if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }

                return 1;

            default:
                if(qtd_t == NULL) {
                    fprintf(stderr, "Usage: %s -s <size> -h\n", argv[0]);
                } else {
                    fprintf(stderr, "Usage: %s -s <size> -t <qtd_t> -h\n", argv[0]);
                }

                return 1;
        }
    }

    if(size == '\0' || (qtd_t != NULL && *qtd_t == '\0')) {  // Se requer threads (-t) e não foi selecionado ou
                                                             // tamanho (-s) não selecionado
        if(size == '\0') {  //tamanho (-s) não selecionado 
            fprintf(stderr, "Option -s requires an argument.\n");
        }

        if(qtd_t != NULL && *qtd_t == '\0') {  // Se requer threads (-t) e não foi selecionado
            fprintf(stderr, "Option -t requires an argument.\n");
        }

        return 1;  // Qualquer um desses casos retorna erro
    }

    return ret;  // 3 - p, 4 - c, 5 - p e c
}

// Aloca memória para a matriz M por referência com dimensões m x n
void alloc_array(double ***M, int m, int n) {
    *M = (double**) malloc(m * sizeof(double*));

    if (*M == NULL) {
        exit(1);
    }

    for(int i = 0; i < m; i++) {
        (*M)[i] = (double*) malloc(n * sizeof(double));

        if ((*M)[i] == NULL) {
            exit(1);
        }
    }
}

// Preenche as matrizes A e B com dimensões "m x m" e "m x n" respectivamente e a constante alpha
// com os valores determinados pelo código sequencial do polybench
void init_arrays(double *alpha, double **A, double **B, int m, int n) {
    int j;

    *alpha = 1.5;

    for (int i = 0; i < m; i++) {
        for (j = 0; j < i; j++) {
            A[i][j] = (double) ((i + j) % m) / m;
        }

        A[i][i] = 1.0;

        for (j = 0; j < n; j++) {
            B[i][j] = (double) ((n + (i - j)) % n) / n;
        }
    }
}

// Libera memória da matriz M com m linhas
void free_array(double **M, int m) {
    for(int i = 0; i < m; i++) {
        free(M[i]);
    }

    free(M);
}

// Transforma matriz 2D B em 1D com colunas de cada processo em sequência 
void flatten_array(double *send_array, double **B, int m, int n, int size, int *displs) {
    int offset;

    for(int r = 0; r < size; r++) {
        for(int j = r; j < n; j += size) {
            for(int i = 0; i < m; i++) {
                offset = (j / size) * m;  // Posição inicial de elementos dessa iteração em send_array

                send_array[displs[r] + offset + i] = B[i][j];
            }
        }
    }
}

// Processo inverso de flatten_array
void unflatten_array(double *send_array, double **B, int m, int n, int size, int *sendcounts, int *displs) {
    int offset;

    for(int r = 0; r < size; r++) {
        for(int j = r; j < n; j += size) {
            for(int i = 0; i < m; i++) {
                offset = (j / size) * m;

                B[i][j] = send_array[displs[r] + offset + i];
            }
        }
    }
}

// Imprimi valores da matriz M de dimensões m x n
void print_array(double **M, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.2f ", M[i][j]);
        }

        printf("\n");
    }

    printf("\n");
}

// Soma todos os elementos da matriz M como forma de comparar resultados entre programas
void checksum(double **M, int m, int n) {
    double sum = 0;  // Onde será armazenado a soma

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            sum += M[i][j];
        }
    }

    printf("Checksum: %.2f\n", sum);
}

// Executa testes de resultado de acordo com as flags
void run_tests(double **B, int m, int n, int args_flag) {
    switch(args_flag) {
        case 3:
            print_array(B, m, n);
            break;

        case 4:
            checksum(B, m, n);
            break;

        case 5:
            print_array(B, m, n);
            checksum(B, m, n);
            break;

        default:
            break;
        
    }
}

#endif