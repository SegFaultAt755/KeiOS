#!/usr/bin/env python3
# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RUST_MANIFEST = ROOT / "rust" / "Cargo.toml"
FORMATTABLE_EXTENSIONS = {".c", ".h", ".rs"}


def tracked_source_files() -> list[str]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [
        path.decode()
        for path in result.stdout.split(b"\0")
        if path and Path(path.decode()).suffix in FORMATTABLE_EXTENSIONS
    ]


def staged_source_files() -> list[str]:
    result = subprocess.run(
        ["git", "diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [
        path.decode()
        for path in result.stdout.split(b"\0")
        if path and Path(path.decode()).suffix in FORMATTABLE_EXTENSIONS
    ]


def format_files(files: list[str]) -> int:
    rust_files = [path for path in files if Path(path).suffix == ".rs"]
    c_files = [path for path in files if Path(path).suffix in {".c", ".h"}]

    if rust_files:
        if not shutil.which("cargo"):
            print("error: cargo is required to format Rust files", file=sys.stderr)
            return 1
        subprocess.run(
            ["cargo", "fmt", "--all", "--manifest-path", str(RUST_MANIFEST)],
            cwd=ROOT,
            check=True,
        )

    if c_files:
        if not shutil.which("clang-format"):
            print("error: clang-format is required to format C files", file=sys.stderr)
            return 1
        subprocess.run(
            ["clang-format", "-i", "--", *c_files],
            cwd=ROOT,
            check=True,
        )

    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Format KeiOS Rust and C source files."
    )
    parser.add_argument(
        "--staged",
        action="store_true",
        help="format only staged Rust, C, and header files",
    )
    parser.add_argument(
        "files",
        nargs="*",
        help="format only these repository-relative source files",
    )
    args = parser.parse_args()

    if args.staged and args.files:
        parser.error("pass either --staged or file paths, not both")

    files = (
        staged_source_files()
        if args.staged
        else args.files if args.files else tracked_source_files()
    )
    files = sorted(set(files))
    if not files:
        print("No source files to format.")
        return 0

    result = format_files(files)
    if result:
        return result

    if args.staged:
        check = subprocess.run(
            ["git", "diff", "--quiet", "--", *files],
            cwd=ROOT,
            check=False,
        )
        if check.returncode == 1:
            print(
                "Formatting changed staged files. Review and stage the formatted "
                "files, then commit again.",
                file=sys.stderr,
            )
            return 1
        if check.returncode:
            print("error: could not check formatted files", file=sys.stderr)
            return check.returncode

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except FileNotFoundError as error:
        print(f"error: required command not found: {error.filename}", file=sys.stderr)
        raise SystemExit(1) from error
    except subprocess.CalledProcessError as error:
        print(
            f"error: formatter command failed with exit code {error.returncode}",
            file=sys.stderr,
        )
        raise SystemExit(error.returncode) from error
