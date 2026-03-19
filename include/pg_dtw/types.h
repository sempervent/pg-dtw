#ifndef PG_DTW_TYPES_H
#define PG_DTW_TYPES_H

#include "postgres.h"

typedef struct DtwPoint
{
    int64 ts_us;
    double value;
} DtwPoint;

typedef struct DtwSequence
{
    DtwPoint *points;
    int32 len;
} DtwSequence;

typedef enum DtwMetric
{
    DTW_METRIC_ABSOLUTE = 0,
    DTW_METRIC_SQUARED = 1
} DtwMetric;

typedef enum DtwNormalization
{
    DTW_NORMALIZATION_NONE = 0,
    DTW_NORMALIZATION_PATH_LENGTH = 1
} DtwNormalization;

typedef enum DtwConstraint
{
    DTW_CONSTRAINT_NONE = 0,
    DTW_CONSTRAINT_SAKOE_CHIBA = 1,
    DTW_CONSTRAINT_ITAKURA = 2
} DtwConstraint;

typedef enum DtwDuplicateAgg
{
    DTW_DUPLICATE_AGG_AVG = 0
} DtwDuplicateAgg;

typedef struct DtwOptions
{
    DtwMetric metric;
    DtwNormalization normalization;
    DtwConstraint constraint;
    DtwDuplicateAgg duplicate_agg;
    double value_weight;
    double time_weight;
    int32 window;
    int64 duplicate_bucket_us;
} DtwOptions;

typedef struct DtwResult
{
    double raw_cost;
    int32 path_length;
} DtwResult;

#endif
