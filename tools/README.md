# Python Execution Harness

This directory contains a shell-init-independent execution harness for local development.

## Entry points

- `tools/env_probe.py`: environment diagnostics
- `tools/run_native.py`: native CMake + PostgreSQL workflow for one PG version
- `tools/run_docker.py`: Docker compose workflow (single version or matrix)
- `tools/run_matrix.py`: convenience matrix runner

## Key properties

- uses `subprocess` with explicit argv vectors (`shell=False`)
- no `bash -lc` / login-shell assumptions
- optional `--clean-env` reduced environment mode
- logs command details and output to `logs/`
- stops on first failure by default (`--keep-going` available)
