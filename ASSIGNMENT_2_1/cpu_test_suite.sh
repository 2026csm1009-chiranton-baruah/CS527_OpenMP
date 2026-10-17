#!/usr/bin/env bash
# CPU characterization suite for the CS527 OpenMP lab. Run from this directory.
set -Eeuo pipefail

REPEATS="${REPEATS:-3}"
RESULTS_DIR="${RESULTS_DIR:-results}"
# Sizes: compute/memory/histogram are element counts; transpose/convolution use N x N.
COMPUTE_N="${COMPUTE_N:-8000000}"
MEMORY_N="${MEMORY_N:-12000000}"
HISTOGRAM_N="${HISTOGRAM_N:-12000000}"
TRANSPOSE_N="${TRANSPOSE_N:-1800}"
CONVOLUTION_N="${CONVOLUTION_N:-1200}"

mkdir -p "$RESULTS_DIR"
STAMP="$(date +%Y%m%d_%H%M%S)"
CSV="$RESULTS_DIR/cpu_benchmark_${STAMP}.csv"
META="$RESULTS_DIR/cpu_info_${STAMP}.txt"

{
  echo "CS527 OpenMP CPU characterization"
  echo "Timestamp: $(date -Is)"
  echo "Hostname: $(hostname)"
  echo "OS: $(uname -a)"
  echo "Logical CPUs reported by getconf: $(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)"
  if command -v lscpu >/dev/null 2>&1; then lscpu; fi
  echo
  echo "Compiler: $(gcc --version | head -n 1)"
  echo "OpenMP settings: OMP_DYNAMIC=FALSE; OMP_PROC_BIND=TRUE; OMP_PLACES=cores"
  echo "Repeat count: $REPEATS"
  echo "Workload sizes: compute=$COMPUTE_N memory=$MEMORY_N histogram=$HISTOGRAM_N transpose=${TRANSPOSE_N}x${TRANSPOSE_N} convolution=${CONVOLUTION_N}x${CONVOLUTION_N}"
} > "$META"

make cpu_benchmark
LOGICAL="$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)"
# Test powers-of-two thread counts and all logical CPUs; remove duplicates and cap to available CPUs.
thread_list="1"
t=2
while (( t < LOGICAL )); do thread_list+=" $t"; t=$((t * 2)); done
thread_list+=" $LOGICAL"
# Honor CPU affinity if taskset is available and reports a smaller allowed set.
if command -v nproc >/dev/null 2>&1; then ALLOWED="$(nproc)"; else ALLOWED="$LOGICAL"; fi
if (( ALLOWED > 0 && ALLOWED < LOGICAL )); then LOGICAL="$ALLOWED"; fi
# Rebuild list using the allowed logical CPU count.
thread_list="1"; t=2
while (( t < LOGICAL )); do thread_list+=" $t"; t=$((t * 2)); done
thread_list+=" $LOGICAL"
read -r -a THREADS <<< "$(tr ' ' '\n' <<< "$thread_list" | awk '!seen[$0]++' | tr '\n' ' ')"

printf 'workload,size,threads,serial_seconds,parallel_seconds,speedup,efficiency,status\n' > "$CSV"
export OMP_DYNAMIC=FALSE OMP_PROC_BIND=TRUE OMP_PLACES=cores

run_workload() {
  local workload="$1" size="$2" threads="$3" rep output
  for ((rep=1; rep<=REPEATS; rep++)); do
    echo "Running $workload size=$size threads=$threads repetition=$rep/$REPEATS"
    output="$(./cpu_benchmark "$workload" "$size" "$threads")"
    printf '%s\n' "$output" >> "$CSV"
  done
}

for threads in "${THREADS[@]}"; do
  run_workload compute "$COMPUTE_N" "$threads"
  run_workload memory "$MEMORY_N" "$threads"
  run_workload histogram "$HISTOGRAM_N" "$threads"
  run_workload transpose "$TRANSPOSE_N" "$threads"
  run_workload convolution "$CONVOLUTION_N" "$threads"
done

echo
echo "CPU metadata: $META"
echo "Raw results CSV: $CSV"
python3 summarize_cpu_results.py "$CSV"
echo "Thread counts tested: ${THREADS[*]}"
echo "Each CSV row contains serial time, parallel time, speedup and efficiency. Compare repeated runs; do not treat tiny timing differences as meaningful."
