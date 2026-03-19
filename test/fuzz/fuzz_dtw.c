#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

double ref_dtw_distance(const double *a, size_t n, const double *b, size_t m);

static uint64_t rng_state = 0x1234abcdefULL;

static uint64_t
xorshift64(void)
{
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

static double
rand_value(void)
{
    return ((double) (xorshift64() % 10000) / 1000.0) - 5.0;
}

int
main(void)
{
    int t;
    for (t = 0; t < 500; t++)
    {
        double a[16];
        double b[16];
        int n = 1 + (int) (xorshift64() % 15);
        int m = 1 + (int) (xorshift64() % 15);
        int i;
        double daa;
        double dbb;
        double dab;
        double dba;

        for (i = 0; i < n; i++)
            a[i] = rand_value();
        for (i = 0; i < m; i++)
            b[i] = rand_value();

        daa = ref_dtw_distance(a, n, a, n);
        dbb = ref_dtw_distance(b, m, b, m);
        dab = ref_dtw_distance(a, n, b, m);
        dba = ref_dtw_distance(b, m, a, n);

        if (fabs(daa) > 1e-9 || fabs(dbb) > 1e-9)
        {
            fprintf(stderr, "identity property failed\n");
            return 1;
        }
        if (fabs(dab - dba) > 1e-9)
        {
            fprintf(stderr, "symmetry property failed\n");
            return 1;
        }
        if (!(dab >= 0.0))
        {
            fprintf(stderr, "non-negativity property failed\n");
            return 1;
        }
    }

    printf("fuzz/property tests passed\n");
    return 0;
}
