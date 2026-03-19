# SQL API (v1)

## Core functions

```sql
dtw_distance(
  ts1 timestamptz[],
  vals1 double precision[],
  ts2 timestamptz[],
  vals2 double precision[],
  options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
```

```sql
dtw_distance_normalized(
  ts1 timestamptz[],
  vals1 double precision[],
  ts2 timestamptz[],
  vals2 double precision[],
  options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
```

`dtw_distance_normalized` applies path-length normalization.

## Helper wrappers

v1 includes SQL wrappers for:

- `timestamp[]` timestamps (cast into `timestamptz[]`)
- `bigint[]` values
- `numeric[]` values

v1 also includes:

- `dtw_canonicalize_sequence(ts_in timestamptz[], vals_in float8[], duplicate_bucket interval)`

## Options JSONB

- `metric`: `"absolute"` or `"squared"`
- `normalization`: `"none"` or `"path_length"`
- `value_weight`: `>= 0`
- `time_weight`: `>= 0`
- `constraint`: `"none"` or `"sakoe_chiba"` (and `"itakura"` parsed but rejected)
- `window`: non-negative integer (required with `sakoe_chiba`)
- `duplicate_bucket_us`: integer microsecond bucket size (`>=1`)
- `duplicate_bucket`: interval string (pure time interval, no day/month fields)
- `duplicate_agg`: `"avg"`

Unsupported options such as `derivative`, `weighted`, and `early_abandon` are explicitly rejected as not yet implemented.
Unknown option keys are rejected.

## Validation behavior

- NULL arrays/elements: error
- empty arrays: error
- length mismatch between ts/values: error
- NaN and +/-Inf in values: error

## Rowset usage patterns (Phase 1.1 scaffolding)

See `examples/rowset_patterns.sql` for SQL-first ingestion from rowsets using `array_agg(... ORDER BY ts)`.

## Notes on canonicalization helper

`dtw_canonicalize_sequence` is a SQL utility for inspection and ETL flows. The core C DTW functions perform canonicalization internally regardless of helper usage.
