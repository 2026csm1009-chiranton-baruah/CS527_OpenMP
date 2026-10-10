CC := gcc
CFLAGS := -O2 -Wall -Wextra -std=c11 -fopenmp
LDLIBS := -lm
PROGRAMS := task1_vecadd task2_sum task3_minmax task4_matadd task5_matmul
all: $(PROGRAMS)
%: %.c common.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)
clean:
	rm -f $(PROGRAMS) *.csv
.PHONY: all clean
