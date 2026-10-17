CC       := gcc
CFLAGS   := -O2 -std=c11 -Wall -Wextra -fopenmp
LDFLAGS  := -fopenmp
LDLIBS   := -lm

PROGRAMS := task1 task2 task3 task4 task5

.PHONY: all clean run test task1 task2 task3 task4 task5

# Default target: build all five programs.
all: $(PROGRAMS)

task1: task1_array_search.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

task2: task2_array_sort.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

task3: task3_histogram.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

task4: task4_matrix_transpose.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

task5: task5_convolution.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

# Build everything, then run each program.
test:
	./run_tests.sh

run: all
	./task1
	./task2
	./task3
	./task4
	./task5

# Remove compiled executables.
clean:
	rm -f $(PROGRAMS) cpu_benchmark

# CPU-focused benchmarks use larger, scalable workloads than the demonstration tasks.
.PHONY: cpu_benchmark cpu-test

cpu_benchmark: cpu_benchmark.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

cpu-test: cpu_benchmark
	./cpu_test_suite.sh
