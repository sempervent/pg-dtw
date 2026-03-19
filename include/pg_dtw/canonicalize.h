#ifndef PG_DTW_CANONICALIZE_H
#define PG_DTW_CANONICALIZE_H

#include "postgres.h"
#include "utils/array.h"

#include "pg_dtw/types.h"

DtwSequence dtw_sequence_from_arrays(ArrayType *ts_array, ArrayType *value_array);
void dtw_sequence_canonicalize(const DtwOptions *opts, DtwSequence *seq);
void dtw_sequence_free(DtwSequence *seq);

#endif
