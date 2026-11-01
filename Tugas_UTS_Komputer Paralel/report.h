/* report.h - Format keluaran seragam untuk ketiga program. */
#ifndef REPORT_H
#define REPORT_H
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* Cetak hasil. Jika csv != 0, cetak satu baris CSV: impl,threads,N,hits,pi,error,time */
static inline void report(const char *impl, int T, uint64_t N, uint64_t hits,
                          double t, int csv) {
    double pi = 4.0 * (double)hits / (double)N;
    double err = fabs(pi - M_PI);
    if (csv) {
        printf("%s,%d,%llu,%llu,%.10f,%.3e,%.4f\n", impl, T,
               (unsigned long long)N, (unsigned long long)hits, pi, err, t);
    } else {
        printf("Implementasi : %s\nThread       : %d\nSampel (N)   : %llu\n"
               "Hits         : %llu\nEstimasi pi  : %.10f\nError |pi-est|: %.3e\n"
               "Waktu        : %.4f detik\n", impl, T, (unsigned long long)N,
               (unsigned long long)hits, pi, err, t);
    }
}

/* Parse argumen: ./prog [N] [threads] [--csv] */
static inline void parse_args(int argc, char **argv, uint64_t *N, int *T, int *csv) {
    *N = 1000000000ULL; *T = 1; *csv = 0;
    int pos = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--csv")) *csv = 1;
        else if (pos == 0) { *N = strtoull(argv[i], NULL, 10); pos++; }
        else if (pos == 1) { *T = atoi(argv[i]); pos++; }
    }
    if (*T < 1) *T = 1;
}
#endif
