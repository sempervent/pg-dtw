from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

from common import base_env, run_step  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description="Run pg_regress against existing PostgreSQL instance.")
    parser.add_argument("--pg-version", required=True, choices=["15", "16", "17"])
    parser.add_argument("--clean-env", action="store_true")
    parser.add_argument("--log-dir", default="logs/regress")
    args = parser.parse_args()

    env = base_env(clean_env=args.clean_env)
    env.setdefault("PGUSER", "postgres")
    env.setdefault("PGDATABASE", "postgres")
    env.setdefault("PGHOST", "/var/run/postgresql")
    env.setdefault("PGPORT", "5432")
    print(f"PG connection env: user={env['PGUSER']} db={env['PGDATABASE']} host={env['PGHOST']} port={env['PGPORT']}")
    pg_regress = Path(f"/usr/lib/postgresql/{args.pg_version}/lib/pgxs/src/test/regress/pg_regress")
    bindir = Path(f"/usr/lib/postgresql/{args.pg_version}/bin")

    argv = [
        str(pg_regress),
        "--inputdir",
        str(ROOT / "test"),
        "--bindir",
        str(bindir),
        "--dbname",
        env["PGDATABASE"],
        "--host",
        env["PGHOST"],
        "--port",
        env["PGPORT"],
        "--user",
        env["PGUSER"],
        "--use-existing",
        "--load-extension",
        "pg_dtw",
        "pg_dtw_core",
    ]

    run_step(argv, cwd=ROOT, env=env, log_dir=ROOT / args.log_dir, step_name=f"pg_regress_pg{args.pg_version}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
