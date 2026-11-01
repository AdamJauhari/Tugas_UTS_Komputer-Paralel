# Paralel Monte Carlo π — Pthreads vs OpenMP
Estimasi nilai π dengan metode Monte Carlo (10⁹ titik) memakai tiga versi:
`pi_serial` (baseline), `pi_pthread` (POSIX Threads), `pi_openmp` (OpenMP).

## Struktur
| File | Fungsi |
|---|---|
| `common.h` | RNG xoshiro256** per-thread, `count_hits()`, pembagian workload, timer |
| `report.h` | Parsing argumen & format keluaran |
| `pi_serial.c` / `pi_pthread.c` / `pi_openmp.c` | Tiga implementasi |
| `verify.sh`, `bench.sh`, `plot.py` | Verifikasi, benchmark, grafik |

## Menjalankan di Lingkungan Windows

Jika dijalankan di Windows tanpa compiler C (`gcc`/`make`/WSL), gunakan script simulasi Python yang setara:
```bash
python run_monte_carlo.py
```
Script ini menjalankan simulasi benchmark lengkap (Serial, Pthreads eksplisit via `threading.Thread`, dan OpenMP directive via `ThreadPoolExecutor`) dengan algoritma PRNG vectorized per-thread, lalu otomatis menghasilkan [results.csv](file:///c:/Users/Adam/Downloads/Tugas_UTS_Komputer%20Paralel/results.csv), [summary.csv](file:///c:/Users/Adam/Downloads/Tugas_UTS_Komputer%20Paralel/summary.csv), dan grafik [speedup_efficiency.png](file:///c:/Users/Adam/Downloads/Tugas_UTS_Komputer%20Paralel/speedup_efficiency.png).

## Compile (Linux / WSL / MinGW)
```bash
make          # build semua
make clean
```
Kebutuhan: `gcc` (dengan dukungan OpenMP), `make`, Linux/WSL. Untuk grafik: `pip install pandas matplotlib`.

## Menjalankan
```bash
./pi_serial  [N]            # default N = 1000000000
./pi_pthread [N] [threads]
./pi_openmp  [N] [threads]
# tambahkan --csv untuk keluaran satu baris CSV
./pi_pthread 1000000000 4
```

## Verifikasi & Benchmark
```bash
make test                   # serial == pthread(T=1) == openmp(T=1); pthread == openmp tiap T
make bench                  # N=1e9, T=1,2,4,8, 5 ulangan -> results.csv
python3 plot.py             # summary.csv + speedup_efficiency.png
```
Variabel opsional: `N=100000000 REPS=3 make bench`.

## Desain Singkat
- **RNG**: xoshiro256** dengan state lokal; seed tiap thread = f(42, thread id) via splitmix64. Tidak ada RNG global.
- **Pthreads**: block partition, hasil per-thread di slot ber-padding (anti false sharing), reduction manual setelah `pthread_join`.
- **OpenMP**: `#pragma omp parallel` + `reduction(+:hits)`, `default(none)` agar scope variabel eksplisit.
- Pembagian workload & seed identik → hasil Pthreads = OpenMP untuk T yang sama, dan serial = T=1.
- Catatan: hasil (hits) bergantung pada jumlah thread karena aliran RNG berbeda, tetapi deterministik untuk T tertentu.
