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
logger = logging.getLogger("KeiOS-Runner")


class ToolchainManager:
    @staticmethod
    def get_make_command() -> str:
        if platform.system() == "Windows":
            for cmd in ["make", "mingw32-make", "muke.bat"]:
                if shutil.which(cmd):
                    return cmd
        return "make"

    @classmethod
    def clean_build(cls) -> None:
        logger.info("Executing clean build pipeline...")
        clean_script = Path(__file__).parent / "clean.py"
        try:
            subprocess.run([sys.executable, str(clean_script)], check=True)
        except subprocess.CalledProcessError as err:
            logger.error(f"Clean step failed with exit code {err.returncode}")
            sys.exit(err.returncode)

    @classmethod
    def build(cls, target: str, make_args: list[str]) -> None:
        make_cmd = cls.get_make_command()
        cmd = [make_cmd, target] + make_args
        logger.info(f"Building project: {' '.join(cmd)}")
        try:
            subprocess.run(cmd, check=True)
        except FileNotFoundError:
            logger.critical(f"Build tool '{make_cmd}' not found in PATH.")
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            logger.error(f"Build failed with exit code {err.returncode}")
            sys.exit(err.returncode)


class QemuRunner:
    def __init__(self, args: argparse.Namespace):
        self.args = args
        self.qemu_bin = f"qemu-system-{args.arch}"

    def ensure_disk(self) -> None:
        # Create qcow2 virtual hard drive
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
            )
        except FileNotFoundError:
            logger.critical(
                "Utility 'qemu-img' not found. Ensure QEMU is installed and in PATH."
            )
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            logger.error(f"Failed to create disk image: {err.stderr.decode().strip()}")
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

        logger.info(f"Launching virtual machine ({self.qemu_bin})...")
        try:
            subprocess.run(cmd, check=True)
        except KeyboardInterrupt:
            logger.info("QEMU terminated by user.")
        except FileNotFoundError:
            logger.critical(f"QEMU binary '{self.qemu_bin}' not found.")
            sys.exit(1)
        except subprocess.CalledProcessError as err:
            logger.error(f"QEMU execution terminated with code {err.returncode}")
            sys.exit(err.returncode)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build and Execution Manager for KeiOS."
    )
    parser.add_argument(
        "--disk-file",
        type=Path,
        default=Path("disk.qcow2"),
        help="Path to QEMU disk image",
    )
    parser.add_argument(
        "--iso-file", type=Path, default=Path("keios.iso"), help="Path to bootable ISO"
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
        help="Additional arguments passed to Make (e.g., --make-args D=DEBUG)",
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
    return parser.parse_args()


def main() -> None:
    args = parse_arguments()

    if args.clean_build and not args.skip_build:
        ToolchainManager.clean_build()

    if not args.skip_build:
        ToolchainManager.build(args.make_target, args.make_args)
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
