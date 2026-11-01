/*
 * pi_openmp.c - Monte Carlo pi dengan OpenMP (pendekatan compiler directive).
 *
 * Strategi:
 *   - "#pragma omp parallel num_threads(T)" membuat tim T thread.
 *   - Variabel tid, n (private) dideklarasikan di dalam region -> otomatis private.
 *   - N, T (shared) hanya dibaca.
 *   - "reduction(+:hits)" membuat salinan private 'hits' per thread, lalu
 *     menjumlahkan otomatis di akhir -> bebas race condition tanpa lock.
 *   - Pembagian workload & seed identik dengan versi Pthreads, sehingga
 *     hasil kedua versi harus sama persis untuk T yang sama.
 */
#include <omp.h>
#include "common.h"
#include "report.h"

int main(int argc, char **argv) {
    uint64_t N; int T, csv;
    parse_args(argc, argv, &N, &T, &csv);

    uint64_t hits = 0;
    double t0 = now_sec();

    #pragma omp parallel num_threads(T) default(none) shared(N, T) reduction(+:hits)
    {
        int tid = omp_get_thread_num();           /* private */
        uint64_t n = chunk_size(N, T, tid);       /* private */
        hits += count_hits(n, (uint64_t)tid);     /* RNG lokal di dalam count_hits */
    }

    double t1 = now_sec();
    report("openmp", T, N, hits, t1 - t0, csv);
    return 0;
}
