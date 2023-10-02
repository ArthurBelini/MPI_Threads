#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "aux.h"

static
void kernel_trmm(int m, int n, double alpha, double **A, double **B) {
    int i, j, k;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            for (k = i+1; k < m; k++) {
                B[i][j] += A[k][i] * B[k][j];
            }

            B[i][j] = alpha * B[i][j];
        }
    }
}

int main(int argc, char** argv) {
    double **A;
    double **B;
    double alpha;
    int m;
    int n;

    int ret = args_parse(argc, argv, "hs:", &m, &n, NULL);

    if(ret <= 1) {
        exit(ret);
    }

    alloc_array(&A, m, m);
    alloc_array(&B, m, n);
    init_arrays(&alpha, A, B, m, n);

    init_arrays(&alpha, A, B, m, n);

    kernel_trmm(m, n, alpha, A, B);
    // print_array(B, m, n);
    // checksum(B, m, n);

    free_array(A, m);
    free_array(B, m);

    return 0;
}
