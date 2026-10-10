#include "common.h"

static void serial(const double *a, const double *b, double *c, long n)
{
    for (long i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

static void parallel(const double *a, const double *b, double *c, long n)
{
    #pragma omp parallel for schedule(static)
    for (long i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char **argv)
{
    long sizes[] = {100000, 1000000, 10000000, 100000000};
    int threads[] = {1, 2, 4, 8, 12};

    if (argc > 1) {
        sizes[0] = parse_long(argv[1], "N");
        sizes[1] = sizes[0];
        sizes[2] = sizes[0];
        sizes[3] = sizes[0];
    }

    if (argc > 2) {
        int count = (int)parse_long(argv[2], "threads");
        for (int i = 0; i < 5; i++) {
            threads[i] = count;
        }
    }

    print_header();

    for (int si = 0; si < 4; si++) {
        long n = sizes[si];

        if (si > 0 && sizes[si] == sizes[si - 1]) {
            continue;
        }

        if ((uint64_t)n > SIZE_MAX / (4 * sizeof(double))) {
            fprintf(stderr, "N too large\n");
            return EXIT_FAILURE;
        }

        double *a = xmalloc((size_t)n * sizeof(*a));
        double *b = xmalloc((size_t)n * sizeof(*b));
        double *serial_result = xmalloc((size_t)n * sizeof(*serial_result));
        double *parallel_result = xmalloc((size_t)n * sizeof(*parallel_result));

        for (long i = 0; i < n; i++) {
            a[i] = (double)(i % 101);
            b[i] = (double)(i % 37);
        }

        double start = omp_get_wtime();
        serial(a, b, serial_result, n);
        double end = omp_get_wtime();
        double baseline = end - start;

        for (int ti = 0; ti < 5; ti++) {
            int nt = threads[ti];

            omp_set_dynamic(0);
            omp_set_num_threads(nt);

            double parallel_start = omp_get_wtime();
            parallel(a, b, parallel_result, n);
            double parallel_end = omp_get_wtime();

            int correct = 1;
            for (long i = 0; i < n; i++) {
                if (serial_result[i] != parallel_result[i]) {
                    correct = 0;
                    break;
                }
            }

            print_row("vecadd", "parallel", n, nt,
                      parallel_end - parallel_start, baseline, correct);
        }

        print_row("vecadd", "serial", n, 1, baseline, baseline, 1);

        free(a);
        free(b);
        free(serial_result);
        free(parallel_result);
    }

    return EXIT_SUCCESS;
}
