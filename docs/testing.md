# Testing

`pg-dtw` v1 ships with:

- SQL regression script in `test/sql/pg_dtw_core.sql`
- expected output baseline in `test/expected/pg_dtw_core.out`
- pgTAP contract tests in `test/pgtap/pg_dtw_api.sql`
- fuzz/property harness in `test/fuzz/`

## Matrix runner

`test/run_pg_matrix.sh <major>` performs:

1. CMake configure/build/install
2. ephemeral PostgreSQL init/start
3. extension install smoke (`CREATE EXTENSION pg_dtw`)
4. `pg_regress` run with expected-output matching
5. pgTAP run through `pg_prove` (failing tests return non-zero)
6. fuzz/property run

## Local

Use Docker compose for version matrix (15/16/17), or run native with an installed PostgreSQL server and matching `pg_config`.

## Python harness workflows

The `tools/` scripts are designed for environments where shell startup/init is unreliable.

- Probe toolchain: `python3 tools/env_probe.py`
- Native single version: `python3 tools/run_native.py --pg-version 15`
- Docker single version: `python3 tools/run_docker.py --pg-version 15`
- Full matrix: `python3 tools/run_matrix.py native` or `python3 tools/run_matrix.py docker`
- Reduced env mode: append `--clean-env`

## Important stability decisions

- regression expected output avoids unstable extension NOTICE noise
- wrapper/coercion and option-parse failures are covered in SQL and pgTAP tests
- Docker matrix scripts set explicit PG connection env (`PGUSER`, `PGDATABASE`, `PGHOST`, `PGPORT`) so tools never default to `root`
