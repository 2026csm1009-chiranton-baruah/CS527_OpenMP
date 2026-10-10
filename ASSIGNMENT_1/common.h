#ifndef COMMON_H
#define COMMON_H

#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *xmalloc(size_t bytes)
{
    void *ptr = malloc(bytes);

    if (ptr == NULL && bytes != 0) {
        fprintf(stderr, "Allocation failed for %zu bytes.\n", bytes);
        exit(EXIT_FAILURE);
    }

    return ptr;
}

static inline long parse_long(const char *text, const char *description)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (text[0] == '\0' || end == NULL || *end != '\0' || value <= 0) {
        fprintf(stderr, "Invalid %s: %s\n", description, text);
        exit(EXIT_FAILURE);
    }

    return value;
}

static inline int nearly_equal(double a, double b)
{
    return fabs(a - b) <=
           1e-10 * fmax(1.0, fmax(fabs(a), fabs(b)));
}

static inline void print_header(void)
{
    puts("task,variant,n,threads,seconds,speedup,efficiency,correct");
}

static inline void print_row(const char *task, const char *variant,
                             long n, int threads, double seconds,
                             double baseline, int correct)
{
    double speedup = seconds > 0.0 ? baseline / seconds : 0.0;

    printf("%s,%s,%ld,%d,%.9f,%.6f,%.6f,%s\n",
           task, variant, n, threads, seconds, speedup,
           speedup / threads, correct ? "yes" : "NO");
}

#endif
