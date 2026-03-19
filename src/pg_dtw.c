#include "postgres.h"

#include <math.h>

#include "catalog/pg_type_d.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/jsonb.h"

#include "pg_dtw/canonicalize.h"
#include "pg_dtw/dtw.h"
#include "pg_dtw/options.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(dtw_distance);
PG_FUNCTION_INFO_V1(dtw_distance_normalized);
PG_FUNCTION_INFO_V1(dtw_lb_kim);

Datum
dtw_distance(PG_FUNCTION_ARGS)
{
    ArrayType  *ts1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType  *vals1 = PG_GETARG_ARRAYTYPE_P(1);
    ArrayType  *ts2 = PG_GETARG_ARRAYTYPE_P(2);
    ArrayType  *vals2 = PG_GETARG_ARRAYTYPE_P(3);
    Jsonb      *options = PG_ARGISNULL(4) ? NULL : PG_GETARG_JSONB_P(4);
    DtwOptions  opts;
    DtwSequence lhs;
    DtwSequence rhs;
    DtwResult   result;

    dtw_options_init_defaults(&opts);
    if (options != NULL)
        dtw_parse_options(options, &opts);

    lhs = dtw_sequence_from_arrays(ts1, vals1);
    rhs = dtw_sequence_from_arrays(ts2, vals2);

    dtw_sequence_canonicalize(&opts, &lhs);
    dtw_sequence_canonicalize(&opts, &rhs);

    result = dtw_compute_distance(&opts, &lhs, &rhs);

    dtw_sequence_free(&lhs);
    dtw_sequence_free(&rhs);

    PG_RETURN_FLOAT8(result.raw_cost);
}

Datum
dtw_distance_normalized(PG_FUNCTION_ARGS)
{
    ArrayType  *ts1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType  *vals1 = PG_GETARG_ARRAYTYPE_P(1);
    ArrayType  *ts2 = PG_GETARG_ARRAYTYPE_P(2);
    ArrayType  *vals2 = PG_GETARG_ARRAYTYPE_P(3);
    Jsonb      *options = PG_ARGISNULL(4) ? NULL : PG_GETARG_JSONB_P(4);
    DtwOptions  opts;
    DtwSequence lhs;
    DtwSequence rhs;
    DtwResult   result;
    double      normalized;

    dtw_options_init_defaults(&opts);
    opts.normalization = DTW_NORMALIZATION_PATH_LENGTH;
    if (options != NULL)
        dtw_parse_options(options, &opts);

    lhs = dtw_sequence_from_arrays(ts1, vals1);
    rhs = dtw_sequence_from_arrays(ts2, vals2);

    dtw_sequence_canonicalize(&opts, &lhs);
    dtw_sequence_canonicalize(&opts, &rhs);

    result = dtw_compute_distance(&opts, &lhs, &rhs);
    normalized = dtw_apply_normalization(&opts, &result);

    dtw_sequence_free(&lhs);
    dtw_sequence_free(&rhs);

    PG_RETURN_FLOAT8(normalized);
}

Datum
dtw_lb_kim(PG_FUNCTION_ARGS)
{
    ArrayType   *vals1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType   *vals2 = PG_GETARG_ARRAYTYPE_P(1);
    Datum       *d1;
    Datum       *d2;
    bool        *n1;
    bool        *n2;
    int          l1;
    int          l2;
    double       v1s;
    double       v1e;
    double       v2s;
    double       v2e;

    deconstruct_array(vals1, FLOAT8OID, 8, FLOAT8PASSBYVAL, 'd', &d1, &n1, &l1);
    deconstruct_array(vals2, FLOAT8OID, 8, FLOAT8PASSBYVAL, 'd', &d2, &n2, &l2);

    if (l1 == 0 || l2 == 0)
        ereport(ERROR, (errmsg("pg_dtw: lb_kim input arrays must be non-empty")));
    if (n1[0] || n1[l1 - 1] || n2[0] || n2[l2 - 1])
        ereport(ERROR, (errmsg("pg_dtw: lb_kim input arrays must not contain NULL values")));

    v1s = DatumGetFloat8(d1[0]);
    v1e = DatumGetFloat8(d1[l1 - 1]);
    v2s = DatumGetFloat8(d2[0]);
    v2e = DatumGetFloat8(d2[l2 - 1]);
    if (!isfinite(v1s) || !isfinite(v1e) || !isfinite(v2s) || !isfinite(v2e))
        ereport(ERROR, (errmsg("pg_dtw: lb_kim input values must be finite")));

    PG_RETURN_FLOAT8(fabs(v1s - v2s) + fabs(v1e - v2e));
}
