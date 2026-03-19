#include <math.h>
#include <stddef.h>

double
ref_dtw_distance(const double *a, size_t n, const double *b, size_t m)
{
    size_t i;
    size_t j;
    double dp[64][64];

    if (n == 0 || m == 0 || n > 63 || m > 63)
        return NAN;

    for (i = 0; i <= n; i++)
        for (j = 0; j <= m; j++)
            dp[i][j] = INFINITY;

    dp[0][0] = 0.0;
    for (i = 1; i <= n; i++)
    {
        for (j = 1; j <= m; j++)
        {
            double c = fabs(a[i - 1] - b[j - 1]);
            double best = dp[i - 1][j - 1];
            if (dp[i - 1][j] < best)
                best = dp[i - 1][j];
            if (dp[i][j - 1] < best)
                best = dp[i][j - 1];
            dp[i][j] = c + best;
        }
    }

    return dp[n][m];
}
