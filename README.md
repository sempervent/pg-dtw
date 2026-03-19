# pg-dtw

`pg-dtw` is a PostgreSQL extension (SQL extension name: `pg_dtw`) that implements Dynamic Time Warping (DTW) for timestamped univariate numeric sequences.

## Status

- v1 implemented: array-based DTW distance and path-length normalized DTW
- initial release target: `0.0.1`
- PostgreSQL targets: 15, 16, 17
- Build system: CMake (official)
- License: BSD-3-Clause

## What v1 provides

- `dtw_distance(timestamptz[], float8[], timestamptz[], float8[], jsonb)`
- `dtw_distance_normalized(...)`
- `dtw_canonicalize_sequence(timestamptz[], float8[], interval)`
- SQL wrappers for `timestamp[]`, `bigint[]`, and `numeric[]` value arrays
- strict validation for null/empty/non-finite/mismatched input
- timestamp-aware local cost with configurable weights
- canonicalization via sorting and duplicate bucket aggregation (`avg`)
- unconstrained DTW + optional Sakoe-Chiba band
- lower-bound scaffold function `dtw_lb_kim(float8[], float8[])`

## Build (native)

```bash
cmake -S . -B build
cmake --build build -j
cmake --install build
```

Use `PG_CONFIG=/path/to/pg_config` when needed.

## Python harness (no shell-init dependency)

Run environment probe:

```bash
python3 tools/env_probe.py
python3 tools/env_probe.py --clean-env
```

Run native workflow for one version:

```bash
python3 tools/run_native.py --pg-version 15
python3 tools/run_native.py --pg-version 16
python3 tools/run_native.py --pg-version 17
```

Run docker workflow:

```bash
python3 tools/run_docker.py --pg-version 15
python3 tools/run_docker.py
```

Run matrix convenience wrapper:

```bash
python3 tools/run_matrix.py native
python3 tools/run_matrix.py docker
```

Clean-env examples:

```bash
python3 tools/run_native.py --pg-version 15 --clean-env
python3 tools/run_docker.py --clean-env
```

## Docker / Compose workflow

Run the full matrix:

```bash
chmod +x test/run_pg_matrix.sh test/run_pg_regress.sh
docker compose build
docker compose run --rm pg15
docker compose run --rm pg16
docker compose run --rm pg17
```

Each target builds the extension, installs it into PostgreSQL in-container, runs SQL regression script, pgTAP checks, and fuzz/property tests.

## Option keys (v1)

- implemented: `metric`, `normalization`, `value_weight`, `time_weight`, `constraint`, `window`, `duplicate_bucket_us`, `duplicate_bucket`, `duplicate_agg`
- explicitly rejected as not implemented: `derivative`, `weighted`, `early_abandon`
- unknown keys are rejected with deterministic errors

## Quick SQL example

```sql
CREATE EXTENSION pg_dtw;

SELECT dtw_distance(
  ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
  ARRAY[1.0, 2.0]::float8[],
  ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:02+00'::timestamptz],
  ARRAY[1.0, 2.0]::float8[],
  '{"time_weight": 0.5, "metric":"absolute"}'::jsonb
);
```

## Repository docs

- `docs/architecture.md`
- `docs/sql-api.md`
- `docs/math-notes.md`
- `docs/algorithm-design.md`
- `docs/testing.md`
- `docs/performance.md`
- `docs/planner-index-design.md`
- `docs/roadmap.md`

## v1 limitations

- no custom base type
- no multivariate DTW implementation
- no alignment path surface yet
- no operator class or custom index AM implementation
- no TimescaleDB-specific integration code
