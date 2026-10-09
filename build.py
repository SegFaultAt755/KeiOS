# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import logging
import platform
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("KeiOS-Builder")


def get_make_command() -> str:
    if platform.system() == "Windows":
        for command in ["make", "mingw32-make", "muke.bat"]:
            if shutil.which(command):
                return command
    return "make"


def clean_build() -> int:
    logger.info("Executing clean build pipeline...")
    clean_script = ROOT / "clean.py"
    try:
        subprocess.run([sys.executable, str(clean_script)], cwd=ROOT, check=True)
    except subprocess.CalledProcessError as error:
        logger.error("Clean step failed with exit code %s", error.returncode)
        return error.returncode
    return 0


def build(target: str, make_args: list[str]) -> int:
    make_command = get_make_command()
    has_jobs_flag = any(
        argument == "-j"
        or argument.startswith("-j")
        or argument == "--jobs"
        or argument.startswith("--jobs=")
        for argument in make_args
    )
    command = [make_command, target] + ([] if has_jobs_flag else ["-j"]) + make_args
    logger.info("Building project: %s", " ".join(command))
    try:
        subprocess.run(command, cwd=ROOT, check=True)
    except FileNotFoundError:
        logger.critical("Build tool '%s' not found in PATH.", make_command)
        return 1
    except subprocess.CalledProcessError as error:
        logger.error("Build failed with exit code %s", error.returncode)
        return error.returncode
    return 0


def parse_arguments() -> tuple[argparse.Namespace, list[str]]:
    parser = argparse.ArgumentParser(description="Build KeiOS.")
    parser.add_argument("--make-target", default="all", help="Primary Makefile target")
    parser.add_argument(
        "--make-args",
        nargs=argparse.REMAINDER,
        default=[],
        help="Additional arguments passed to Make (or pass them inline)",
    )
    parser.add_argument(
        "--clean-build", action="store_true", help="Run clean.py before compiling"
    )
    args, inline_make_args = parser.parse_known_args()
    return args, inline_make_args + args.make_args


def main() -> int:
    args, make_args = parse_arguments()

    if args.clean_build:
        result = clean_build()
        if result:
            return result

    return build(args.make_target, make_args)


if __name__ == "__main__":
    raise SystemExit(main())
