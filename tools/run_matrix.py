from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List


ROOT = Path(__file__).resolve().parent.parent


def run(argv: List[str]) -> int:
    print(f"-> {argv}")
    return subprocess.run(argv, cwd=str(ROOT), check=False).returncode


def main() -> int:
    parser = argparse.ArgumentParser(description="Top-level matrix runner for native and docker workflows.")
    sub = parser.add_subparsers(dest="mode", required=True)

    native = sub.add_parser("native")
    native.add_argument("--clean-env", action="store_true")
    native.add_argument("--keep-going", action="store_true")
    native.add_argument("--skip-install", action="store_true")
    native.add_argument("--skip-regress", action="store_true")
    native.add_argument("--skip-pgtap", action="store_true")
    native.add_argument("--skip-fuzz", action="store_true")

    docker = sub.add_parser("docker")
    docker.add_argument("--clean-env", action="store_true")
    docker.add_argument("--keep-going", action="store_true")
    docker.add_argument("--skip-build", action="store_true")

    args = parser.parse_args()

    if args.mode == "native":
        failures = 0
        for ver in ("15", "16", "17"):
            cmd = [sys.executable, str(ROOT / "tools" / "run_native.py"), "--pg-version", ver]
            if args.clean_env:
                cmd.append("--clean-env")
            if args.keep_going:
                cmd.append("--keep-going")
            if args.skip_install:
                cmd.append("--skip-install")
            if args.skip_regress:
                cmd.append("--skip-regress")
            if args.skip_pgtap:
                cmd.append("--skip-pgtap")
            if args.skip_fuzz:
                cmd.append("--skip-fuzz")
            rc = run(cmd)
            if rc != 0:
                failures += 1
                if not args.keep_going:
                    return rc
        return 1 if failures else 0

    cmd = [sys.executable, str(ROOT / "tools" / "run_docker.py")]
    if args.clean_env:
        cmd.append("--clean-env")
    if args.keep_going:
        cmd.append("--keep-going")
    if args.skip_build:
        cmd.append("--skip-build")
    return run(cmd)


if __name__ == "__main__":
    sys.exit(main())
