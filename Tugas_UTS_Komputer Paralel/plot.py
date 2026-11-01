#!/usr/bin/env python3
"""plot.py - Baca results.csv, hitung rata-rata waktu, speedup, efficiency, dan buat grafik.
Pakai: python3 plot.py   (butuh pandas & matplotlib)"""
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")
t_serial = df[df.impl == "serial"].time.mean()
g = df[df.impl != "serial"].groupby(["impl", "threads"]).agg(
    time=("time", "mean"), std=("time", "std"), error=("error", "mean")).reset_index()
g["speedup"] = t_serial / g["time"]
g["efficiency"] = g["speedup"] / g["threads"]
print(f"Waktu serial rata-rata: {t_serial:.4f} s")
print(g.to_string(index=False))
g.to_csv("summary.csv", index=False)

fig, ax = plt.subplots(1, 2, figsize=(11, 4))
for impl, d in g.groupby("impl"):
    ax[0].plot(d.threads, d.speedup, marker="o", label=impl)
    ax[1].plot(d.threads, d.efficiency, marker="o", label=impl)
ax[0].plot([1, 8], [1, 8], "k--", label="ideal")
ax[0].set(title="Speedup", xlabel="Thread", ylabel="Speedup", xticks=[1, 2, 4, 8])
ax[1].axhline(1, color="k", ls="--")
ax[1].set(title="Efficiency", xlabel="Thread", ylabel="Efficiency", xticks=[1, 2, 4, 8])
for a in ax: a.legend(); a.grid(alpha=.3)
plt.tight_layout(); plt.savefig("speedup_efficiency.png", dpi=150)
