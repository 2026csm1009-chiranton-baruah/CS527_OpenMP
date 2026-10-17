# CS527 Lab 8 — OpenMP: Arrays and Matrices

## Source programs
1. `task1_array_search.c` — parallel search; a reduction determines whether a match exists, while a critical section safely records the lowest matching index.
2. `task2_array_sort.c` — odd-even transposition sort. Each phase compares disjoint adjacent pairs; the implicit `omp for` barrier is needed before the next phase.
3. `task3_histogram.c` — 10-bin histogram with an OpenMP array-section reduction (requires compatible OpenMP 4.5+ support).
4. `task4_matrix_transpose.c` — 4x5 matrix transposed to 5x4 with `collapse(2)`.
5. `task5_convolution.c` — 5x5 image with the handout's 3x3 all-ones averaging kernel; valid/no-padding output is 3x3.

## Build and run
```bash
make             # compile all five programs
make task3       # compile only task 3
make run         # compile all, then run each program once
make clean       # remove compiled executables
```

Override the compiler or flags if needed, for example `make CC=gcc`.

## Detailed correctness test suite
```bash
make test
# or: ./run_tests.sh
```

The suite performs a clean build with `-Wall -Wextra`, then checks all five programs under 1, 2, and 4 OpenMP threads, repeating each task three times per thread count. It verifies expected search index, exact sorted order, all ten histogram bins and total count, transpose correctness marker/dimensions, and all nine expected convolution output values. It returns a non-zero exit status if any check fails.

Configure the scenario matrix with environment variables:
```bash
THREAD_COUNTS="1 2 4 8" REPEATS=5 ./run_tests.sh
```
`THREAD_COUNTS` is a space-separated list of positive integers; `REPEATS` is the number of repetitions per task/thread-count pair. Results are saved under `results/`: a timestamped human-readable `.log` and a `.csv` with task, thread count, repetition, kernel time, and pass/fail status. The test suite creates this directory if it does not exist.

**Scope limitation:** the current programs use fixed demonstration inputs, so this suite tests correctness and repeatability across thread counts, not arbitrary input edge cases. It cannot test missing targets, empty arrays, alternate image sizes, etc. until the programs accept configurable inputs. Timing from tiny sort/matrix examples is diagnostic only, not credible speedup evidence.

## Benchmark script
`benchmark.sh` remains available for the original simple timing run at 1, 2, and 4 threads. Run it with `./benchmark.sh`. The detailed `run_tests.sh` suite is intended for correctness regression checks; use scaled-up inputs and serial baselines for formal speedup/efficiency reporting.

## Correctness expectations
- Search: target `777777` is at index 777777 in the generated array.
- Sort: output is `1 2 3 4 7 8 12 17 18 25 29 33`.
- Histogram: input cycles evenly through 0..9, so each bin should equal 100000 and total count 1000000.
- Transpose: every output element is checked against its source element.
- Convolution: the 3x3 output should be rows `70 80 90`, `120 130 140`, and `170 180 190`.

## Timing, speedup and efficiency
For each task, record a serial baseline time `T1` and parallel time `Tp` at p threads using the same input and equivalent algorithm. Calculate:
- Speedup: `S(p) = T1 / Tp`
- Parallel efficiency: `E(p) = S(p) / p`

The supplied demo inputs for sorting, transpose and convolution are intentionally small. Scale up these inputs, repeat runs, and report a median or average. Do not invent timing numbers; use measurements from the target machine.

## Compiler compatibility
`reduction(+:histogram[:10])` uses OpenMP array-section reduction support. If the compiler/runtime does not support it, replace it with per-thread private histograms and combine them after the parallel region, or use `#pragma omp atomic update` for each bin increment (simpler but potentially slower).


## CPU-focused performance characterization

The original five lab programs use small fixed demonstration inputs, so their runtime is primarily useful for correctness checks—not for measuring CPU throughput. The additional `cpu_benchmark.c` driver runs larger serial and OpenMP workloads and checks that the results match:

- `compute`: arithmetic-heavy loop (compute-bound behavior)
- `memory`: large vector reduction (memory bandwidth / reduction overhead)
- `histogram`: parallel per-thread bins and merge (histogram workload)
- `transpose`: square matrix transpose (memory access/cache behavior)
- `convolution`: 3x3 averaging over a larger image (mixed compute and memory work)

Run `./cpu_test_suite.sh` or `make cpu-test`. The script records CPU/OS/compiler information, detects the available logical CPU count, tests thread counts 1, 2, 4, ... and the available logical CPU count, repeats each workload, and writes raw CSV, CPU metadata, and a median summary CSV under `results/`. You can customize workload sizes and repeats, for example:

```bash
REPEATS=5 TRANSPOSE_N=2400 CONVOLUTION_N=1600 ./cpu_test_suite.sh
```

The vector sizes are element counts; transpose and convolution sizes are square matrix dimensions. Larger sizes use more RAM. Start with defaults, close other heavy applications, keep the laptop plugged in, use a stable power/performance mode, and avoid blocking the vents. Run at least twice if results look surprising. The first run can be affected by CPU boost, temperature, background work and power limits.

The output records serial time, parallel time, speedup (`T_serial / T_parallel`) and efficiency (`speedup / thread_count`). These are workload-specific results, not a universal CPU score. More threads can make small or memory-bound workloads slower; that is useful evidence, not necessarily a bug. The script detects the CPU model with `lscpu` when available, but does not assume a particular Inspiron 15 3310 processor configuration.
