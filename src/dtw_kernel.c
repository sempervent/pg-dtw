#include "postgres.h"

#include <float.h>
#include <limits.h>
#include <math.h>

#include "utils/memutils.h"

#include "pg_dtw/dtw.h"
#include "pg_dtw/errors.h"

static inline double
value_cost(DtwMetric metric, double a, double b)
{
    double d = fabs(a - b);
    if (metric == DTW_METRIC_SQUARED)
        return d * d;
    return d;
}

static inline double
local_cost(const DtwOptions *opts, const DtwPoint *a, const DtwPoint *b)
{
    double vc = value_cost(opts->metric, a->value, b->value);
    long double ts_diff = fabsl((long double) a->ts_us - (long double) b->ts_us);
    double tc = (double) (ts_diff / 1000000.0L);
    return (opts->value_weight * vc) + (opts->time_weight * tc);
}

static inline void
best_predecessor(
    double diag, double left, double up,
    int32 diag_len, int32 left_len, int32 up_len,
    double *best_cost, int32 *best_len
)
{
    *best_cost = diag;
    *best_len = diag_len;

    if (left < *best_cost || (left == *best_cost && left_len < *best_len))
    {
        *best_cost = left;
        *best_len = left_len;
    }
    if (up < *best_cost || (up == *best_cost && up_len < *best_len))
    {
        *best_cost = up;
        *best_len = up_len;
    }
}

DtwResult
dtw_compute_distance(const DtwOptions *opts, const DtwSequence *lhs, const DtwSequence *rhs)
{
    int32 n = lhs->len;
    int32 m = rhs->len;
    double *prev_cost;
    double *curr_cost;
    int32 *prev_len;
    int32 *curr_len;
    int32 i;
    int32 j;
    DtwResult out;
    int32 window = opts->window;

    if (n <= 0 || m <= 0)
        ereport(ERROR, (errmsg(DTW_ERR_EMPTY_SEQUENCE)));
    if (opts->constraint == DTW_CONSTRAINT_ITAKURA)
        ereport(ERROR, (errmsg(DTW_ERR_NOT_IMPLEMENTED)));
    if (opts->constraint == DTW_CONSTRAINT_SAKOE_CHIBA && window < 0)
        ereport(ERROR, (errmsg("pg_dtw: sakoe_chiba requires window option")));
    if ((Size) (m + 1) > (MaxAllocSize / sizeof(double)) ||
        (Size) (m + 1) > (MaxAllocSize / sizeof(int32)))
        ereport(ERROR, (errmsg("pg_dtw: input sequence is too large for current memory limits")));

    prev_cost = palloc(sizeof(double) * (m + 1));
    curr_cost = palloc(sizeof(double) * (m + 1));
    prev_len = palloc(sizeof(int32) * (m + 1));
    curr_len = palloc(sizeof(int32) * (m + 1));

    for (j = 0; j <= m; j++)
    {
        prev_cost[j] = INFINITY;
        prev_len[j] = INT_MAX;
    }
    prev_cost[0] = 0.0;
    prev_len[0] = 0;

    for (i = 1; i <= n; i++)
    {
        int32 j_start = 1;
        int32 j_end = m;

        if (opts->constraint == DTW_CONSTRAINT_SAKOE_CHIBA)
        {
            j_start = Max(1, i - window);
            j_end = Min(m, i + window);
        }

        for (j = 0; j <= m; j++)
        {
            curr_cost[j] = INFINITY;
            curr_len[j] = INT_MAX;
        }

        for (j = j_start; j <= j_end; j++)
        {
            double pred_cost;
            int32 pred_len;
            double lc = local_cost(opts, &lhs->points[i - 1], &rhs->points[j - 1]);

            best_predecessor(
                prev_cost[j - 1],
                curr_cost[j - 1],
                prev_cost[j],
                prev_len[j - 1],
                curr_len[j - 1],
                prev_len[j],
                &pred_cost,
                &pred_len
            );

            if (isfinite(pred_cost))
            {
                curr_cost[j] = pred_cost + lc;
                curr_len[j] = pred_len + 1;
            }
        }

        {
            double *tmpc = prev_cost;
            int32 *tmpl = prev_len;
            prev_cost = curr_cost;
            curr_cost = tmpc;
            prev_len = curr_len;
            curr_len = tmpl;
        }
    }

    if (!isfinite(prev_cost[m]))
        ereport(ERROR, (errmsg("pg_dtw: no feasible alignment path under configured constraints")));

    out.raw_cost = prev_cost[m];
    out.path_length = prev_len[m];

    pfree(prev_cost);
    pfree(curr_cost);
    pfree(prev_len);
    pfree(curr_len);

    return out;
}
