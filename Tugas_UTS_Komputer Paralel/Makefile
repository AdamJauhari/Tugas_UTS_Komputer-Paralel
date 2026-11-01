# Makefile - build otomatis Monte Carlo pi (serial, Pthreads, OpenMP)
CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -D_GNU_SOURCE
LDLIBS  = -lm

all: pi_serial pi_pthread pi_openmp

pi_serial: pi_serial.c common.h report.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

pi_pthread: pi_pthread.c common.h report.h
	$(CC) $(CFLAGS) -pthread -o $@ $< $(LDLIBS)

pi_openmp: pi_openmp.c common.h report.h
	$(CC) $(CFLAGS) -fopenmp -o $@ $< $(LDLIBS)

# Verifikasi correctness: serial == pthread(T=1) == openmp(T=1); pthread == openmp utk T=2,4,8
test: all
	./verify.sh

# Benchmark penuh -> results.csv (N=1e9, thread 1,2,4,8, 5x ulangan)
bench: all
	./bench.sh

clean:
	rm -f pi_serial pi_pthread pi_openmp results.csv

.PHONY: all test bench clean
