#!/usr/bin/env bash
set -euo pipefail

PG_MAJOR="${1:?postgres major required}"
PG_REGRESS="/usr/lib/postgresql/${PG_MAJOR}/lib/pgxs/src/test/regress/pg_regress"
export PGUSER="${PGUSER:-postgres}"
export PGDATABASE="${PGDATABASE:-postgres}"
export PGHOST="${PGHOST:-/var/run/postgresql}"
export PGPORT="${PGPORT:-5432}"

echo "Running pg_regress with:"
echo "  PGUSER=${PGUSER}"
echo "  PGDATABASE=${PGDATABASE}"
echo "  PGHOST=${PGHOST}"
echo "  PGPORT=${PGPORT}"

"${PG_REGRESS}" \
  --inputdir=/work/test \
  --bindir="/usr/lib/postgresql/${PG_MAJOR}/bin" \
  --dbname="${PGDATABASE}" \
  --host="${PGHOST}" \
  --port="${PGPORT}" \
  --user="${PGUSER}" \
  --use-existing \
  --load-extension=pg_dtw \
  pg_dtw_core
