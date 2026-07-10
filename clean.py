# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import platform
import shutil
import subprocess
import sys
from pathlib import Path

def get_make_command():
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd
    return "make"

def main():
    parser = argparse.ArgumentParser(description="Clean build artifacts, disk images, and logs.")
    parser.add_argument("--disk-file", type=Path, default=Path("disk.qcow2"), help="Path to the QEMU disk image to remove")
    parser.add_argument("--skip-make", action="store_true", help="Skip running 'make clean'")
    parser.add_argument("--skip-disk", action="store_true", help="Skip removing the virtual disk file")
    parser.add_argument("--skip-logs", action="store_true", help="Skip removing log files")
    
    args = parser.parse_args()

    if not args.skip_make:
        make_cmd = get_make_command()
        print(f">>> Running '{make_cmd} clean'...")
        try:
            subprocess.run([make_cmd, "clean"], check=True)
        except FileNotFoundError:
            print(f"Error: '{make_cmd}' command not found. Skipping 'make clean'.", file=sys.stderr)
        except subprocess.CalledProcessError as e:
            print(f"'{make_cmd} clean' failed: {e}", file=sys.stderr)
            sys.exit(e.returncode)

    # Remove virtual disk
    if not args.skip_disk:
        print(f">>> Removing {args.disk_file}...")
        try:
            args.disk_file.unlink(missing_ok=False)
        except FileNotFoundError:
            print(f"Warning: '{args.disk_file}' does not exist. Skipping.", file=sys.stderr)
        except PermissionError:
            print(f"Error: Cannot remove '{args.disk_file}'. The file is locked or open.", file=sys.stderr)
            sys.exit(1)
        except OSError as e:
            print(f"Error removing file: {e}", file=sys.stderr)
            sys.exit(1)

    # Remove logs
    if not args.skip_logs:
        log_files = [Path("debug.log"), Path("qemu.log")]
        for log in log_files:
            print(f">>> Removing {log}...")
            try:
                log.unlink(missing_ok=False)
            except FileNotFoundError:
                print(f"Warning: '{log}' does not exist. Skipping.", file=sys.stderr)
            except PermissionError:
                print(f"Error: Cannot remove '{log}'. The file is locked by another process.", file=sys.stderr)
                sys.exit(1)
            except OSError as e:
                print(f"Error removing log: {e}", file=sys.stderr)
                sys.exit(1)

if __name__ == "__main__":
    main()
