#!/usr/bin/env bash
set -euo pipefail

PG_MAJOR="${1:?postgres major required}"
BUILD_DIR="build-pg${PG_MAJOR}"
DATA_DIR="/tmp/pg_dtw_data"
export PGUSER="${PGUSER:-postgres}"
export PGDATABASE="${PGDATABASE:-postgres}"
export PGHOST="${PGHOST:-/var/run/postgresql}"
export PGPORT="${PGPORT:-5432}"

cleanup() {
  if [ -d "${DATA_DIR}" ]; then
    su - postgres -c "/usr/lib/postgresql/${PG_MAJOR}/bin/pg_ctl -D ${DATA_DIR} -m fast stop" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

echo "PostgreSQL test connection env:"
echo "  PGUSER=${PGUSER}"
echo "  PGDATABASE=${PGDATABASE}"
echo "  PGHOST=${PGHOST}"
echo "  PGPORT=${PGPORT}"

cmake -S . -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"
cmake --install "${BUILD_DIR}"

rm -rf "${DATA_DIR}"
su - postgres -c "/usr/lib/postgresql/${PG_MAJOR}/bin/initdb -D ${DATA_DIR}"
su - postgres -c "/usr/lib/postgresql/${PG_MAJOR}/bin/pg_ctl -D ${DATA_DIR} -o \"-c listen_addresses=''\" -w start"

psql -v ON_ERROR_STOP=1 -c "CREATE EXTENSION IF NOT EXISTS pg_dtw;"
psql -v ON_ERROR_STOP=1 -c "CREATE EXTENSION IF NOT EXISTS pgtap;"
chmod +x /work/test/run_pg_regress.sh
/work/test/run_pg_regress.sh "${PG_MAJOR}"
echo "Running pg_prove against ${PGHOST}:${PGPORT}/${PGDATABASE} as ${PGUSER}"
pg_prove --dbname="${PGDATABASE}" --host="${PGHOST}" --port="${PGPORT}" --username="${PGUSER}" /work/test/pgtap/pg_dtw_api.sql

"${BUILD_DIR}/pg_dtw_fuzz"
