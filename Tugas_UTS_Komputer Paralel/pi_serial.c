/* pi_serial.c - Baseline serial. Setara dengan versi paralel dengan T=1. */
#include "common.h"
#include "report.h"

int main(int argc, char **argv) {
    uint64_t N; int T, csv;
    parse_args(argc, argv, &N, &T, &csv);

    double t0 = now_sec();
    uint64_t hits = count_hits(N, 0);   /* satu RNG, thread id = 0 */
    double t1 = now_sec();

    report("serial", 1, N, hits, t1 - t0, csv);
    return 0;
}
