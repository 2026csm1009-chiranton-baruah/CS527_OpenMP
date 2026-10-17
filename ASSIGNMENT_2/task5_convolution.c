#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 5
#define K 3
#define OUT (N - K + 1)

int main(void) {
    const int image[N][N] = {
        {10, 20, 30, 40, 50},
        {60, 70, 80, 90, 100},
        {110, 120, 130, 140, 150},
        {160, 170, 180, 190, 200},
        {210, 220, 230, 240, 250}
    };
    const int kernel[K][K] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };
    double output[OUT][OUT];

    double start = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < OUT; ++i) {
        for (int j = 0; j < OUT; ++j) {
            int sum = 0;
            for (int ki = 0; ki < K; ++ki)
                for (int kj = 0; kj < K; ++kj)
                    sum += image[i + ki][j + kj] * kernel[ki][kj];
            output[i][j] = (double)sum / 9.0;
        }
    }
    double elapsed = omp_get_wtime() - start;

    printf("Valid 3x3 averaging-filter output (%dx%d):\n", OUT, OUT);
    for (int i = 0; i < OUT; ++i) {
        for (int j = 0; j < OUT; ++j) printf("%7.2f ", output[i][j]);
        putchar('\n');
    }

    int correct = fabs(output[0][0] - 70.0) < 1e-9;
    printf("First output expected 70.00: %s\n", correct ? "PASS" : "FAIL");
    printf("Threads: %d\nParallel convolution time: %.6f s\n",
           omp_get_max_threads(), elapsed);
    return correct ? EXIT_SUCCESS : EXIT_FAILURE;
}
