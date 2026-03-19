#include "postgres.h"

#include <math.h>

#include "catalog/pg_type_d.h"
#include "utils/array.h"
#include "utils/memutils.h"
#include "utils/timestamp.h"

#include "pg_dtw/errors.h"
#include "pg_dtw/types.h"

static int32
array_length_1d(ArrayType *arr)
{
    if (ARR_NDIM(arr) != 1)
        ereport(ERROR, (errmsg("pg_dtw: input arrays must be one-dimensional")));
    return ArrayGetNItems(ARR_NDIM(arr), ARR_DIMS(arr));
}

static void
validate_array_type(Oid ts_oid, Oid value_oid)
{
    if (ts_oid != TIMESTAMPTZOID)
        ereport(ERROR, (errmsg("pg_dtw: timestamp arrays must be timestamptz[] in v1")));
    if (value_oid != FLOAT8OID)
        ereport(ERROR, (errmsg("pg_dtw: value arrays must be double precision[] in core functions")));
}

DtwSequence
dtw_sequence_from_arrays(ArrayType *ts_array, ArrayType *value_array)
{
    DtwSequence seq;
    Datum *ts_datums;
    Datum *value_datums;
    bool *ts_nulls;
    bool *value_nulls;
    int ts_len;
    int v_len;
    int i;

    if (ts_array == NULL || value_array == NULL)
        ereport(ERROR, (errmsg(DTW_ERR_NULL_ARRAY)));

    validate_array_type(ARR_ELEMTYPE(ts_array), ARR_ELEMTYPE(value_array));

    ts_len = array_length_1d(ts_array);
    v_len = array_length_1d(value_array);
    if (ts_len == 0 || v_len == 0)
        ereport(ERROR, (errmsg(DTW_ERR_EMPTY_SEQUENCE)));
    if (ts_len != v_len)
        ereport(ERROR, (errmsg(DTW_ERR_LENGTH_MISMATCH)));
    if ((Size) ts_len > (MaxAllocSize / sizeof(DtwPoint)))
        ereport(ERROR, (errmsg("pg_dtw: input sequence is too large for current memory limits")));

    deconstruct_array(
        ts_array,
        TIMESTAMPTZOID,
        8,
        true,
        'd',
        &ts_datums,
        &ts_nulls,
        &ts_len
    );
    deconstruct_array(
        value_array,
        FLOAT8OID,
        8,
        FLOAT8PASSBYVAL,
        'd',
        &value_datums,
        &value_nulls,
        &v_len
    );

    seq.len = ts_len;
    seq.points = palloc0(sizeof(DtwPoint) * seq.len);

    for (i = 0; i < seq.len; i++)
    {
        double value;
        if (ts_nulls[i] || value_nulls[i])
            ereport(ERROR, (errmsg(DTW_ERR_NULL_ELEMENT)));

        value = DatumGetFloat8(value_datums[i]);
        if (!isfinite(value))
            ereport(ERROR, (errmsg(DTW_ERR_NONFINITE_VALUE)));

        seq.points[i].ts_us = DatumGetTimestampTz(ts_datums[i]);
        seq.points[i].value = value;
    }

    pfree(ts_datums);
    pfree(value_datums);
    pfree(ts_nulls);
    pfree(value_nulls);
    return seq;
}
