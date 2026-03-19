#include "postgres.h"

#include <stdlib.h>

#include "utils/memutils.h"

#include "pg_dtw/canonicalize.h"

static int
point_ts_cmp(const void *a, const void *b)
{
    const DtwPoint *pa = (const DtwPoint *) a;
    const DtwPoint *pb = (const DtwPoint *) b;

    if (pa->ts_us < pb->ts_us)
        return -1;
    if (pa->ts_us > pb->ts_us)
        return 1;
    return 0;
}

static inline int64
floor_div_i64(int64 x, int64 y)
{
    int64 q = x / y;
    int64 r = x % y;

    if (r != 0 && ((r > 0) != (y > 0)))
        q--;
    return q;
}

void
dtw_sequence_canonicalize(const DtwOptions *opts, DtwSequence *seq)
{
    DtwPoint *dst;
    int32 out = 0;
    int32 i = 0;
    int64 bucket = opts->duplicate_bucket_us;

    if (seq->len <= 1)
        return;
    if (bucket < 1)
        bucket = 1;

    qsort(seq->points, seq->len, sizeof(DtwPoint), point_ts_cmp);

    dst = palloc0(sizeof(DtwPoint) * seq->len);

    while (i < seq->len)
    {
        int32 j = i + 1;
        int64 bucket_key = floor_div_i64(seq->points[i].ts_us, bucket);
        double sum = seq->points[i].value;
        int32 count = 1;

        while (j < seq->len && floor_div_i64(seq->points[j].ts_us, bucket) == bucket_key)
        {
            sum += seq->points[j].value;
            count++;
            j++;
        }

        dst[out].ts_us = bucket_key * bucket;
        dst[out].value = sum / (double) count;
        out++;
        i = j;
    }

    pfree(seq->points);
    seq->points = dst;
    seq->len = out;
}

void
dtw_sequence_free(DtwSequence *seq)
{
    if (seq->points != NULL)
        pfree(seq->points);
    seq->points = NULL;
    seq->len = 0;
}
