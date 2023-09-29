#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#ifndef AUX_H
#define AUX_H

int isInteger(const char *str) {
    char *endptr; // Pointer to the first character that couldn't be converted
    strtol(str, &endptr, 10); // Attempt to convert the string to a long integer base 10

    // Check if conversion failed or if there are any non-digit characters
    if (*endptr != '\0' || endptr == str) {
        return 0; // Conversion failed
    }

    return 1; // Conversion successful
}

void args_parse(int argc, char** argv, char *opts, int *m, int *n, int *qtd_t) {  // Arrumar bugs!!! 
    int opt;
    char size = '\0';

    if(qtd_t != NULL) {
        *qtd_t = '\0';
    }

    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
            case 'h':
                if(qtd_t == NULL) {
                    printf("Usage: %s -s <size> -h\n", argv[0]);
                    printf("Options:\n");
                    printf("  -s <size>     Size ('s', 'm', or 'l')\n");
                    printf("  -h            Show this help message\n");
                }
                printf("Usage: %s -s <size> -h\n", argv[0]);
                printf("Options:\n");
                printf("  -s <size>     Size ('s', 'm', or 'l')\n");
                printf("  -t <qtd_t>    Qtd_t (> 0)\n");
                printf("  -h            Show this help message\n");
                
                exit(0);

            case 's':
                size = optarg[0];

                // 4000 5000 39
                // 4000 6000 46
                // 5000 6000 11
                switch(size) {
                    case 's':
                        *m = 50;
                        *n = 50;
                        break;
                    case 'm':
                        *m = 50;
                        *n = 50;
                        break;
                    case 'l':
                        *m = 50;
                        *n = 50;
                        break;
                    default:
                        fprintf(stderr, "Invalid size value. Use 's', 'm', or 'l'.\n");
                        exit(1);
                }
                break;

            case 't':
                if((*qtd_t = atoi(optarg)) <= 0 || !isInteger(optarg)) {
                    fprintf(stderr, "Invalid -t value. Must be an integer > 0.\n");
                    exit(1);
                }
                
                break;

            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);

                exit(1);

            case '?':
                if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }

                exit(1);

            default:
                if(qtd_t == NULL) {
                    fprintf(stderr, "Usage: %s -s <size> -h\n", argv[0]);
                }
                fprintf(stderr, "Usage: %s -s <size> -t <qtd_t> -h\n", argv[0]);

                exit(1);
        }
    }

    if(size == '\0' || (qtd_t != NULL && *qtd_t == '\0')) {
        if(size == '\0') {
            fprintf(stderr, "Option -s requires an argument.\n");
        }

        if(qtd_t != NULL && *qtd_t == '\0') {
            fprintf(stderr, "Option -t requires an argument.\n");
        }

        exit(1);
    }
}

void alloc_array(double ***C, int m, int n) {
    int i;

    *C = (double**) calloc(m, sizeof(double*));

    // if (*C == NULL) {
    //     // Handle memory allocation error here (e.g., print an error message and exit)
    //     perror("Memory allocation failed for row pointers");
    //     exit(EXIT_FAILURE);
    // }

    for(i = 0; i < m; i++) {
        (*C)[i] = (double*) calloc(n, sizeof(double));

        // if ((*C)[i] == NULL) {
        //     // Handle memory allocation error for a specific row
        //     perror("Memory allocation failed for row");
        //     exit(EXIT_FAILURE);
        // }
    }
}

void init_arrays(double *alpha, double **A, double **B, int m, int n) {
    int i, j;

    *alpha = 1.5;

    for (i = 0; i < m; i++) {
        for (j = 0; j < i; j++) {
            A[i][j] = (double)((i+j) % m)/m;
        }
        A[i][i] = 1.0;
        for (j = 0; j < n; j++) {
            B[i][j] = (double)((n+(i-j)) % n)/n;
        }
    }
}

void free_array(double **A, int m) {
    int i;

    for(i = 0; i < m; i++) {
        free(A[i]);
    }

    free(A);
}

void flatten_array(double *send_array, double **B, int m, int n, int size, int *sendcounts, int *displs) {
    int offset;

    for(int r = 0; r < size; r++) {
        for(int j = r; j < n; j+=size) {
            for(int i = 0; i < m; i++) {
                offset = j/size*m;

                send_array[displs[r] + offset + i] = B[i][j];
            }
        }
    }
}

void unflatten_array(double *send_array, double **B, int m, int n, int size, int *sendcounts, int *displs) {
    int offset;

    for(int r = 0; r < size; r++) {
        for(int j = r; j < n; j+=size) {
            for(int i = 0; i < m; i++) {
                offset = j/size*m;

                B[i][j] = send_array[displs[r] + offset + i];
            }
        }
    }
}

void print_array(double **A, int m, int n) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%.2f ", A[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void print_array_int(int **A, int m, int n) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void checksum(double **B, int m, int n) {
    double sum = 0;

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            sum += B[i][j];
        }
    }

    printf("Checksum: %.2f\n", sum);
}

#endif