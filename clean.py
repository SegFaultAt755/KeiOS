# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import logging
import platform
import shutil
import subprocess
import sys
from pathlib import Path

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("KeiOS-Cleaner")

# Never delete these even if matched by gitignore patterns
PROTECTED_EXTENSIONS = {".md", ".txt", ".cfg", ".py", ".c", ".asm", ".h", ".ld", ".mk"}
PROTECTED_DIRS = {".obsidian", ".vscode", ".git", "src", "include", "rootfs"}


def get_make_command() -> str:
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd
    return "make"


def parse_gitignore_patterns(gitignore_path: Path) -> list[str]:
    if not gitignore_path.is_file():
        return []

    patterns = []
    with open(gitignore_path, "r", encoding="utf-8") as f:
        for line in f:
            clean_line = line.strip()
            # Ignore empty lines, comments, and explicitly protected directories
            if (
                not clean_line
                or clean_line.startswith("#")
                or any(prot in clean_line for prot in PROTECTED_DIRS)
            ):
                continue
            patterns.append(clean_line.lstrip("/"))
    return patterns


def safe_delete(path: Path, dry_run: bool) -> None:
    if path.name in PROTECTED_DIRS or path.suffix in PROTECTED_EXTENSIONS:
        return

    if dry_run:
        logger.info(f"[DRY-RUN] Would remove: {path}")
        return

    try:
        if path.is_file() or path.is_symlink():
            path.unlink()
            logger.info(f"Removed file: {path}")
        elif path.is_dir():
            shutil.rmtree(path)
            logger.info(f"Removed directory: {path}")
    except PermissionError:
        logger.error(f"Permission denied: Cannot remove '{path}'. File may be in use.")
    except OSError as err:
        logger.error(f"Error removing '{path}': {err}")


def clean_gitignore_artifacts(workspace_root: Path, dry_run: bool) -> None:
    patterns = parse_gitignore_patterns(workspace_root / ".gitignore")
    logger.info(
        f"Scanning workspace using {len(patterns)} pattern rules from .gitignore..."
    )

    for pattern in patterns:
        matched_paths = list(workspace_root.glob(pattern)) + list(
            workspace_root.glob(f"**/{pattern}")
        )
        for path in set(matched_paths):
            if path.exists() and path != workspace_root:
                safe_delete(path, dry_run)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Safely clean build artifacts, disk images, and logs."
    )
    parser.add_argument(
        "--disk-file",
        type=Path,
        default=Path("disk.qcow2"),
        help="Path to virtual disk image",
    )
    parser.add_argument(
        "--skip-make", action="store_true", help="Skip executing 'make clean'"
    )
    parser.add_argument(
        "--skip-disk", action="store_true", help="Preserve the virtual disk file"
    )
    parser.add_argument("--skip-logs", action="store_true", help="Preserve log files")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview files to be deleted without removing them",
    )
    args = parser.parse_args()

    workspace_root = Path(__file__).parent.resolve()

    # Execute Makefile clean rule
    if not args.skip_make:
        make_cmd = get_make_command()
        logger.info(f"Executing '{make_cmd} clean'...")
        if not args.dry_run:
            try:
                subprocess.run([make_cmd, "clean"], check=True, capture_output=True)
            except FileNotFoundError:
                logger.warning(f"Command '{make_cmd}' not found. Skipping make clean.")
            except subprocess.CalledProcessError as err:
                logger.error(f"Make clean failed: {err.stderr.decode().strip()}")

    # Clean defined artifacts from .gitignore
    clean_gitignore_artifacts(workspace_root, args.dry_run)

    # Explicitly target disk and logs
    if not args.skip_disk and args.disk_file.exists():
        safe_delete(args.disk_file, args.dry_run)

    if not args.skip_logs:
        for log_file in ["debug.log", "qemu.log", "config.mk"]:
            log_path = workspace_root / log_file
            if log_path.exists():
                safe_delete(log_path, args.dry_run)

    logger.info("Cleanup operation completed.")


if __name__ == "__main__":
    main()
