#include "common.h"

typedef struct {
    int value;
    long index;
} Result;

static Result serial_max(const int *a, long n)
{
    Result result = {a[0], 0};

    for (long i = 1; i < n; i++) {
        if (a[i] > result.value) {
            result.value = a[i];
            result.index = i;
        }
    }

    return result;
}

static Result serial_min(const int *a, long n)
{
    Result result = {a[0], 0};

    for (long i = 1; i < n; i++) {
        if (a[i] < result.value) {
            result.value = a[i];
            result.index = i;
        }
    }

    return result;
}

/* Critical-section version retained for comparison. */
static Result parallel_idx(const int *a, long n, int is_max)
{
    Result best = {a[0], 0};

    #pragma omp parallel for schedule(static)
    for (long i = 1; i < n; i++) {
        #pragma omp critical(best_update)
        {
            if ((is_max && a[i] > best.value) ||
                (!is_max && a[i] < best.value)) {
                best.value = a[i];
                best.index = i;
            }
        }
    }

    return best;
}

/* Each thread computes a local result, then the results are combined. */
static Result manual_idx(const int *a, long n, int is_max, int thread_count)
{
    Result *thread_results =
        xmalloc((size_t)thread_count * sizeof(*thread_results));

    for (int t = 0; t < thread_count; t++) {
        thread_results[t].value = is_max ? INT_MIN : INT_MAX;
        thread_results[t].index = -1;
    }

    #pragma omp parallel num_threads(thread_count)
    {
        int tid = omp_get_thread_num();
        Result local = {is_max ? INT_MIN : INT_MAX, -1};

        #pragma omp for schedule(static)
        for (long i = 0; i < n; i++) {
            if ((is_max && a[i] > local.value) ||
                (!is_max && a[i] < local.value)) {
                local.value = a[i];
                local.index = i;
            }
        }

        thread_results[tid] = local;
    }

    Result best = thread_results[0];

    for (int t = 1; t < thread_count; t++) {
        if ((is_max && thread_results[t].value > best.value) ||
            (!is_max && thread_results[t].value < best.value)) {
            best = thread_results[t];
        }
    }

    free(thread_results);
    return best;
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
            a[i] = (int)((i * 48271u + 17u) % 1000003u) - 500001;
        }

        /* Inject known extrema to make the expected result deterministic. */
        a[n / 3] = 900000;
        a[(n / 3) * 2] = -900000;

        Result max_result = serial_max(a, n);
        Result min_result = serial_min(a, n);

        volatile int sink = 0;
        double serial_start = omp_get_wtime();

        for (int rep = 0; rep < 3; rep++) {
            sink ^= serial_max(a, n).value;
            sink ^= serial_min(a, n).value;
        }

        double baseline = (omp_get_wtime() - serial_start) / 3.0;
        (void)sink;

        for (int ti = 0; ti < 5; ti++) {
            int nt = threads[ti];

            omp_set_dynamic(0);
            omp_set_num_threads(nt);

            double parallel_start = omp_get_wtime();
            Result parallel_max = manual_idx(a, n, 1, nt);
            Result parallel_min = manual_idx(a, n, 0, nt);
            double parallel_end = omp_get_wtime();

            int correct = parallel_max.value == max_result.value &&
                          parallel_min.value == min_result.value;

            print_row("minmax", "manual_pair", n, nt,
                      parallel_end - parallel_start, baseline, correct);
        }

        omp_set_dynamic(0);
        omp_set_num_threads(8);

        double critical_start = omp_get_wtime();
        Result critical_max = parallel_idx(a, n, 1);
        Result critical_min = parallel_idx(a, n, 0);
        double critical_end = omp_get_wtime();

        int critical_correct = critical_max.value == max_result.value &&
                               critical_min.value == min_result.value;

        print_row("minmax", "critical_pair", n, 8,
                  critical_end - critical_start, baseline, critical_correct);

        printf("# extrema,N=%ld,max=%d,max_index=%ld,min=%d,min_index=%ld\n",
               n, max_result.value, max_result.index,
               min_result.value, min_result.index);

        free(a);
    }

    return EXIT_SUCCESS;
}
