# Architecture

`pg_dtw` is split into small C modules with explicit boundaries:

- `src/pg_dtw.c`: SQL-callable entrypoints and high-level orchestration
- `src/options.c`: JSONB option parsing and validation
- `src/array_input.c`: array extraction, type checks, and finite-value validation
- `src/canonicalize.c`: timestamp sorting and duplicate bucket aggregation
- `src/dtw_kernel.c`: DTW dynamic programming kernel
- `src/normalize.c`: result normalization policies
- `src/lower_bounds.c`: v1 reserve module for pruning/lower-bound growth

Internal canonical type is a contiguous array of `DtwPoint { int64 ts_us; double value; }`.

SQL helper surface includes `dtw_canonicalize_sequence(...)` for explicit canonicalization visibility, while C entrypoints always canonicalize internally.

## Request flow

1. SQL function enters `pg_dtw.c`
2. defaults loaded, `options jsonb` parsed
3. arrays converted to `DtwSequence`
4. each sequence canonicalized (sort + bucket aggregate)
5. DTW kernel computes accumulated cost and path length
6. optional normalization applied
7. float8 result returned

## Memory and contexts

All allocations use PostgreSQL `palloc` in the function memory context; failures are raised via `ereport(ERROR, ...)` and unwound by PostgreSQL.

## Extension identity

- Repository: `pg-dtw`
- SQL extension name: `pg_dtw`
