#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static void print_array(const int *a, int n) {
    for (int i = 0; i < n; ++i) printf("%d%s", a[i], i + 1 == n ? "\n" : " ");
}

static int is_sorted(const int *a, int n) {
    for (int i = 1; i < n; ++i) if (a[i - 1] > a[i]) return 0;
    return 1;
}

/* Odd-even transposition sort: each phase has independent adjacent pairs.
   The implicit barrier at the end of each omp for keeps phases in order. */
int main(void) {
    const int n = 12;
    int a[] = {29, 4, 17, 8, 1, 33, 12, 7, 25, 3, 18, 2};

    double start = omp_get_wtime();
    #pragma omp parallel shared(a)
    {
        for (int phase = 0; phase < n; ++phase) {
            int first = phase % 2;
            #pragma omp for
            for (int i = first; i < n - 1; i += 2) {
                if (a[i] > a[i + 1]) {
                    int tmp = a[i];
                    a[i] = a[i + 1];
                    a[i + 1] = tmp;
                }
            }
            /* omp for has an implicit barrier: required between phases. */
        }
    }
    double elapsed = omp_get_wtime() - start;

    printf("Sorted array: ");
    print_array(a, n);
    printf("Correctness: %s\n", is_sorted(a, n) ? "PASS" : "FAIL");
    printf("Threads: %d\nParallel sort time: %.6f s\n",
           omp_get_max_threads(), elapsed);

    return is_sorted(a, n) ? EXIT_SUCCESS : EXIT_FAILURE;
}
