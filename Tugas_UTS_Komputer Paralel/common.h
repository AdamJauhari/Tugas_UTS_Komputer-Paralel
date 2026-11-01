/*
 * common.h - Fungsi bersama untuk simulasi Monte Carlo estimasi pi.
 *
 * Berisi:
 *   - RNG xoshiro256** (generator modern, state 256 bit, TIDAK pakai state
 * global)
 *   - splitmix64 untuk seeding state tiap thread
 *   - count_hits(): inti komputasi (hitung titik di dalam lingkaran satuan)
 *   - chunk_size(): pembagian workload yang adil antar thread
 *   - now_sec(): timer wall-clock monotonic
 *
 * Semua versi (serial, Pthreads, OpenMP) memakai fungsi yang SAMA, sehingga
 * perbedaan performa murni berasal dari mekanisme paralelisasi.
 */
#ifndef COMMON_H
#define COMMON_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdint.h>
#include <time.h>

#define BASE_SEED 42ULL

typedef struct {
  uint64_t s[4];
} rng_t;

/* splitmix64: dipakai untuk mengisi state xoshiro dari satu seed 64 bit. */
static inline uint64_t splitmix64(uint64_t *x) {
  uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

static inline uint64_t rotl64(uint64_t v, int k) {
  return (v << k) | (v >> (64 - k));
}

/* Inisialisasi RNG milik satu thread; seed berbeda tiap thread id. */
static inline void rng_seed(rng_t *r, uint64_t seed, uint64_t tid) {
  uint64_t x = seed ^ ((tid + 1) * 0xD1B54A32D192ED03ULL);
  for (int i = 0; i < 4; i++)
    r->s[i] = splitmix64(&x);
}

/* xoshiro256** : menghasilkan 64 bit acak. */
static inline uint64_t rng_next(rng_t *r) {
  uint64_t *s = r->s;
  uint64_t result = rotl64(s[1] * 5, 7) * 9;
  uint64_t t = s[1] << 17;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = rotl64(s[3], 45);
  return result;
}

/* Bilangan double acak seragam di [0,1) dengan 53 bit mantissa. */
static inline double rng_double(rng_t *r) {
  return (double)(rng_next(r) >> 11) * (1.0 / 9007199254740992.0);
}

/*
 * Hitung berapa dari n titik acak (x,y) di [0,1]x[0,1] yang jatuh di dalam
 * lingkaran satuan (x^2 + y^2 <= 1). RNG bersifat LOKAL (private) per
 * pemanggil.
 */
static inline uint64_t count_hits(uint64_t n, uint64_t tid) {
  rng_t rng;
  rng_seed(&rng, BASE_SEED, tid);
  uint64_t hits = 0;
  for (uint64_t i = 0; i < n; i++) {
    double x = rng_double(&rng);
    double y = rng_double(&rng);
    hits += (x * x + y * y <= 1.0);
  }
  return hits;
}

/* Jumlah sampel untuk thread ke-tid dari total N dan T thread (sisa dibagi
 * merata). */
static inline uint64_t chunk_size(uint64_t N, int T, int tid) {
  return N / T + ((uint64_t)tid < N % (uint64_t)T ? 1 : 0);
}

static inline double now_sec(void) {
#ifdef _WIN32
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart / (double)freq.QuadPart;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

#endif
