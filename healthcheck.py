# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import re
import subprocess
import sys
from enum import Enum
from pathlib import Path

ROOT = Path(__file__).resolve().parent
README = ROOT / "README.md"
BADGE_MARKERS = re.compile(
    r"(?ms)^<!-- healthcheck:badges:start -->\n.*?\n"
    r"<!-- healthcheck:badges:end -->$"
)


class CheckStatus(Enum):
    PASSING = ("Passing", "brightgreen")
    WARNINGS = ("Warnings", "yellow")
    FAILING = ("Failing", "red")
    UNDEFINED = ("Undefined", "black")


def run_check(name: str, command: list[str], cwd: Path) -> CheckStatus:
    print(f"\n==> {' '.join(command)} (in {cwd.relative_to(ROOT) or '.'})", flush=True)
    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
    except FileNotFoundError as error:
        print(f"UNDEFINED: Could not run {name}: {error}", file=sys.stderr)
        return CheckStatus.UNDEFINED
    except OSError as error:
        print(f"ERROR: Could not run {name}: {error}", file=sys.stderr)
        return CheckStatus.FAILING

    if result.stdout:
        print(result.stdout, end="" if result.stdout.endswith("\n") else "\n")

    if result.returncode:
        if re.search(
            r"\bnot found\b|can't find crate|cannot find crate",
            result.stdout,
            re.I,
        ):
            print(f"UNDEFINED: {name} could not find a required tool or crate")
            return CheckStatus.UNDEFINED
        print(f"FAIL: {name} exited with status {result.returncode}", file=sys.stderr)
        return CheckStatus.FAILING

    if name == "cargo clippy" and re.search(r"^warning:", result.stdout, re.M):
        print(f"HAS WARNINGS: {name}")
        return CheckStatus.WARNINGS

    print(f"PASS: {name}")
    return CheckStatus.PASSING


def badge(name: str, status: CheckStatus) -> str:
    label, color = status.value
    return f"![{name}](https://img.shields.io/badge/{name}-{label}-{color})"


def update_readme_badges(results: dict[str, CheckStatus]) -> None:
    content = README.read_text(encoding="utf-8")
    if not BADGE_MARKERS.search(content):
        raise RuntimeError(f"Could not find healthcheck badge markers in {README}")

    badges = " ".join(
        [
            badge("Clippy", results["Clippy"]),
            badge("Tests", results["Tests"]),
            badge("Build", results["Build"]),
        ]
    )
    updated = BADGE_MARKERS.sub(
        f"<!-- healthcheck:badges:start -->\n{badges}\n"
        "<!-- healthcheck:badges:end -->",
        content,
        count=1,
    )
    README.write_text(updated, encoding="utf-8")


def main() -> int:
    results = {
        "Clippy": run_check(
            "cargo clippy",
            [
                "cargo",
                "clippy",
                "--",
                "-A",
                "clippy::all",
                "-D",
                "clippy::correctness",
            ],
            ROOT / "rust",
        ),
        "Tests": run_check("cargo test", ["cargo", "test"], ROOT / "rust"),
        "Build": run_check(
            "python3 run.py --skip-run",
            ["python3", "run.py", "--skip-run"],
            ROOT,
        ),
    }

    try:
        update_readme_badges(results)
    except (OSError, RuntimeError) as error:
        print(f"ERROR: Could not update README badges: {error}", file=sys.stderr)
        return 1

    print("\nUpdated README badges:")
    print(badge("Clippy", results["Clippy"]))
    print(badge("Tests", results["Tests"]))
    print(badge("Build", results["Build"]))
    all_checks_ok = all(
        status in (CheckStatus.PASSING, CheckStatus.WARNINGS)
        for status in results.values()
    )
    return 0 if all_checks_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
