/*
 * pi_pthread.c - Monte Carlo pi dengan POSIX Threads (pendekatan eksplisit).
 *
 * Strategi:
 *   - Workload: N sampel dibagi rata (block partition) ke T thread.
 *   - RNG: tiap thread membuat RNG lokal di dalam count_hits() (tidak ada state global).
 *   - Sinkronisasi / reduction MANUAL: tiap thread menulis hasil ke slot
 *     miliknya sendiri (results[tid]); thread utama menjumlahkan setelah
 *     pthread_join(). Tidak ada mutex di hot loop -> tidak ada race condition.
 *   - Slot hasil diberi padding 64 byte agar tidak terjadi false sharing.
 */
#include <pthread.h>
#include "common.h"
#include "report.h"

typedef struct {
    int      tid;
    uint64_t n;       /* jumlah sampel milik thread ini */
    char     pad0[48];
    uint64_t hits;    /* hasil lokal */
    char     pad1[56];/* padding -> satu cache line per struct */
} task_t;

static void *worker(void *arg) {
    task_t *t = (task_t *)arg;
    t->hits = count_hits(t->n, (uint64_t)t->tid);
    return NULL;
}

int main(int argc, char **argv) {
    uint64_t N; int T, csv;
    parse_args(argc, argv, &N, &T, &csv);

    pthread_t *th = malloc(sizeof(pthread_t) * T);
    task_t *tasks = aligned_alloc(64, sizeof(task_t) * T);
    for (int i = 0; i < T; i++) {
        tasks[i].tid = i;
        tasks[i].n = chunk_size(N, T, i);
        tasks[i].hits = 0;
    }

    double t0 = now_sec();
    for (int i = 0; i < T; i++)
        if (pthread_create(&th[i], NULL, worker, &tasks[i]) != 0) {
            perror("pthread_create"); return 1;
        }
    for (int i = 0; i < T; i++) pthread_join(th[i], NULL);

    uint64_t hits = 0;                       /* reduction manual */
    for (int i = 0; i < T; i++) hits += tasks[i].hits;
    double t1 = now_sec();

    report("pthread", T, N, hits, t1 - t0, csv);
    free(th); free(tasks);
    return 0;
}
