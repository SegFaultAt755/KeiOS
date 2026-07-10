# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import platform
import shutil
import subprocess
import sys
from pathlib import Path

def get_make_command() -> str:
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd
    return "make"

def main() -> None:
    parser = argparse.ArgumentParser(description="Build the project and run it inside a QEMU virtual machine.")
    parser.add_argument("--disk-file", type=Path, default=Path("disk.qcow2"), help="Path to the QEMU disk image (default: disk.qcow2)")
    parser.add_argument("--iso-file", type=Path, default=Path("keios.iso"), help="Path to the system ISO file (default: keios.iso)")
    parser.add_argument("--disk-size", default="8G", help="Size of the disk image if created (default: 8G)")
    parser.add_argument("--arch", default="i386", help="QEMU architecture target (default: i386)")
    parser.add_argument("--cpu", default="n270", help="QEMU CPU model (default: n270)")
    parser.add_argument("--memory", default="4G", help="RAM allocated to QEMU (default: 4G)")
    parser.add_argument("--make-target", default="all", help="Makefile target to run (default: all)")
    parser.add_argument("--skip-build", action="store_true", help="Skip the compilation step")
    parser.add_argument("--skip-run", action="store_true", help="Skip launching QEMU after building")
    args = parser.parse_args()

    # Check if disk exists, otherwise create it
    if not args.disk_file.is_file():
        print(f">>> Creating {args.disk_file} ({args.disk_size})...")
        try:
            subprocess.run(
                ["qemu-img", "create", "-f", "qcow2", str(args.disk_file), args.disk_size],
                check=True
            )
        except FileNotFoundError:
            print(
                "Error: 'qemu-img' not found. Ensure QEMU is installed and added to your PATH.",
                file=sys.stderr
            )
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            print(f"Error creating disk image: {err}", file=sys.stderr)
            sys.exit(err.returncode)

    # Run the makefile
    if not args.skip_build:
        make_cmd = get_make_command()
        print(f">>> Building project with '{make_cmd} {args.make_target}'...")
        try:
            subprocess.run([make_cmd, args.make_target], check=True)
        except FileNotFoundError:
            print(
                f"Error: '{make_cmd}' command not found. Please install Make and add it to your PATH.",
                file=sys.stderr
            )
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            print(f"Build failed: {err}", file=sys.stderr)
            sys.exit(err.returncode)
    else:
        print(">>> Skipping build step...")

    # Exit early if only building
    if args.skip_run:
        print(">>> Build finished (--skip-run is True). Exiting.")
        return

    # Launch QEMU
    qemu_executable = f"qemu-system-{args.arch}"
    qemu_cmd = [
        qemu_executable,
        "-cpu", args.cpu,
        "-m", args.memory,
        "-machine", "pc",
        "-rtc", "base=localtime",
        "-vga", "std",
        "-hda", str(args.disk_file),
        "-net", "nic,model=rtl8139", "-net", "user",
        "-device", "intel-hda",
        "-device", "hda-duplex",
        "-d", "int,cpu_reset", "-D", "qemu.log",
        "-cdrom", str(args.iso_file),
        "-debugcon", "file:debug.log"
    ]

    print(f">>> Starting {qemu_executable}...")
    try:
        subprocess.run(qemu_cmd, check=True)
    except FileNotFoundError:
        print(
            f"Error: '{qemu_executable}' not found. Ensure QEMU is installed and added to your PATH.",
            file=sys.stderr
        )
        sys.exit(1)
    except subprocess.CalledProcessError as err:
        print(f"QEMU exited with error: {err}", file=sys.stderr)
        sys.exit(err.returncode)

if __name__ == "__main__":
    main()
