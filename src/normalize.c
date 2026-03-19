#include "postgres.h"

#include "pg_dtw/dtw.h"

double
dtw_apply_normalization(const DtwOptions *opts, const DtwResult *result)
{
    if (opts->normalization == DTW_NORMALIZATION_PATH_LENGTH)
    {
        if (result->path_length <= 0)
            ereport(ERROR, (errmsg("pg_dtw: path length cannot be zero for normalization")));
        return result->raw_cost / (double) result->path_length;
    }
    return result->raw_cost;
}
