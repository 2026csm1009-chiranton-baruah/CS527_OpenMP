#include "common.h"

static void serial(const double *a, const double *b, double *c,
                   long rows, long cols)
{
    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            c[i * cols + j] = a[i * cols + j] + b[i * cols + j];
        }
    }
}

static void outer(const double *a, const double *b, double *c,
                  long rows, long cols)
{
    #pragma omp parallel for schedule(static)
    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            c[i * cols + j] = a[i * cols + j] + b[i * cols + j];
        }
    }
}

static void collapse2(const double *a, const double *b, double *c,
                      long rows, long cols)
{
    #pragma omp parallel for collapse(2) schedule(static)
    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            c[i * cols + j] = a[i * cols + j] + b[i * cols + j];
        }
    }
}

int main(int argc, char **argv)
{
    long dimensions[] = {8, 4096};
    int threads[] = {1, 2, 4, 8, 12};

    if (argc > 1) {
        dimensions[0] = parse_long(argv[1], "dimension");
        dimensions[1] = dimensions[0];
    }

    if (argc > 2) {
        int count = (int)parse_long(argv[2], "threads");
        for (int i = 0; i < 5; i++) {
            threads[i] = count;
        }
    }

    print_header();

    for (int d = 0; d < 2; d++) {
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

        for (size_t q = 0; q < count; q++) {
            a[q] = (double)(q % 31);
            b[q] = (double)(q % 13);
        }

        double serial_start = omp_get_wtime();
        serial(a, b, serial_result, n, n);
        double serial_end = omp_get_wtime();
        double baseline = serial_end - serial_start;

        for (int ti = 0; ti < 5; ti++) {
            int nt = threads[ti];

            omp_set_dynamic(0);
            omp_set_num_threads(nt);

            double parallel_start = omp_get_wtime();
            outer(a, b, parallel_result, n, n);
            double parallel_end = omp_get_wtime();

            int correct = 1;
            for (size_t q = 0; q < count; q++) {
                if (parallel_result[q] != serial_result[q]) {
                    correct = 0;
                    break;
                }
            }

            print_row("matadd", "outer", n, nt,
                      parallel_end - parallel_start, baseline, correct);

            parallel_start = omp_get_wtime();
            collapse2(a, b, parallel_result, n, n);
            parallel_end = omp_get_wtime();

            correct = 1;
            for (size_t q = 0; q < count; q++) {
                if (parallel_result[q] != serial_result[q]) {
                    correct = 0;
                    break;
                }
            }

            print_row("matadd", "collapse2", n, nt,
                      parallel_end - parallel_start, baseline, correct);
        }

        free(a);
        free(b);
        free(serial_result);
        free(parallel_result);
    }

    return EXIT_SUCCESS;
}
