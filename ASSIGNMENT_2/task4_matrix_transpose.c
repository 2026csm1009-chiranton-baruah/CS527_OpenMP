#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ROWS 4
#define COLS 5

int main(void) {
    int a[ROWS][COLS];
    int transposed[COLS][ROWS];

    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            a[i][j] = i * COLS + j + 1;

    double start = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            transposed[j][i] = a[i][j];
        }
    }
    double elapsed = omp_get_wtime() - start;

    printf("Input (%d x %d):\n", ROWS, COLS);
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) printf("%3d ", a[i][j]);
        putchar('\n');
    }
    printf("Transpose (%d x %d):\n", COLS, ROWS);
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < ROWS; ++j) printf("%3d ", transposed[i][j]);
        putchar('\n');
    }

    int correct = 1;
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            if (transposed[j][i] != a[i][j]) correct = 0;

    printf("Correctness: %s\n", correct ? "PASS" : "FAIL");
    printf("Threads: %d\nParallel transpose time: %.6f s\n",
           omp_get_max_threads(), elapsed);
    return correct ? EXIT_SUCCESS : EXIT_FAILURE;
}
