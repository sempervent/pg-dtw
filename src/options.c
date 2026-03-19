#include "postgres.h"

#include <limits.h>
#include <math.h>

#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/jsonb.h"
#include "utils/numeric.h"
#include "utils/timestamp.h"

#include "pg_dtw/errors.h"
#include "pg_dtw/options.h"

static double
jsonb_numeric_to_double(JsonbValue *val, const char *key)
{
    Datum d;
    double out;

    if (val->type != jbvNumeric)
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("option '%s' must be numeric", key)));

    d = DirectFunctionCall1(numeric_float8, NumericGetDatum(val->val.numeric));
    out = DatumGetFloat8(d);
    if (!isfinite(out))
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("option '%s' must be finite", key)));
    return out;
}

static int64
jsonb_interval_to_bucket_us(JsonbValue *val, const char *key)
{
    Datum interval_datum;
    Interval *itv;
    int64 micros;

    if (val->type != jbvString)
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("option '%s' must be an interval string", key)));

    interval_datum = DirectFunctionCall3(
        interval_in,
        CStringGetDatum(pnstrdup(val->val.string.val, val->val.string.len)),
        ObjectIdGetDatum(InvalidOid),
        Int32GetDatum(-1)
    );
    itv = DatumGetIntervalP(interval_datum);

    if (itv->month != 0 || itv->day != 0)
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("option '%s' must be a pure time interval without days or months", key)));
    if (itv->time <= 0)
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("option '%s' must be > 0", key)));

    micros = itv->time;
    return micros;
}

void
dtw_options_init_defaults(DtwOptions *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->metric = DTW_METRIC_ABSOLUTE;
    opts->normalization = DTW_NORMALIZATION_NONE;
    opts->constraint = DTW_CONSTRAINT_NONE;
    opts->duplicate_agg = DTW_DUPLICATE_AGG_AVG;
    opts->value_weight = 1.0;
    opts->time_weight = 0.0;
    opts->window = -1;
    opts->duplicate_bucket_us = 1;
}

void
dtw_parse_options(Jsonb *jsonb, DtwOptions *opts)
{
    JsonbIterator *it;
    JsonbValue v;
    int r;
    char *pending_key = NULL;

    it = JsonbIteratorInit(&jsonb->root);
    while ((r = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
    {
        if (r == WJB_KEY)
        {
            pending_key = pnstrdup(v.val.string.val, v.val.string.len);
            continue;
        }
        if (r != WJB_VALUE || pending_key == NULL)
            continue;

        if (strcmp(pending_key, "metric") == 0)
        {
            if (v.type != jbvString)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("metric must be a string")));
            if (pg_strncasecmp(v.val.string.val, "absolute", v.val.string.len) == 0)
                opts->metric = DTW_METRIC_ABSOLUTE;
            else if (pg_strncasecmp(v.val.string.val, "squared", v.val.string.len) == 0)
                opts->metric = DTW_METRIC_SQUARED;
            else
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("unsupported metric")));
        }
        else if (strcmp(pending_key, "normalization") == 0)
        {
            if (v.type != jbvString)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("normalization must be a string")));
            if (pg_strncasecmp(v.val.string.val, "none", v.val.string.len) == 0)
                opts->normalization = DTW_NORMALIZATION_NONE;
            else if (pg_strncasecmp(v.val.string.val, "path_length", v.val.string.len) == 0)
                opts->normalization = DTW_NORMALIZATION_PATH_LENGTH;
            else
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("unsupported normalization")));
        }
        else if (strcmp(pending_key, "constraint") == 0)
        {
            if (v.type != jbvString)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("constraint must be a string")));
            if (pg_strncasecmp(v.val.string.val, "none", v.val.string.len) == 0)
                opts->constraint = DTW_CONSTRAINT_NONE;
            else if (pg_strncasecmp(v.val.string.val, "sakoe_chiba", v.val.string.len) == 0)
                opts->constraint = DTW_CONSTRAINT_SAKOE_CHIBA;
            else if (pg_strncasecmp(v.val.string.val, "itakura", v.val.string.len) == 0)
                opts->constraint = DTW_CONSTRAINT_ITAKURA;
            else
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("unsupported constraint")));
        }
        else if (strcmp(pending_key, "value_weight") == 0)
        {
            opts->value_weight = jsonb_numeric_to_double(&v, "value_weight");
            if (opts->value_weight < 0.0)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("value_weight must be >= 0")));
        }
        else if (strcmp(pending_key, "time_weight") == 0)
        {
            opts->time_weight = jsonb_numeric_to_double(&v, "time_weight");
            if (opts->time_weight < 0.0)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("time_weight must be >= 0")));
        }
        else if (strcmp(pending_key, "window") == 0)
        {
            double wd = jsonb_numeric_to_double(&v, "window");
            if (wd < 0.0 || wd > (double) INT_MAX)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("window must be in [0, int_max]")));
            opts->window = (int32) wd;
        }
        else if (strcmp(pending_key, "duplicate_bucket_us") == 0)
        {
            double bd = jsonb_numeric_to_double(&v, "duplicate_bucket_us");
            if (bd < 1.0 || bd > (double) LLONG_MAX)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("duplicate_bucket_us must be >= 1")));
            opts->duplicate_bucket_us = (int64) bd;
        }
        else if (strcmp(pending_key, "duplicate_bucket") == 0)
        {
            opts->duplicate_bucket_us = jsonb_interval_to_bucket_us(&v, "duplicate_bucket");
        }
        else if (strcmp(pending_key, "duplicate_agg") == 0)
        {
            if (v.type != jbvString)
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("duplicate_agg must be a string")));
            if (pg_strncasecmp(v.val.string.val, "avg", v.val.string.len) == 0)
                opts->duplicate_agg = DTW_DUPLICATE_AGG_AVG;
            else
                ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("unsupported duplicate_agg")));
        }
        else if (strcmp(pending_key, "derivative") == 0 ||
                 strcmp(pending_key, "weighted") == 0 ||
                 strcmp(pending_key, "early_abandon") == 0)
        {
            ereport(ERROR, (errmsg(DTW_ERR_NOT_IMPLEMENTED), errdetail("option '%s' is parsed but unavailable in v1", pending_key)));
        }
        else
        {
            ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("unknown option '%s'", pending_key)));
        }
        pfree(pending_key);
        pending_key = NULL;
    }

    if (pending_key != NULL)
        pfree(pending_key);

    if (opts->constraint == DTW_CONSTRAINT_ITAKURA)
        ereport(ERROR, (errmsg(DTW_ERR_NOT_IMPLEMENTED), errdetail("itakura constraint is planned for a later phase")));
    if (opts->constraint == DTW_CONSTRAINT_SAKOE_CHIBA && opts->window < 0)
        ereport(ERROR, (errmsg(DTW_ERR_BAD_OPTION), errdetail("window is required for sakoe_chiba")));
}
