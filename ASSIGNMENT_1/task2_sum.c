#include "common.h"

static long long serial_sum(const int *a, long n)
{
    long long sum = 0;

    for (long i = 0; i < n; i++) {
        sum += a[i];
    }

    return sum;
}

/*
 * Intentionally incorrect parallel implementation:
 * multiple threads update the same variable without synchronization.
 * This data race is included for demonstration only.
 */
static long long broken_sum(const int *a, long n)
{
    long long sum = 0;

    #pragma omp parallel for schedule(static)
    for (long i = 0; i < n; i++) {
        sum += a[i];
    }

    return sum;
}

static long long reduction_sum(const int *a, long n)
{
    long long sum = 0;

    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (long i = 0; i < n; i++) {
        sum += a[i];
    }

    return sum;
}

static long long critical_sum(const int *a, long n)
{
    long long sum = 0;

    #pragma omp parallel for schedule(static)
    for (long i = 0; i < n; i++) {
        #pragma omp critical(sum_update)
        {
            sum += a[i];
        }
    }

    return sum;
}

int main(int argc, char **argv)
{
    long sizes[] = {10000000, 100000000};
    int threads[] = {1, 2, 4, 8, 12};

    if (argc > 1) {
        sizes[0] = parse_long(argv[1], "N");
        sizes[1] = sizes[0];
    }

    if (argc > 2) {
        int count = (int)parse_long(argv[2], "threads");
        for (int i = 0; i < 5; i++) {
            threads[i] = count;
        }
    }

    print_header();

    for (int si = 0; si < 2; si++) {
        long n = sizes[si];

        if (si > 0 && n == sizes[0]) {
            continue;
        }

        int *a = xmalloc((size_t)n * sizeof(*a));
        for (long i = 0; i < n; i++) {
            a[i] = (int)(i % 7) - 3;
        }

        long long reference = serial_sum(a, n);
        volatile long long sink = 0;

        double serial_start = omp_get_wtime();
        for (int rep = 0; rep < 3; rep++) {
            sink ^= serial_sum(a, n);
        }
        double serial_end = omp_get_wtime();
        double baseline = (serial_end - serial_start) / 3.0;
        (void)sink;

        for (int ti = 0; ti < 5; ti++) {
            int nt = threads[ti];

            omp_set_dynamic(0);
            omp_set_num_threads(nt);

            /* Warm-up call, not included in the recorded time. */
            long long result = reduction_sum(a, n);
            (void)result;

            double parallel_start = omp_get_wtime();
            result = reduction_sum(a, n);
            double parallel_end = omp_get_wtime();

            print_row("sum", "reduction", n, nt,
                      parallel_end - parallel_start, baseline,
                      result == reference);
        }

        omp_set_dynamic(0);
        omp_set_num_threads(8);

        for (int run = 0; run < 5; run++) {
            long long result = broken_sum(a, n);

            printf("# naive_race,N=%ld,run=%d,result=%lld,expected=%lld,correct=%s\n",
                   n, run + 1, result, reference,
                   result == reference ? "yes" : "NO");
        }

        omp_set_num_threads(8);

        double critical_start = omp_get_wtime();
        long long critical_result = critical_sum(a, n);
        double critical_end = omp_get_wtime();

        print_row("sum", "critical", n, 8,
                  critical_end - critical_start, baseline,
                  critical_result == reference);

        free(a);
    }

    return EXIT_SUCCESS;
}
