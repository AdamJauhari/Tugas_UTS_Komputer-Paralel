#!/bin/bash
# verify.sh - Cek correctness: hasil paralel harus sama persis dengan serial (T=1)
# dan Pthreads harus sama dengan OpenMP untuk jumlah thread yang sama.
N=${1:-10000000}
fail=0
get() { "$@" --csv | cut -d, -f4; }   # kolom 4 = hits
S=$(get ./pi_serial $N)
echo "serial            hits=$S"
for T in 1 2 4 8; do
  P=$(get ./pi_pthread $N $T); O=$(get ./pi_openmp $N $T)
  ok="OK"; [ "$P" != "$O" ] && ok="BEDA" && fail=1
  [ $T -eq 1 ] && [ "$P" != "$S" ] && ok="BEDA-DARI-SERIAL" && fail=1
  echo "T=$T pthread=$P openmp=$O -> $ok"
done
[ $fail -eq 0 ] && echo "SEMUA TES LULUS" || { echo "ADA TES GAGAL"; exit 1; }
