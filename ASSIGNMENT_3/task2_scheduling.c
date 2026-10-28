/*
 * CS527 Lab 9 - Task 2: OpenMP scheduling with uneven iteration costs
 * Compile: gcc -O2 -Wall -Wextra -fopenmp task2_scheduling.c -o task2
 * Run:     ./task2
 *
 * Work is intentionally uneven: iteration i executes (i % 1000) * 10
 * arithmetic steps. The result is stored so the computation is observable.
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static double do_work(double x, int work) {
    for (int j = 0; j < work; ++j)
        x = x * 1.0000001 + 0.0000001;
    return x;
}

int main(void) {
    const long sizes[] = {100000L, 1000000L, 10000000L};
    const int thread_counts[] = {1, 2, 4, 8};
    const int chunk_size = 100;
    puts("N,threads,method,seconds,speedup,efficiency,checksum");

    for (size_t si = 0; si < sizeof(sizes)/sizeof(sizes[0]); ++si) {
        long n = sizes[si];
        double *input = malloc((size_t)n * sizeof(*input));
        double *serial_out = malloc((size_t)n * sizeof(*serial_out));
        double *out = malloc((size_t)n * sizeof(*out));
        if (!input || !serial_out || !out) {
            fprintf(stderr, "Allocation failed for N=%ld\n", n);
            free(input); free(serial_out); free(out);
            return EXIT_FAILURE;
        }
        for (long i = 0; i < n; ++i) input[i] = (double)i * 0.001;

        double start = omp_get_wtime();
        for (long i = 0; i < n; ++i) {
            int work = (int)(i % 1000L) * 10;
            serial_out[i] = do_work(input[i], work);
        }
        double serial_time = omp_get_wtime() - start;
        double serial_sum = 0.0;
        for (long i = 0; i < n; ++i) serial_sum += serial_out[i];

        for (int method = 0; method < 3; ++method) {
            for (size_t ti = 0; ti < sizeof(thread_counts)/sizeof(thread_counts[0]); ++ti) {
                int threads = thread_counts[ti];
                omp_set_num_threads(threads);
                start = omp_get_wtime();

                if (method == 0) {
                    #pragma omp parallel for schedule(static)
                    for (long i = 0; i < n; ++i) {
                        int work = (int)(i % 1000L) * 10;
                        out[i] = do_work(input[i], work);
                    }
                } else if (method == 1) {
                    #pragma omp parallel for schedule(dynamic, chunk_size)
                    for (long i = 0; i < n; ++i) {
                        int work = (int)(i % 1000L) * 10;
                        out[i] = do_work(input[i], work);
                    }
                } else {
                    #pragma omp parallel for schedule(guided)
                    for (long i = 0; i < n; ++i) {
                        int work = (int)(i % 1000L) * 10;
                        out[i] = do_work(input[i], work);
                    }
                }
                double elapsed = omp_get_wtime() - start;
                double sum = 0.0;
                for (long i = 0; i < n; ++i) sum += out[i];
                const char *name = method == 0 ? "static" :
                                   method == 1 ? "dynamic_100" : "guided";
                double speedup = elapsed > 0.0 ? serial_time / elapsed : 0.0;
                printf("%ld,%d,%s,%.6f,%.4f,%.4f,%.12e\n",
                       n, threads, name, elapsed, speedup, speedup / threads, sum);
                /* A checksum mismatch can indicate a bug or a changed workload. */
                if (sum != sum || serial_sum != serial_sum)
                    fprintf(stderr, "Warning: non-finite checksum\n");
            }
        }
        free(input); free(serial_out); free(out);
    }
    return 0;
}
