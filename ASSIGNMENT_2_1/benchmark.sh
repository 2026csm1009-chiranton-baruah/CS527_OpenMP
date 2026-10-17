#!/usr/bin/env bash
# Compile and run each task with 1, 2, and 4 OpenMP threads.
# Times from these tiny/demo inputs are illustrative; use larger input sizes
# for meaningful performance measurements and repeat runs.
set -euo pipefail
mkdir -p bin results
for src in task1_array_search task2_array_sort task3_histogram task4_matrix_transpose task5_convolution; do
  gcc -O2 -std=c11 -Wall -Wextra -fopenmp "${src}.c" -o "bin/${src}" -lm
done
for t in 1 2 4; do
  export OMP_NUM_THREADS="$t"
  echo "===== OMP_NUM_THREADS=$t =====" | tee -a results/benchmark.txt
  for exe in bin/task1_array_search bin/task2_array_sort bin/task3_histogram bin/task4_matrix_transpose bin/task5_convolution; do
    "$exe" | tee -a results/benchmark.txt
  done
done
