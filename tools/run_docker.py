from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path
from typing import Dict, List

from common import base_env, print_env_header, repo_root, run_step


def require_docker(env: Dict[str, str]) -> None:
    docker = shutil.which("docker", path=env.get("PATH"))
    if not docker:
        raise FileNotFoundError("docker executable not found in PATH")


def main() -> int:
    parser = argparse.ArgumentParser(description="Run docker compose pg-dtw matrix without shell init assumptions.")
    parser.add_argument("--pg-version", choices=["15", "16", "17"], help="Run only a single compose service.")
    parser.add_argument("--skip-build", action="store_true", help="Skip docker compose build.")
    parser.add_argument("--clean-env", action="store_true")
    parser.add_argument("--keep-going", action="store_true")
    parser.add_argument("--log-dir", default="logs/docker")
    args = parser.parse_args()

    env = base_env(clean_env=args.clean_env)
    print_env_header(env)
    require_docker(env)
    root = repo_root()
    log_dir = root / args.log_dir

    services: List[str] = [f"pg{args.pg_version}"] if args.pg_version else ["pg15", "pg16", "pg17"]
    steps: List[List[str]] = []
    if not args.skip_build:
        steps.append(["docker", "compose", "build", *services])
    for svc in services:
        steps.append(["docker", "compose", "run", "--rm", svc])

    failures = 0
    for idx, argv in enumerate(steps, start=1):
        try:
            run_step(argv, cwd=root, env=env, log_dir=log_dir, step_name=f"{idx:02d}_{argv[2] if len(argv) > 2 else argv[0]}")
        except Exception as exc:
            failures += 1
            print(f"ERROR: {exc}")
            if not args.keep_going:
                return 1
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
