#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_positive(const char *s, long *out) {
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || end == s || *end != '\0' || v < 1) return 0;
    *out = v;
    return 1;
}

static int bench_compute(long n, int threads, double *serial_s, double *parallel_s) {
    double serial = 0.0, parallel = 0.0;
    double t = omp_get_wtime();
    for (long i = 0; i < n; ++i) {
        double x = (double)(i % 1000) / 1000.0 + 0.5;
        for (int k = 0; k < 24; ++k) x = x * 1.0000001 + 0.0000003;
        serial += x;
    }
    *serial_s = omp_get_wtime() - t;
    t = omp_get_wtime();
    #pragma omp parallel for num_threads(threads) reduction(+:parallel) schedule(static)
    for (long i = 0; i < n; ++i) {
        double x = (double)(i % 1000) / 1000.0 + 0.5;
        for (int k = 0; k < 24; ++k) x = x * 1.0000001 + 0.0000003;
        parallel += x;
    }
    *parallel_s = omp_get_wtime() - t;
    return fabs(serial - parallel) <= fmax(1e-8, fabs(serial) * 1e-10);
}

static int bench_memory(long n, int threads, double *serial_s, double *parallel_s) {
    double *a = malloc((size_t)n * sizeof(*a));
    if (!a) return 0;
    for (long i = 0; i < n; ++i) a[i] = (double)(i % 1009) * 0.25;
    double serial = 0.0, parallel = 0.0;
    double t = omp_get_wtime();
    for (long i = 0; i < n; ++i) serial += a[i];
    *serial_s = omp_get_wtime() - t;
    t = omp_get_wtime();
    #pragma omp parallel for num_threads(threads) reduction(+:parallel) schedule(static)
    for (long i = 0; i < n; ++i) parallel += a[i];
    *parallel_s = omp_get_wtime() - t;
    free(a);
    return fabs(serial - parallel) <= fmax(1e-6, fabs(serial) * 1e-10);
}

static int bench_histogram(long n, int threads, double *serial_s, double *parallel_s) {
    int *a = malloc((size_t)n * sizeof(*a));
    if (!a) return 0;
    for (long i = 0; i < n; ++i) a[i] = (int)((i * 17 + i / 7) % 10);
    long long serial[10] = {0}, parallel[10] = {0};
    double t = omp_get_wtime();
    for (long i = 0; i < n; ++i) serial[a[i]]++;
    *serial_s = omp_get_wtime() - t;
    t = omp_get_wtime();
    #pragma omp parallel num_threads(threads)
    {
        long long local[10] = {0};
        #pragma omp for schedule(static)
        for (long i = 0; i < n; ++i) local[a[i]]++;
        #pragma omp critical
        for (int b = 0; b < 10; ++b) parallel[b] += local[b];
    }
    *parallel_s = omp_get_wtime() - t;
    int ok = 1;
    for (int b = 0; b < 10; ++b) if (serial[b] != parallel[b]) ok = 0;
    free(a);
    return ok;
}

static int bench_transpose(long n, int threads, double *serial_s, double *parallel_s) {
    if (n > 10000) return 0;
    size_t count = (size_t)n * (size_t)n;
    double *a = malloc(count * sizeof(*a));
    double *b = malloc(count * sizeof(*b));
    double *c = malloc(count * sizeof(*c));
    if (!a || !b || !c) { free(a); free(b); free(c); return 0; }
    for (size_t i = 0; i < count; ++i) a[i] = (double)(i % 997);
    double t = omp_get_wtime();
    for (long i = 0; i < n; ++i)
        for (long j = 0; j < n; ++j) b[(size_t)j*n+i] = a[(size_t)i*n+j];
    *serial_s = omp_get_wtime() - t;
    t = omp_get_wtime();
    #pragma omp parallel for num_threads(threads) collapse(2) schedule(static)
    for (long i = 0; i < n; ++i)
        for (long j = 0; j < n; ++j) c[(size_t)j*n+i] = a[(size_t)i*n+j];
    *parallel_s = omp_get_wtime() - t;
    int ok = 1;
    for (size_t i = 0; i < count; ++i) if (b[i] != c[i]) { ok = 0; break; }
    free(a); free(b); free(c);
    return ok;
}

