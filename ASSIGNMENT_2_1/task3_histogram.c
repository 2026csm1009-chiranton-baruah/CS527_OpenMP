#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void) {
    const int n = 1000000;
    int *a = malloc((size_t)n * sizeof(*a));
    if (a == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) a[i] = i % 10;

    long long histogram[10] = {0};
    long long total = 0;

    double start = omp_get_wtime();
    /* Each thread gets a private 10-bin histogram; OpenMP combines bins. */
    #pragma omp parallel for reduction(+:histogram[:10])
    for (int i = 0; i < n; ++i) {
        if (a[i] >= 0 && a[i] <= 9) histogram[a[i]]++;
    }
    double elapsed = omp_get_wtime() - start;

    for (int i = 0; i < 10; ++i) {
        printf("%d: %lld\n", i, histogram[i]);
        total += histogram[i];
    }
    printf("Total counted: %lld (expected %d)\n", total, n);
    printf("Threads: %d\nParallel histogram time: %.6f s\n",
           omp_get_max_threads(), elapsed);

    free(a);
    return total == n ? EXIT_SUCCESS : EXIT_FAILURE;
}
