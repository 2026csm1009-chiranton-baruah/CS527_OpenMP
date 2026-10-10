#include "common.h"

static void serial(const double *a, const double *b, double *c, long n)
{
    memset(c, 0, (size_t)n * (size_t)n * sizeof(*c));

    for (long i = 0; i < n; i++) {
        for (long j = 0; j < n; j++) {
            double sum = 0.0;

            for (long k = 0; k < n; k++) {
                sum += a[i * n + k] * b[k * n + j];
            }

            c[i * n + j] = sum;
        }
    }
}

static void outer(const double *a, const double *b, double *c, long n)
{
    #pragma omp parallel for schedule(static)
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < n; j++) {
            double sum = 0.0;

            for (long k = 0; k < n; k++) {
                sum += a[i * n + k] * b[k * n + j];
            }

            c[i * n + j] = sum;
        }
    }
}

static void collapse2(const double *a, const double *b, double *c, long n)
{
    #pragma omp parallel for collapse(2) schedule(static)
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < n; j++) {
            double sum = 0.0;

            for (long k = 0; k < n; k++) {
                sum += a[i * n + k] * b[k * n + j];
            }

            c[i * n + j] = sum;
        }
    }
}

static int equal(const double *a, const double *b, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        if (!nearly_equal(a[i], b[i])) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    long dimensions[] = {256, 512, 1024};
    int threads[] = {1, 2, 4, 8, 12};

    if (argc > 1) {
        dimensions[0] = parse_long(argv[1], "dimension");
        dimensions[1] = dimensions[0];
        dimensions[2] = dimensions[0];
    }

    if (argc > 2) {
        int count = (int)parse_long(argv[2], "threads");
        for (int i = 0; i < 5; i++) {
            threads[i] = count;
        }
    }

    print_header();

    for (int d = 0; d < 3; d++) {
        long n = dimensions[d];

        if (d > 0 && n == dimensions[d - 1]) {
            continue;
        }

        if ((uint64_t)n * (uint64_t)n > SIZE_MAX / sizeof(double)) {
            fprintf(stderr, "Matrix too large\n");
            return EXIT_FAILURE;
        }

        size_t count = (size_t)n * (size_t)n;
        double *a = xmalloc(count * sizeof(*a));
        double *b = xmalloc(count * sizeof(*b));
        double *serial_result = xmalloc(count * sizeof(*serial_result));
        double *parallel_result = xmalloc(count * sizeof(*parallel_result));

        for (long i = 0; i < n; i++) {
            for (long j = 0; j < n; j++) {
                a[i * n + j] = (double)((i * 3 + j * 7) % 17) / 17.0;
                b[i * n + j] = (double)((i * 11 + j * 5) % 19) / 19.0;
            }
        }

        double serial_start = omp_get_wtime();
        serial(a, b, serial_result, n);
        double serial_end = omp_get_wtime();
        double baseline = serial_end - serial_start;

        print_row("matmul", "serial", n, 1, baseline, baseline, 1);

        for (int ti = 0; ti < 5; ti++) {
            int nt = threads[ti];

            omp_set_dynamic(0);
            omp_set_num_threads(nt);

            double parallel_start = omp_get_wtime();
            outer(a, b, parallel_result, n);
            double parallel_end = omp_get_wtime();

            print_row("matmul", "outer", n, nt,
                      parallel_end - parallel_start, baseline,
                      equal(serial_result, parallel_result, count));

            parallel_start = omp_get_wtime();
            collapse2(a, b, parallel_result, n);
            parallel_end = omp_get_wtime();

            print_row("matmul", "collapse2", n, nt,
                      parallel_end - parallel_start, baseline,
                      equal(serial_result, parallel_result, count));
        }

        free(a);
        free(b);
        free(serial_result);
        free(parallel_result);
    }

    return EXIT_SUCCESS;
}
