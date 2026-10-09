# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

import argparse
import logging
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("KeiOS-Runner")


def build_project(target: str, clean: bool, make_args: list[str]) -> None:
    build_script = ROOT / "build.py"
    command = [sys.executable, str(build_script), "--make-target", target]
    if clean:
        command.append("--clean-build")
    command.extend(make_args)

    try:
        subprocess.run(command, cwd=ROOT, check=True)
    except FileNotFoundError:
        logger.critical("Python interpreter '%s' could not be run.", sys.executable)
        sys.exit(1)
    except subprocess.CalledProcessError as error:
        sys.exit(error.returncode)


class QemuRunner:
    def __init__(self, args: argparse.Namespace):
        self.args = args
        self.qemu_bin = f"qemu-system-{args.arch}"

    def ensure_disk(self) -> None:
        # Create the QCOW2 virtual hard drive
        if self.args.disk_file.is_file():
            return

        logger.info(
            f"Creating virtual disk: {self.args.disk_file} ({self.args.disk_size})"
        )
        try:
            subprocess.run(
                [
                    "qemu-img",
                    "create",
                    "-f",
                    "qcow2",
                    str(self.args.disk_file),
                    self.args.disk_size,
                ],
                check=True,
                capture_output=True,
                cwd=ROOT,
            )
        except FileNotFoundError:
            logger.critical(
                "Utility 'qemu-img' not found. Ensure QEMU is installed and in PATH."
            )
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            logger.error("Failed to create disk image: %s", err.stderr.decode().strip())
            sys.exit(err.returncode)

    def launch(self) -> None:
        cmd = [
            self.qemu_bin,
            "-cpu",
            self.args.cpu,
            "-m",
            self.args.memory,
            "-machine",
            "pc",
            "-rtc",
            "base=localtime",
            "-hda",
            str(self.args.disk_file),
            "-cdrom",
            str(self.args.iso_file),
            "-net",
            "nic,model=rtl8139",
            "-net",
            "user",
            "-device",
            "intel-hda",
            "-device",
            "hda-duplex",
            "-d",
            "int,cpu_reset",
            "-D",
            "qemu.log",
            "-debugcon",
            "file:debug.log",
        ]

        if self.args.headless:
            cmd.extend(["-display", "none", "-serial", "stdio"])
        else:
            cmd.extend(["-vga", "std"])

        logger.info("Launching virtual machine (%s)...", self.qemu_bin)
        try:
            subprocess.run(cmd, check=True, cwd=ROOT)
        except KeyboardInterrupt:
            logger.info("QEMU terminated by user.")
        except FileNotFoundError:
            logger.critical("QEMU binary '%s' not found.", self.qemu_bin)
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            logger.error("QEMU execution terminated with code %s", err.returncode)
            sys.exit(err.returncode)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run KeiOS in QEMU, building it when needed."
    )
    parser.add_argument(
        "--disk-file",
        type=Path,
        default=ROOT / "disk.qcow2",
        help="Path to QEMU disk image",
    )
    parser.add_argument(
        "--iso-file", type=Path, default=ROOT / "keios.iso", help="Path to bootable ISO"
    )
    parser.add_argument(
        "--disk-size", default="8G", help="Virtual disk capacity (e.g., 8G, 512M)"
    )
    parser.add_argument(
        "--arch", default="i386", help="Target architecture (i386, x86_64)"
    )
    parser.add_argument("--cpu", default="n270", help="Emulated CPU model")
    parser.add_argument("--memory", default="4G", help="RAM allocation")
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
    parser.add_argument(
        "--skip-build", action="store_true", help="Bypass compilation step"
    )
    parser.add_argument("--skip-run", action="store_true", help="Bypass QEMU execution")
    parser.add_argument(
        "--headless", action="store_true", help="Run QEMU without GUI (ideal for CI/CD)"
    )
    args, inline_make_args = parser.parse_known_args()
    args.make_args = inline_make_args + args.make_args
    return args


def main() -> None:
    args = parse_arguments()

    if not args.skip_build:
        build_project(args.make_target, args.clean_build, args.make_args)
    else:
        logger.info("Skipping build phase (--skip-build active).")

    if args.skip_run:
        logger.info("Build complete. Exiting (--skip-run active).")
        return

    runner = QemuRunner(args)
    runner.ensure_disk()
    runner.launch()


if __name__ == "__main__":
    main()
