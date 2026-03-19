from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional

from common import base_env, print_env_header, repo_root, run_step


def detect_pg_config(pg_version: Optional[str], explicit: Optional[str], env: Dict[str, str]) -> str:
    if explicit:
        p = Path(explicit)
        if not p.exists():
            raise FileNotFoundError(f"--pg-config does not exist: {explicit}")
        return str(p)

    if pg_version:
        candidate = Path(f"/usr/lib/postgresql/{pg_version}/bin/pg_config")
        if candidate.exists():
            return str(candidate)

    discovered = shutil.which("pg_config", path=env.get("PATH"))
    if discovered:
        return discovered
    raise FileNotFoundError("pg_config not found; pass --pg-config explicitly.")


def pg_bindir(pg_config: str, env: Dict[str, str]) -> str:
    out = subprocess.run([pg_config, "--bindir"], env=env, check=True, capture_output=True, text=True)
    return out.stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser(description="Run native pg-dtw build/test flow without shell-init dependency.")
    parser.add_argument("--pg-version", choices=["15", "16", "17"], help="Target PostgreSQL major version.")
    parser.add_argument("--pg-config", help="Explicit pg_config path.")
    parser.add_argument("--build-dir", help="Build directory (default build-pg<version>).")
    parser.add_argument("--clean-env", action="store_true", help="Use minimal environment.")
    parser.add_argument("--keep-going", action="store_true", help="Continue after step failures.")
    parser.add_argument("--log-dir", default="logs/native", help="Log directory.")
    parser.add_argument("--skip-install", action="store_true")
    parser.add_argument("--skip-regress", action="store_true")
    parser.add_argument("--skip-pgtap", action="store_true")
    parser.add_argument("--skip-fuzz", action="store_true")
    args = parser.parse_args()

    env = base_env(clean_env=args.clean_env)
    print_env_header(env)
    root = repo_root()
    log_dir = root / args.log_dir
    pg_config = detect_pg_config(args.pg_version, args.pg_config, env)
    env["PG_CONFIG"] = pg_config
    env.setdefault("PGUSER", "postgres")
    env.setdefault("PGDATABASE", "postgres")
    env.setdefault("PGHOST", "/var/run/postgresql")
    env.setdefault("PGPORT", "5432")
    print(f"- resolved PG_CONFIG: {pg_config}")
    print(f"- PGUSER={env['PGUSER']} PGDATABASE={env['PGDATABASE']} PGHOST={env['PGHOST']} PGPORT={env['PGPORT']}")

    build_dir = args.build_dir or f"build-pg{args.pg_version or 'local'}"
    build_path = root / build_dir
    bindir = pg_bindir(pg_config, env)
    psql = str(Path(bindir) / "psql")
    pg_prove = shutil.which("pg_prove", path=env.get("PATH")) or "pg_prove"

    steps: List[List[str]] = [
        ["cmake", "-S", str(root), "-B", str(build_path)],
        ["cmake", "--build", str(build_path), "-j"],
    ]
    if not args.skip_install:
        steps.append(["cmake", "--install", str(build_path)])
    if not args.skip_regress:
        if not args.pg_version:
            raise ValueError("--pg-version is required when running pg_regress via this harness.")
        steps.append([sys.executable, str(root / "test" / "run_pg_regress.py"), "--pg-version", args.pg_version])
    if not args.skip_pgtap:
        steps.append([
            pg_prove,
            "--dbname",
            env["PGDATABASE"],
            "--host",
            env["PGHOST"],
            "--port",
            env["PGPORT"],
            "--username",
            env["PGUSER"],
            str(root / "test" / "pgtap" / "pg_dtw_api.sql"),
        ])
    if not args.skip_fuzz:
        steps.append([str(build_path / "pg_dtw_fuzz")])

    # Ensure extension can be created before tests.
    steps.insert(
        3 if not args.skip_install else 2,
        [psql, "-h", env["PGHOST"], "-p", env["PGPORT"], "-U", env["PGUSER"], "-d", env["PGDATABASE"], "-c", "CREATE EXTENSION IF NOT EXISTS pg_dtw;"],
    )
    steps.insert(
        (4 if not args.skip_install else 3),
        [psql, "-h", env["PGHOST"], "-p", env["PGPORT"], "-U", env["PGUSER"], "-d", env["PGDATABASE"], "-c", "CREATE EXTENSION IF NOT EXISTS pgtap;"],
    )

    failures = 0
    for idx, argv in enumerate(steps, start=1):
        try:
            run_step(argv, cwd=root, env=env, log_dir=log_dir, step_name=f"{idx:02d}_{Path(argv[0]).name}")
        except Exception as exc:
            failures += 1
            print(f"ERROR: {exc}")
            if not args.keep_going:
                return 1
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