static int bench_convolution(long n, int threads, double *serial_s, double *parallel_s) {
    if (n < 3 || n > 10000) return 0;
    size_t count = (size_t)n * (size_t)n;
    double *a = malloc(count * sizeof(*a));
    size_t outn = (size_t)(n - 2) * (size_t)(n - 2);
    double *b = malloc(outn * sizeof(*b));
    double *c = malloc(outn * sizeof(*c));
    if (!a || !b || !c) { free(a); free(b); free(c); return 0; }
    for (size_t i = 0; i < count; ++i) a[i] = (double)(i % 251);
    double t = omp_get_wtime();
    for (long i = 0; i < n - 2; ++i) for (long j = 0; j < n - 2; ++j) {
        double sum = 0.0;
        for (int ki = 0; ki < 3; ++ki) for (int kj = 0; kj < 3; ++kj)
            sum += a[(size_t)(i+ki)*n+(size_t)(j+kj)];
        b[(size_t)i*(n-2)+(size_t)j] = sum / 9.0;
    }
    *serial_s = omp_get_wtime() - t;
    t = omp_get_wtime();
    #pragma omp parallel for num_threads(threads) collapse(2) schedule(static)
    for (long i = 0; i < n - 2; ++i) for (long j = 0; j < n - 2; ++j) {
        double sum = 0.0;
        for (int ki = 0; ki < 3; ++ki) for (int kj = 0; kj < 3; ++kj)
            sum += a[(size_t)(i+ki)*n+(size_t)(j+kj)];
        c[(size_t)i*(n-2)+(size_t)j] = sum / 9.0;
    }
    *parallel_s = omp_get_wtime() - t;
    int ok = 1;
    for (size_t i = 0; i < outn; ++i) if (fabs(b[i]-c[i]) > 1e-12) { ok = 0; break; }
    free(a); free(b); free(c);
    return ok;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <compute|memory|histogram|transpose|convolution> <size> <threads>\n", argv[0]);
        return 2;
    }
    long n; long thread_long;
    if (!parse_positive(argv[2], &n) || !parse_positive(argv[3], &thread_long) || thread_long > 4096) {
        fprintf(stderr, "size and threads must be positive integers (threads <= 4096)\n"); return 2;
    }
    int threads = (int)thread_long;
    double serial_s = 0.0, parallel_s = 0.0;
    int ok = 0;
    if (!strcmp(argv[1], "compute")) ok = bench_compute(n, threads, &serial_s, &parallel_s);
    else if (!strcmp(argv[1], "memory")) ok = bench_memory(n, threads, &serial_s, &parallel_s);
    else if (!strcmp(argv[1], "histogram")) ok = bench_histogram(n, threads, &serial_s, &parallel_s);
    else if (!strcmp(argv[1], "transpose")) ok = bench_transpose(n, threads, &serial_s, &parallel_s);
    else if (!strcmp(argv[1], "convolution")) ok = bench_convolution(n, threads, &serial_s, &parallel_s);
    else { fprintf(stderr, "Unknown workload: %s\n", argv[1]); return 2; }
    if (!ok) { fprintf(stderr, "ERROR: allocation, workload limit, or correctness comparison failed\n"); return 1; }
    double speedup = parallel_s > 0.0 ? serial_s / parallel_s : 0.0;
    double efficiency = speedup / (double)threads;
    printf("%s,%ld,%d,%.9f,%.9f,%.6f,%.6f,PASS\n", argv[1], n, threads,
           serial_s, parallel_s, speedup, efficiency);
    return 0;
}
