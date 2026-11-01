# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import logging
import platform
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("KeiOS-Cleaner")

def get_make_command() -> str:
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd
    return "make"


def remove_file(path: Path, dry_run: bool) -> bool:
    if not path.exists() and not path.is_symlink():
        return True
    if dry_run:
        logger.info("[DRY-RUN] Would remove: %s", path)
        return True
    try:
        path.unlink()
        logger.info("Removed file: %s", path)
        return True
    except OSError as err:
        logger.error("Unable to remove '%s': %s", path, err)
        return False


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Safely clean build artifacts, disk images, and logs."
    )
    parser.add_argument(
        "--disk-file",
        type=Path,
        default=ROOT / "disk.qcow2",
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

    # Run the Makefile clean rule
    if not args.skip_make:
        make_cmd = get_make_command()
        command = [make_cmd, "clean"]
        if args.dry_run:
            command.insert(1, "-n")
        logger.info("Executing '%s'...", " ".join(command))
        try:
            result = subprocess.run(
                command,
                check=True,
                capture_output=True,
                text=True,
                cwd=ROOT,
            )
            if args.dry_run and result.stdout:
                logger.info("Make clean preview:\n%s", result.stdout.rstrip())
        except FileNotFoundError:
            logger.error("Command '%s' not found.", make_cmd)
            raise SystemExit(1)
        except subprocess.CalledProcessError as err:
            logger.error("Make clean failed: %s", err.stderr.strip())
            raise SystemExit(err.returncode)

    # Remove disk images and logs explicitly
    cleanup_succeeded = True
    if not args.skip_disk:
        cleanup_succeeded &= remove_file(args.disk_file, args.dry_run)

    if not args.skip_logs:
        for log_file in ["debug.log", "qemu.log"]:
            cleanup_succeeded &= remove_file(ROOT / log_file, args.dry_run)

    if not cleanup_succeeded:
        raise SystemExit(1)
    logger.info("Cleanup operation completed.")


if __name__ == "__main__":
    main()
