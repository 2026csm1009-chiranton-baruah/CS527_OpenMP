# CS527 OpenMP Lab — Arrays and Matrices

This package follows the five tasks in the supplied CS527 lab sheet. It uses C, GCC, OpenMP, `omp_get_wtime()`, serial reference implementations, correctness checks, and CSV-formatted benchmark rows.

## Build in WSL

```bash
sudo apt update
sudo apt install build-essential
make
```

The Makefile compiles each task with `-O2 -Wall -Wextra -std=c11 -fopenmp`.

## Run

Run each task independently. Default thread counts are `1,2,4,8,12`; the lab-required counts `1,2,4,8` are included, and 12 is an additional experiment for the i5-1235U.

```bash
./task1_vecadd > task1.csv
./task2_sum > task2.csv
./task3_minmax > task3.csv
./task4_matadd > task4.csv
./task5_matmul > task5.csv
```

To do a quick smoke test with a smaller input and a single thread count, pass `N THREADS` (for matrix tasks, N is the square dimension):

```bash
./task1_vecadd 100000 4
./task2_sum 1000000 4
./task3_minmax 1000000 4
./task4_matadd 256 4
./task5_matmul 128 4
```

When `N` is supplied, each program runs only that size. The second argument sets every tested thread count to the supplied value. Without arguments, the programs use the lab's specified problem sizes, plus 12 threads as an optional measurement. Some default cases—especially 100-million-element arrays and 1024x1024 GEMM—can take substantial memory/time. Run them separately and close memory-heavy applications first.

## Reproducibility

- Use a stable power mode, close heavy background workloads, and avoid running all five benchmarks simultaneously.
- The programs print CSV columns: task, variant, N, threads, seconds, speedup, efficiency, correctness.
- The reported speedup is serial time divided by the measured parallel time; efficiency is speedup divided by thread count.
- Task 2 additionally prints `#`-prefixed observations from intentionally racy runs. Those lines are diagnostics, not CSV records.
- Do not expect the racy sum to fail on every run: a data race is undefined behavior, so the observed outcome is not guaranteed.
- For comparisons, ensure the serial baseline and parallel run use the same input, compiler flags, machine, and power conditions. Repeat runs if measurements fluctuate.

## Notes and limitations

- Matrices are flat row-major arrays. This is explicitly stated to make storage and memory access clear.
- Task 3's manual per-thread result array merges each thread's local `(value,index)` pair in a serial pass. The critical-section variant checks each candidate while holding a critical section, matching the lab-sheet example; it is correct but may contend heavily.
- For equal extrema, the serial reference keeps the first index. The parallel versions guarantee the correct extreme value; if duplicate extrema exist, their selected index can differ. The generated test data injects unique maximum and minimum values, so index comparisons are deterministic for the included data.
- Floating-point matrix multiplication is verified with a relative/absolute tolerance, since different legal accumulation orders can produce tiny rounding differences.
- Timing includes the parallel region overhead but excludes allocation and initialization. Correctness checks are outside the timed region. Task 2 and Task 3 time their serial reference three times and use the average to reduce timer noise; other tasks use a single serial timing, so repeat the benchmark if results fluctuate.
