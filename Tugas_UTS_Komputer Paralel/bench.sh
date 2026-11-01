#!/bin/bash
# bench.sh - Benchmark: ulangi REPS kali, simpan ke results.csv
N=${N:-1000000000}; REPS=${REPS:-5}
echo "impl,threads,N,hits,pi,error,time" > results.csv
for r in $(seq $REPS); do
  ./pi_serial $N --csv >> results.csv
  for T in 1 2 4 8; do
    ./pi_pthread $N $T --csv >> results.csv
    ./pi_openmp  $N $T --csv >> results.csv
  done
done
echo "Selesai -> results.csv"
