import time
import math
import threading
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from concurrent.futures import ThreadPoolExecutor

# Seed dasar
BASE_SEED = 42

def count_hits_worker(n_samples, seed_val):
    """
    Menghitung jumlah titik yang jatuh di dalam lingkaran satuan (x^2 + y^2 <= 1.0).
    Menggunakan generator acak lokal (PCG64) per thread/worker untuk thread-safety.
    Diproses secara vectorized dalam batch agar efisien memori dan cepat.
    """
    rng = np.random.default_rng(seed_val)
    batch_size = 5000000
    hits = 0
    remaining = n_samples
    while remaining > 0:
        cur = min(remaining, batch_size)
        x = rng.random(cur)
        y = rng.random(cur)
        hits += int(np.sum((x * x + y * y) <= 1.0))
        remaining -= cur
    return hits

class PthreadWorkerThread(threading.Thread):
    """
    Merepresentasikan model eksplisit Pthreads (POSIX Threads):
    - Thread dibuat secara eksplisit (pthread_create setara thread.start())
    - Hasil ditampung pada variabel lokal thread (menghindari false sharing & mutex contention)
    - Thread di-join (pthread_join setara thread.join()) lalu diakumulasi
    """
    def __init__(self, n_samples, seed_val):
        super().__init__()
        self.n_samples = n_samples
        self.seed_val = seed_val
        self.hits = 0

    def run(self):
        self.hits = count_hits_worker(self.n_samples, self.seed_val)

def run_simulation(impl, N, threads):
    t_start = time.perf_counter()
    chunk = N // threads
    remainder = N % threads
    tasks = []
    for tid in range(threads):
        cnt = chunk + (1 if tid < remainder else 0)
        seed = BASE_SEED + tid * 10007
        tasks.append((cnt, seed))

    if impl == "serial":
        hits = count_hits_worker(N, BASE_SEED)
    elif impl == "pthread":
        # Pendekatan eksplisit mirip POSIX Threads (threading.Thread)
        th_list = [PthreadWorkerThread(c, s) for c, s in tasks]
        for th in th_list:
            th.start()
        for th in th_list:
            th.join()
        hits = sum(th.hits for th in th_list)
    elif impl == "openmp":
        # Pendekatan compiler directive / thread pool mirip OpenMP
        with ThreadPoolExecutor(max_workers=threads) as executor:
            futures = [executor.submit(count_hits_worker, c, s) for c, s in tasks]
            hits = sum(f.result() for f in futures)

    t_end = time.perf_counter()
    elapsed = t_end - t_start
    pi_est = 4.0 * hits / N
    err = abs(pi_est - math.pi)
    return impl, threads, N, hits, pi_est, err, elapsed

def main():
    N = 50_000_000
    REPS = 3
    print("=== Menjalankan Simulasi Paralel Monte Carlo Pi ===")
    print(f"N = {N:,} titik, Pengulangan (REPS) = {REPS}x")
    print("Menguji Serial, Pthreads (threading.Thread), dan OpenMP (ThreadPool)...\n")

    records = []
    
    # 1. Serial
    print("[1/3] Menjalankan Serial...")
    for r in range(REPS):
        res = run_simulation("serial", N, 1)
        records.append(res)
        print(f"  Rep {r+1}: Waktu = {res[6]:.4f}s, Pi = {res[4]:.7f}, Error = {res[5]:.2e}")

    # 2. Pthreads & OpenMP untuk thread 1, 2, 4, 8
    threads_list = [1, 2, 4, 8]
    print("\n[2/3] Menjalankan Pthreads...")
    for T in threads_list:
        for r in range(REPS):
            res = run_simulation("pthread", N, T)
            records.append(res)
            print(f"  Thread {T} - Rep {r+1}: Waktu = {res[6]:.4f}s, Pi = {res[4]:.7f}")

    print("\n[3/3] Menjalankan OpenMP...")
    for T in threads_list:
        for r in range(REPS):
            res = run_simulation("openmp", N, T)
            records.append(res)
            print(f"  Thread {T} - Rep {r+1}: Waktu = {res[6]:.4f}s, Pi = {res[4]:.7f}")

    # Simpan hasil ke results.csv
    df = pd.DataFrame(records, columns=["impl", "threads", "N", "hits", "pi", "error", "time"])
    df.to_csv("results.csv", index=False)
    print("\nHasil benchmark disimpan ke results.csv!")

    # Analisis & visualisasi sesuai plot.py asli
    t_serial = df[df.impl == "serial"].time.mean()
    g = df[df.impl != "serial"].groupby(["impl", "threads"]).agg(
        time=("time", "mean"), std=("time", "std"), error=("error", "mean")).reset_index()
    g["speedup"] = t_serial / g["time"]
    g["efficiency"] = g["speedup"] / g["threads"]

    print("\n=== RINGKASAN BENCHMARK ===")
    print(f"Waktu serial rata-rata: {t_serial:.4f} s\n")
    print(g.to_string(index=False))
    g.to_csv("summary.csv", index=False)

    # Plot grafik
    fig, ax = plt.subplots(1, 2, figsize=(11, 4))
    for impl, d in g.groupby("impl"):
        ax[0].plot(d.threads, d.speedup, marker="o", label=impl)
        ax[1].plot(d.threads, d.efficiency, marker="o", label=impl)
    ax[0].plot([1, 8], [1, 8], "k--", label="ideal")
    ax[0].set(title="Speedup", xlabel="Thread", ylabel="Speedup", xticks=[1, 2, 4, 8])
    ax[1].axhline(1, color="k", ls="--")
    ax[1].set(title="Efficiency", xlabel="Thread", ylabel="Efficiency", xticks=[1, 2, 4, 8])
    for a in ax:
        a.legend()
        a.grid(alpha=.3)
    plt.tight_layout()
    plt.savefig("speedup_efficiency.png", dpi=150)
    print("\nGrafik berhasil disimpan ke 'speedup_efficiency.png'!")

if __name__ == "__main__":
    main()
