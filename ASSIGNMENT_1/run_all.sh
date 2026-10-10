#!/usr/bin/env bash
set -euo pipefail
make
for task in task1_vecadd task2_sum task3_minmax task4_matadd task5_matmul; do
  echo "Running $task" >&2
  ./$task > "${task}.csv"
done
