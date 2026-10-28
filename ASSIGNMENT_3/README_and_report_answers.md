# CS527 Lab 9 — OpenMP solution

## Build
Requires a C compiler with OpenMP support (e.g. GCC).

```sh
gcc -O2 -Wall -Wextra -fopenmp task1_merge_sort.c -o task1
gcc -O2 -Wall -Wextra -fopenmp task2_scheduling.c -o task2
gcc -O2 -Wall -Wextra -fopenmp task3_reduction.c -o task3
```

Run each program and save the CSV-style output:
```sh
./task1 > task1_results.csv
./task2 > task2_results.csv
./task3 > task3_results.csv
```

The programs benchmark the requested array sizes and thread counts. Actual timings depend on your CPU, compiler, operating system, and system load. Do not invent or copy timings from another machine; use the output from your own run.

## Metrics
For each case:
- Speedup = serial execution time / parallel execution time
- Parallel efficiency = speedup / number of threads

Efficiency can exceed 1 only if the measurement or comparison is unusual; normally efficiency is between 0 and 1 for a well-behaved strong-scaling result. A speedup below 1 means the parallel version was slower than the serial baseline.

## Task 1 report answers
1. **OpenMP tasks vs. parallel for:** `parallel for` distributes iterations of a loop among threads. Tasks represent independent units of work that may be scheduled dynamically by the OpenMP runtime; they suit recursive algorithms such as merge sort.
2. **Why `taskwait`?** The parent must ensure both child calls have finished sorting their respective halves before merging. Without the wait, merge may read unfinished or concurrently modified data.
3. **Cutoff threshold:** A larger cutoff creates fewer tasks and reduces task-management overhead, but may expose less parallelism. A smaller cutoff creates more parallelism but can spend too much time creating and scheduling tasks. The best cutoff is hardware- and workload-dependent.
4. **Why can parallel be slower for small arrays?** Thread startup/synchronization, task creation, scheduling and memory-management overhead can exceed the work saved by parallel execution.
5. **Task overhead and scalability:** Each task consumes runtime bookkeeping and scheduling resources. As task counts grow, overhead and contention can reduce speedup; enough sufficiently large tasks are needed to keep threads busy.

## Task 2 report answers
1. **Static, dynamic, guided:** Static assigns chunks before execution (low overhead); dynamic assigns another chunk when a thread finishes its current chunk (good adaptability, more scheduling overhead); guided assigns progressively smaller chunks as the remaining work decreases (a compromise between overhead and load balance).
2. **Uniform vs. uneven workloads:** Static scheduling is often best for uniform iteration costs because it has low overhead. Dynamic or guided scheduling can perform better when iteration costs vary substantially, although the winner depends on the workload and chunk size.
3. **Dynamic overhead:** Threads repeatedly request chunks from the runtime, which requires coordination and bookkeeping.
4. **Chunk size:** Small chunks improve load balancing but increase scheduling overhead. Large chunks reduce overhead but may leave threads waiting if chunks have very different costs. Guided scheduling adapts chunk sizes automatically.

## Task 3 report answers
1. **Multiple reductions:** List each reduction variable after the `+` operator, separated by commas, e.g. `reduction(+:even_count, odd_count)`. Each thread gets private partial values which OpenMP combines at the end.
2. **Reduction vs. atomic:** Reduction normally updates thread-local counters during the loop and combines them at the end. Atomic updates serialize competing writes to each shared counter, often causing contention.
3. **Why even + odd = N?** Every integer is either even or odd, and the loop classifies each array element exactly once. Thus the counts should sum to the number of elements.
4. **Do more threads always help?** No. Parallel overhead, memory bandwidth, load imbalance, synchronization, and limited physical cores can prevent improvement or make execution slower.

## Notes / limitations
- The Task 2 workload intentionally varies by index. Its loop count is `(i % 1000) * 10`, which is uneven but may still take a long time at N=10^7. If your lab machine is slow, first test smaller N and then run the full required sizes.
- Task 3's input is deterministic, so serial, reduction and atomic versions use identical data.
- The CSV outputs are measurements, not pre-filled claims. Include a brief discussion of observed trends from your own results.
