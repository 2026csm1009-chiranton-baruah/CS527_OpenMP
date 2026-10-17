# CS527 Lab 8 — OpenMP: Arrays and Matrices

## Files
1. `task1_array_search.c` — parallel search; reduction determines whether any match exists, critical section safely records the lowest matching index.
2. `task2_array_sort.c` — odd-even transposition sort. Each phase compares disjoint adjacent pairs; the implicit `omp for` barrier is needed before the next phase.
3. `task3_histogram.c` — 10-bin histogram with an OpenMP array-section reduction (requires OpenMP 4.5-compatible compiler).
4. `task4_matrix_transpose.c` — 4x5 matrix transposed to 5x4 with `collapse(2)`.
5. `task5_convolution.c` — 5x5 image with the handout's 3x3 all-ones averaging kernel; valid/no-padding output is 3x3.
6. `benchmark.sh` — compiles and runs with 1, 2, and 4 threads, saving output to `results/benchmark.txt`.

## Compile
Use GCC:
```bash
gcc -O2 -std=c11 -Wall -Wextra -fopenmp task1_array_search.c -o task1
gcc -O2 -std=c11 -Wall -Wextra -fopenmp task2_array_sort.c -o task2
gcc -O2 -std=c11 -Wall -Wextra -fopenmp task3_histogram.c -o task3
gcc -O2 -std=c11 -Wall -Wextra -fopenmp task4_matrix_transpose.c -o task4
gcc -O2 -std=c11 -Wall -Wextra -fopenmp task5_convolution.c -o task5 -lm
```
Run with a chosen thread count, for example:
```bash
OMP_NUM_THREADS=4 ./task1
```
On Linux, run all benchmarks:
```bash
chmod +x benchmark.sh
./benchmark.sh
```

## Correctness notes
- Search: target is at index 777777 in the generated array.
- Sort: correctness check verifies nondecreasing order.
- Histogram: input cycles evenly through 0..9, so each bin should equal 100000.
- Transpose: every output element is checked against its source element.
- Convolution: first output should be 70.00, as shown in the lab handout. The remaining values are also printed for inspection.

## Timing, speedup and efficiency
The program times only the parallel kernel using `omp_get_wtime()`. For each task, record the serial baseline time `T1` and parallel time `Tp` at p threads using the same input and algorithm. Calculate:
- Speedup: `S(p) = T1 / Tp`
- Parallel efficiency: `E(p) = S(p) / p`

The supplied demo inputs for sorting, transpose and convolution are intentionally small for clarity; their times are too short to make reliable performance claims. For your report, scale up the inputs (especially sort, transpose and convolution), repeat each run several times, and report a median or average. Do not invent timing numbers; fill your table with measured results from your machine.

## Potential compiler compatibility
`reduction(+:histogram[:10])` uses OpenMP array-section reduction support (OpenMP 4.5+). If your GCC/OpenMP runtime does not support it, replace with per-thread private histograms and combine them after the parallel region, or use `#pragma omp atomic update` for the bin increment (simpler but often slower).
