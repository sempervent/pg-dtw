from __future__ import annotations

import os
import platform
import shlex
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional


@dataclass
class StepResult:
    argv: List[str]
    exit_code: int
    elapsed_sec: float
    log_file: Optional[Path]


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def pretty_argv(argv: Iterable[str]) -> str:
    return " ".join(shlex.quote(x) for x in argv)


def base_env(clean_env: bool = False, extra_env: Optional[Dict[str, str]] = None) -> Dict[str, str]:
    if clean_env:
        env = {
            "PATH": os.environ.get("PATH", "/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin"),
            "HOME": os.environ.get("HOME", ""),
            "LANG": os.environ.get("LANG", "C.UTF-8"),
            "LC_ALL": os.environ.get("LC_ALL", "C.UTF-8"),
        }
    else:
        env = dict(os.environ)
    if extra_env:
        env.update(extra_env)
    return env


def run_step(
    argv: List[str],
    *,
    cwd: Optional[Path] = None,
    env: Optional[Dict[str, str]] = None,
    log_dir: Optional[Path] = None,
    step_name: Optional[str] = None,
    check: bool = True,
) -> StepResult:
    cwd = cwd or repo_root()
    log_file: Optional[Path] = None
    log_handle = None
    if log_dir is not None:
        log_dir.mkdir(parents=True, exist_ok=True)
        safe_name = (step_name or "step").replace("/", "_").replace(" ", "_")
        log_file = log_dir / f"{safe_name}.log"
        log_handle = log_file.open("w", encoding="utf-8")

    print(f"\n==> STEP: {step_name or 'command'}")
    print(f"cwd: {cwd}")
    print(f"argv: {argv!r}")
    print(f"cmd : {pretty_argv(argv)}")
    start = time.time()

    proc = subprocess.Popen(
        argv,
        cwd=str(cwd),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    assert proc.stdout is not None
    for line in proc.stdout:
        sys.stdout.write(line)
        if log_handle:
            log_handle.write(line)
    exit_code = proc.wait()
    elapsed = time.time() - start

    if log_handle:
        log_handle.flush()
        log_handle.close()

    print(f"<== exit={exit_code} elapsed={elapsed:.2f}s")
    if log_file:
        print(f"    log={log_file}")

    if check and exit_code != 0:
        raise RuntimeError(f"step failed ({step_name or argv[0]}), exit={exit_code}")

    return StepResult(argv=argv, exit_code=exit_code, elapsed_sec=elapsed, log_file=log_file)


def print_env_header(env: Dict[str, str]) -> None:
    print("Environment summary:")
    print(f"- python: {sys.version.split()[0]}")
    print(f"- platform: {platform.platform()}")
    print(f"- PATH: {env.get('PATH', '')}")
