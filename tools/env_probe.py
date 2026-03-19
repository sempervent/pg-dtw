from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional

from common import base_env, print_env_header


def which(name: str, env: Dict[str, str]) -> Optional[str]:
    return shutil.which(name, path=env.get("PATH"))


def run_version(argv: List[str], env: Dict[str, str]) -> str:
    try:
        out = subprocess.run(argv, env=env, check=False, capture_output=True, text=True)
        text = (out.stdout or out.stderr).strip().splitlines()
        return text[0] if text else f"exit={out.returncode}"
    except Exception as exc:
        return f"error: {exc}"


def candidate_pg_configs() -> Dict[str, str]:
    out: Dict[str, str] = {}
    for ver in ("15", "16", "17"):
        p = Path(f"/usr/lib/postgresql/{ver}/bin/pg_config")
        if p.exists():
            out[ver] = str(p)
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description="Probe local execution environment for pg-dtw tooling.")
    parser.add_argument("--clean-env", action="store_true", help="Run probe with a reduced environment.")
    args = parser.parse_args()

    env = base_env(clean_env=args.clean_env)
    print_env_header(env)

    tools = [
        "python3",
        "cmake",
        "pg_config",
        "psql",
        "pg_prove",
        "docker",
    ]
    print("\nExecutable probe:")
    for t in tools:
        p = which(t, env)
        print(f"- {t}: {p or 'NOT FOUND'}")

    print("\nVersion probe:")
    if which("cmake", env):
        print(f"- cmake: {run_version(['cmake', '--version'], env)}")
    if which("pg_config", env):
        print(f"- pg_config: {run_version(['pg_config', '--version'], env)}")
    if which("psql", env):
        print(f"- psql: {run_version(['psql', '--version'], env)}")
    if which("pg_prove", env):
        print(f"- pg_prove: {run_version(['pg_prove', '--version'], env)}")
    if which("docker", env):
        print(f"- docker: {run_version(['docker', '--version'], env)}")
        print(f"- docker compose: {run_version(['docker', 'compose', 'version'], env)}")

    print("\nVersion-specific pg_config candidates:")
    cands = candidate_pg_configs()
    if not cands:
        print("- none found under /usr/lib/postgresql/{15,16,17}/bin")
    else:
        for ver, path in cands.items():
            print(f"- pg{ver}: {path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
