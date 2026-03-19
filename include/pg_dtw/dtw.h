#ifndef PG_DTW_DTW_H
#define PG_DTW_DTW_H

#include "pg_dtw/types.h"

DtwResult dtw_compute_distance(
    const DtwOptions *opts,
    const DtwSequence *lhs,
    const DtwSequence *rhs
);

double dtw_apply_normalization(const DtwOptions *opts, const DtwResult *result);

#endif
