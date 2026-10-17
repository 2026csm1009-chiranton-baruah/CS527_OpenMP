#!/usr/bin/env bash
# Correctness + repeatability + thread-count test suite for CS527 Lab 8.
# Run from the directory containing this script: ./run_tests.sh
set -Eeuo pipefail

THREAD_COUNTS="${THREAD_COUNTS:-1 2 4}"
REPEATS="${REPEATS:-3}"
RESULTS_DIR="${RESULTS_DIR:-results}"
mkdir -p "$RESULTS_DIR"
STAMP="$(date +%Y%m%d_%H%M%S)"
LOG="$RESULTS_DIR/test_${STAMP}.log"
CSV="$RESULTS_DIR/test_${STAMP}.csv"

exec > >(tee "$LOG") 2>&1

pass_count=0
fail_count=0

pass() { printf 'PASS: %s\n' "$1"; pass_count=$((pass_count + 1)); }
fail() { printf 'FAIL: %s\n' "$1"; fail_count=$((fail_count + 1)); }

check_contains() {
    local output="$1" pattern="$2" label="$3"
    if grep -Eq "$pattern" <<< "$output"; then pass "$label"; else fail "$label (expected pattern: $pattern)"; fi
}

# A clean build makes sure the source files, flags and Makefile are consistent.
echo "CS527 Lab 8 test suite — $STAMP"
echo "Thread counts: $THREAD_COUNTS; repetitions per task/thread count: $REPEATS"
echo "Compiler: ${CC:-gcc}"
if ! make clean all; then
    echo "Build failed; cannot continue testing."
    exit 2
fi
pass "clean build of all five programs"

printf 'task,threads,repeat,seconds,status\n' > "$CSV"

for threads in $THREAD_COUNTS; do
    if ! [[ "$threads" =~ ^[1-9][0-9]*$ ]]; then
        echo "Invalid thread count '$threads' (must be a positive integer)."; exit 2
    fi
    export OMP_NUM_THREADS="$threads"
    export OMP_DYNAMIC=FALSE
    export OMP_PROC_BIND=FALSE
    echo
    echo "===== THREAD COUNT: $threads ====="

    for repeat in $(seq 1 "$REPEATS"); do
        echo "--- repetition $repeat/$REPEATS ---"
        for task in task1 task2 task3 task4 task5; do
            tmp_out="$(mktemp)"
            if ./$task >"$tmp_out" 2>&1; then
                output="$(cat "$tmp_out")"
                case "$task" in
                    task1)
                        ok=1
                        grep -q 'Found: yes' "$tmp_out" || ok=0
                        grep -q 'Index: 777777' "$tmp_out" || ok=0
                        time_line="$(grep -E 'Parallel search time:' "$tmp_out" | tail -1 || true)"
                        ;;
                    task2)
                        ok=1
                        grep -q 'Correctness: PASS' "$tmp_out" || ok=0
                        grep -q 'Sorted array: 1 2 3 4 7 8 12 17 18 25 29 33' "$tmp_out" || ok=0
                        time_line="$(grep -E 'Parallel sort time:' "$tmp_out" | tail -1 || true)"
                        ;;
                    task3)
                        ok=1
                        grep -q '^Total counted: 1000000 (expected 1000000)$' "$tmp_out" || ok=0
                        for bin in $(seq 0 9); do grep -q "^${bin}: 100000$" "$tmp_out" || ok=0; done
                        time_line="$(grep -E 'Parallel histogram time:' "$tmp_out" | tail -1 || true)"
                        ;;
                    task4)
                        ok=1
                        grep -q 'Correctness: PASS' "$tmp_out" || ok=0
                        grep -q 'Transpose (5 x 4):' "$tmp_out" || ok=0
                        time_line="$(grep -E 'Parallel transpose time:' "$tmp_out" | tail -1 || true)"
                        ;;
                    task5)
                        ok=1
                        grep -q 'First output expected 70.00: PASS' "$tmp_out" || ok=0
                        # For the supplied linear 5x5 image, all valid 3x3 averages are known.
                        grep -Eq '70\.00 +80\.00 +90\.00' "$tmp_out" || ok=0
                        grep -Eq '120\.00 +130\.00 +140\.00' "$tmp_out" || ok=0
                        grep -Eq '170\.00 +180\.00 +190\.00' "$tmp_out" || ok=0
                        time_line="$(grep -E 'Parallel convolution time:' "$tmp_out" | tail -1 || true)"
                        ;;
                esac
                if [[ "$ok" -eq 1 ]]; then
                    pass "$task correctness (threads=$threads, repeat=$repeat)"
                    seconds="$(sed -nE 's/.*time: ([0-9]+\.[0-9]+) s.*/\1/p' <<< "$time_line")"
                    [[ -n "$seconds" ]] || seconds="NA"
                    printf '%s,%s,%s,%s,PASS\n' "$task" "$threads" "$repeat" "$seconds" >> "$CSV"
                else
                    fail "$task output validation (threads=$threads, repeat=$repeat)"
                    echo "Output was:"; cat "$tmp_out"
                    printf '%s,%s,%s,NA,FAIL\n' "$task" "$threads" "$repeat" >> "$CSV"
                fi
            else
                fail "$task exited non-zero (threads=$threads, repeat=$repeat)"
                cat "$tmp_out"
                printf '%s,%s,%s,NA,FAIL\n' "$task" "$threads" "$repeat" >> "$CSV"
            fi
            rm -f "$tmp_out"
        done
    done
done

echo
echo "===== SUMMARY ====="
echo "Passed checks: $pass_count"
echo "Failed checks: $fail_count"
echo "Detailed log: $LOG"
echo "Timing CSV:   $CSV"
echo "Note: timings from the small demo sort/matrix inputs are correctness diagnostics, not meaningful speedup evidence."
if (( fail_count > 0 )); then exit 1; fi
exit 0
