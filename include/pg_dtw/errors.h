#ifndef PG_DTW_ERRORS_H
#define PG_DTW_ERRORS_H

#define DTW_ERR_PREFIX "pg_dtw: "

#define DTW_ERR_NULL_ARRAY DTW_ERR_PREFIX "input arrays must not be NULL"
#define DTW_ERR_EMPTY_SEQUENCE DTW_ERR_PREFIX "input sequences must be non-empty"
#define DTW_ERR_LENGTH_MISMATCH DTW_ERR_PREFIX "timestamp/value array lengths must match"
#define DTW_ERR_NULL_ELEMENT DTW_ERR_PREFIX "input arrays must not contain NULL elements"
#define DTW_ERR_NONFINITE_VALUE DTW_ERR_PREFIX "input values must be finite"
#define DTW_ERR_BAD_OPTION DTW_ERR_PREFIX "invalid options jsonb"
#define DTW_ERR_NOT_IMPLEMENTED DTW_ERR_PREFIX "requested option is not yet implemented"

#endif
