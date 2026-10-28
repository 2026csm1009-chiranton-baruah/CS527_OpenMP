/*
 * CS527 Lab 9 - Task 3: Count even and odd values with reduction and atomic
 * Compile: gcc -O2 -Wall -Wextra -fopenmp task3_reduction.c -o task3
 * Run:     ./task3
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void) {
    const long sizes[] = {100000L, 1000000L, 10000000L, 100000000L};
    const int thread_counts[] = {1, 2, 4, 8};
    puts("N,threads,method,seconds,speedup,efficiency,even,odd,correct");

    for (size_t si = 0; si < sizeof(sizes)/sizeof(sizes[0]); ++si) {
        long n = sizes[si];
        int *a = malloc((size_t)n * sizeof(*a));
        if (!a) {
            fprintf(stderr, "Allocation failed for N=%ld\n", n);
            return EXIT_FAILURE;
        }
        for (long i = 0; i < n; ++i) a[i] = (int)((i * 17L + 3L) % 1000003L);

        long serial_even = 0, serial_odd = 0;
        double start = omp_get_wtime();
        for (long i = 0; i < n; ++i) {
            if (a[i] % 2 == 0) ++serial_even;
            else ++serial_odd;
        }
        double serial_time = omp_get_wtime() - start;

        for (size_t ti = 0; ti < sizeof(thread_counts)/sizeof(thread_counts[0]); ++ti) {
            int threads = thread_counts[ti];
            omp_set_num_threads(threads);

            long even_count = 0, odd_count = 0;
            start = omp_get_wtime();
            #pragma omp parallel for reduction(+:even_count, odd_count)
            for (long i = 0; i < n; ++i) {
                if (a[i] % 2 == 0) ++even_count;
                else ++odd_count;
            }
            double elapsed = omp_get_wtime() - start;
            int correct = even_count == serial_even && odd_count == serial_odd &&
                          even_count + odd_count == n;
            double speedup = elapsed > 0.0 ? serial_time / elapsed : 0.0;
            printf("%ld,%d,reduction,%.6f,%.4f,%.4f,%ld,%ld,%s\n",
                   n, threads, elapsed, speedup, speedup / threads,
                   even_count, odd_count, correct ? "yes" : "NO");

            long atomic_even = 0, atomic_odd = 0;
            start = omp_get_wtime();
            #pragma omp parallel for
            for (long i = 0; i < n; ++i) {
                if (a[i] % 2 == 0) {
                    #pragma omp atomic update
                    atomic_even++;
                } else {
                    #pragma omp atomic update
                    atomic_odd++;
                }
            }
            elapsed = omp_get_wtime() - start;
            correct = atomic_even == serial_even && atomic_odd == serial_odd &&
                      atomic_even + atomic_odd == n;
            speedup = elapsed > 0.0 ? serial_time / elapsed : 0.0;
            printf("%ld,%d,atomic,%.6f,%.4f,%.4f,%ld,%ld,%s\n",
                   n, threads, elapsed, speedup, speedup / threads,
                   atomic_even, atomic_odd, correct ? "yes" : "NO");
        }
        free(a);
    }
    return 0;
}
