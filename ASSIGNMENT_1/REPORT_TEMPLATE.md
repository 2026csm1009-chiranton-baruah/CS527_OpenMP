# CS527 OpenMP Lab Report — Template

**Environment:** Windows + WSL, GCC version: ______, CPU: Intel Core i5-1235U, RAM: ______, OS/WSL version: ______  
**Thread control:** `omp_set_num_threads()` in the programs; default benchmark thread counts 1, 2, 4, 8, and optional 12.  
**Compiler flags:** `-O2 -Wall -Wextra -std=c11 -fopenmp`

> Replace blanks and tables with your actual output. Do not report sample or invented timings as measurements.

## Method

Each task has a serial reference and one or more OpenMP implementations. Timing uses `omp_get_wtime()`. The code verifies parallel results against serial results before reporting correctness. Speedup is `T_serial / T_parallel`; parallel efficiency is `speedup / number_of_threads`. Allocation, initialization, and correctness scans are outside the timed kernel; parallel-region overhead is included.

## Task 1 — Vector addition

Test `N = 10^5, 10^6, 10^7, 10^8` and thread counts 1, 2, 4, 8 (12 is an optional CPU-specific extension). Attach `task1.csv`.

**Questions**

- **When does parallelization pay off?** Determine this from the measured crossover point. Small inputs may not amortize OpenMP team/dispatch overhead. Vector addition performs little arithmetic per byte moved, so memory bandwidth can become the limiting factor.
- **Does speedup scale linearly?** Usually not indefinitely. Threads share memory bandwidth; scheduling, thread startup, cache behavior, and the hybrid CPU's P/E cores can limit scaling.

## Task 2 — Sum of array elements

Test `N = 10^7, 10^8` and thread counts 1, 2, 4, 8. Attach `task2.csv`.

**Questions**

- **Why is naive `sum += a[i]` unsafe?** Multiple threads read, modify, and write the same shared `sum` without synchronization. Updates can be lost; this is a data race and the program's behavior is undefined. It may appear correct in some runs, so repeated failures are not guaranteed.
- **Would `critical` fix it?** Yes, a critical section serializes each update and protects correctness. It will generally be much slower than `reduction`, because every element contends for one lock. Reduction gives each thread a private partial sum and combines partial sums after the loop.

## Task 3 — Maximum/minimum and index

Test `N = 10^7, 10^8`. Attach `task3.csv`.

**Questions**

- **Why not `reduction(max:best_idx)`?** The largest index is not necessarily the index of the largest value. The value and its associated index must be reduced as a pair; a scalar max reduction on the index loses that relationship.
- **Critical vs manual per-thread reduction?** The critical approach can contend when threads repeatedly try to update the shared pair. A manual reduction finds one local pair per thread and combines only those thread-level results, greatly reducing synchronization. Compare actual timings; for small cases overhead may dominate.

## Task 4 — Matrix addition

Test the small outer-loop case `N = 8` and the large case `N = 4096`, using thread counts 1, 2, 4, 8. Attach `task4.csv`.

**Questions**

- **Does `collapse(2)` help at small N?** With only 8 outer iterations, outer-loop-only parallelism exposes at most 8 row iterations and each thread receives a row. `collapse(2)` exposes 64 element iterations, giving the runtime finer-grained work distribution. Overhead can still outweigh the benefit for such a tiny matrix.
- **Does it matter at large N?** At N=4096, the outer loop already exposes 4096 independent rows, usually enough to keep 8 or 12 threads occupied. `collapse(2)` may offer little benefit and can have similar or higher scheduling overhead. Base the conclusion on measured data.

## Task 5 — Naive matrix multiplication

Test `N = 256, 512, 1024`, thread counts 1, 2, 4, 8. Attach `task5.csv`.

**Questions**

- **Why is the i loop safe to parallelize?** Each `i` iteration writes a distinct row of C. The `j` and `k` loops for a given row are executed by one thread, so each `C[i][j]` accumulation has a single writer.
- **What if k is parallelized with plain `parallel for`?** Multiple threads would update the same `C[i][j]` concurrently, causing a data race and lost contributions. A reduction or explicit partial sums would be required to parallelize the dot-product dimension safely.
- **Outer i vs `collapse(2)`?** Outer-loop parallelism exposes N tasks. Collapsing i and j exposes N² output elements. It matters most when N is small relative to the thread count; at larger N, the i loop often provides enough parallel work already.
- **Why compute a local `sum`?** It keeps each output's accumulation local and writes `C[i][j]` once, reducing repeated memory traffic and making the ownership of each output clear.

## Results and conclusion

Insert actual timings, speedup and efficiency tables/plots. Discuss (1) correctness, (2) small-input overhead, (3) memory bandwidth for array/matrix addition, (4) synchronization costs for reductions, (5) how well the hybrid i5-1235U scales from 1 to 12 threads, and (6) differences between `parallel for` and `collapse(2)`. Do not claim that the fastest result is universal; it depends on input size, compiler, runtime, and machine conditions.
