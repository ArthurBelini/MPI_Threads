#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "polybench.h"
#include "trmm.h"

#ifndef AUX_H
#define AUX_H

void alloc_array_aux(DATA_TYPE ***C, int m, int n) {
    int i;

    *C = (DATA_TYPE**) calloc(m, sizeof(DATA_TYPE*));

    // if (*C == NULL) {
    //     // Handle memory allocation error here (e.g., print an error message and exit)
    //     perror("Memory allocation failed for row pointers");
    //     exit(EXIT_FAILURE);
    // }

    for(i = 0; i < m; i++) {
        (*C)[i] = (DATA_TYPE*) calloc(n, sizeof(DATA_TYPE));

        // if ((*C)[i] == NULL) {
        //     // Handle memory allocation error for a specific row
        //     perror("Memory allocation failed for row");
        //     exit(EXIT_FAILURE);
        // }
    }
}

void init_array_aux(DATA_TYPE *alpha, DATA_TYPE **A, DATA_TYPE **B, int m, int n) {
    int i, j;

    *alpha = 1.5;

    for (i = 0; i < m; i++) {
        for (j = 0; j < i; j++) {
            A[i][j] = (DATA_TYPE)((i+j) % m)/m;
        }
        A[i][i] = 1.0;
        for (j = 0; j < n; j++) {
            B[i][j] = (DATA_TYPE)((n+(i-j)) % n)/n;
        }
    }
}

void free_array_aux(DATA_TYPE **C, int m) {
    int i;

    for(i = 0; i < m; i++) {
        free(C[i]);
    }

    free(C);
}

void flatten_array(DATA_TYPE *send_array, DATA_TYPE **B, int m, int n, int size, int *sendcounts, int *displs) {
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

void unflatten_array(DATA_TYPE *send_array, DATA_TYPE **B, int m, int n, int size, int *sendcounts, int *displs) {
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

void print_array_aux(DATA_TYPE **A, int m, int n) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%.2f ", A[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void print_array_aux2(int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,M,M,m,m)) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < m; j++) {
            printf("%.2f ", A[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void print_array_aux3(int m, int n,
		 DATA_TYPE POLYBENCH_2D(B,M,N,m,n)) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%.2f ", B[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void print_array_aux4(int **A, int m, int n) {
    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void checksum1(DATA_TYPE POLYBENCH_2D(B,M,N,m,n), int m, int n) {
    double sum = 0;

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            sum += B[i][j];
        }
    }

    printf("Checksum: %.2f\n", sum);
}

void checksum2(DATA_TYPE **B, int m, int n) {
    double sum = 0;

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            sum += B[i][j];
        }
    }

    printf("Checksum: %.2f\n", sum);
}

#endif