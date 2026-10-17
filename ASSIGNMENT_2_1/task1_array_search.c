#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void) {
    const int n = 1000000;
    const int target = 777777;
    int *a = malloc((size_t)n * sizeof(*a));
    if (a == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) a[i] = i;

    int found = 0;
    int index = -1;

    double start = omp_get_wtime();
    #pragma omp parallel for reduction(|:found) 
    for (int i = 0; i < n; ++i) {
        if (a[i] == target) {
            found = 1;
            /* Multiple threads may find matches; critical section keeps
               the shared index update race-free and retains the first index. */
            #pragma omp critical
            {
                if (index == -1 || i < index) index = i;
            }
        }
    }
    double elapsed = omp_get_wtime() - start;

    printf("Target: %d\nFound: %s\nIndex: %d\n", target,
           found ? "yes" : "no", index);
    printf("Threads: %d\nParallel search time: %.6f s\n",
           omp_get_max_threads(), elapsed);

    free(a);
    return (found && index == target) ? EXIT_SUCCESS : EXIT_FAILURE;
}
