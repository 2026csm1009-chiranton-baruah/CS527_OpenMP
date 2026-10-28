/*
 * CS527 Lab 9 - Task 1: Serial and OpenMP task-based merge sort
 * Compile: gcc -O2 -Wall -Wextra -fopenmp task1_merge_sort.c -o task1
 * Run:     ./task1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define CUTOFF 2048

static void merge(int *a, int *tmp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= right) tmp[k++] = a[j++];
    for (i = left; i <= right; ++i) a[i] = tmp[i];
}

static void merge_sort_serial(int *a, int *tmp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort_serial(a, tmp, left, mid);
    merge_sort_serial(a, tmp, mid + 1, right);
    merge(a, tmp, left, mid, right);
}

static void merge_sort_parallel(int *a, int *tmp, int left, int right) {
    if (left >= right) return;
    if (right - left < CUTOFF) {
        merge_sort_serial(a, tmp, left, right);
        return;
    }
    int mid = left + (right - left) / 2;
    #pragma omp task shared(a, tmp) firstprivate(left, mid)
    merge_sort_parallel(a, tmp, left, mid);

    #pragma omp task shared(a, tmp) firstprivate(mid, right)
    merge_sort_parallel(a, tmp, mid + 1, right);

    #pragma omp taskwait
    merge(a, tmp, left, mid, right);
}

static int is_sorted(const int *a, long n) {
    for (long i = 1; i < n; ++i)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    const long sizes[] = {100000L, 1000000L, 10000000L};
    const int thread_counts[] = {1, 2, 4, 8};
    puts("N,threads,serial_seconds,parallel_seconds,speedup,efficiency,correct");
    for (size_t si = 0; si < sizeof(sizes)/sizeof(sizes[0]); ++si) {
        long n = sizes[si];
        int *original = malloc((size_t)n * sizeof(*original));
        int *serial = malloc((size_t)n * sizeof(*serial));
        int *parallel = malloc((size_t)n * sizeof(*parallel));
        int *tmp_s = malloc((size_t)n * sizeof(*tmp_s));
        int *tmp_p = malloc((size_t)n * sizeof(*tmp_p));
        if (!original || !serial || !parallel || !tmp_s || !tmp_p) {
            fprintf(stderr, "Allocation failed for N=%ld\n", n);
            free(original); free(serial); free(parallel); free(tmp_s); free(tmp_p);
            return EXIT_FAILURE;
        }

        unsigned int seed = 527;
        for (long i = 0; i < n; ++i) {
            seed = 1664525u * seed + 1013904223u;
            original[i] = (int)(seed & 0x7fffffff);
        }

        memcpy(serial, original, (size_t)n * sizeof(*serial));
        double start = omp_get_wtime();
        merge_sort_serial(serial, tmp_s, 0, (int)n - 1);
        double serial_time = omp_get_wtime() - start;

        for (size_t ti = 0; ti < sizeof(thread_counts)/sizeof(thread_counts[0]); ++ti) {
            int threads = thread_counts[ti];
            memcpy(parallel, original, (size_t)n * sizeof(*parallel));
            omp_set_num_threads(threads);
            start = omp_get_wtime();
            #pragma omp parallel
            {
                #pragma omp single
                merge_sort_parallel(parallel, tmp_p, 0, (int)n - 1);
            }
            double parallel_time = omp_get_wtime() - start;
            int correct = is_sorted(parallel, n) &&
                          memcmp(parallel, serial, (size_t)n * sizeof(*serial)) == 0;
            double speedup = parallel_time > 0.0 ? serial_time / parallel_time : 0.0;
            double efficiency = speedup / threads;
            printf("%ld,%d,%.6f,%.6f,%.4f,%.4f,%s\n",
                   n, threads, serial_time, parallel_time, speedup, efficiency,
                   correct ? "yes" : "NO");
        }
        free(original); free(serial); free(parallel); free(tmp_s); free(tmp_p);
    }
    return 0;
}
