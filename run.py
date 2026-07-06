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

config = {
    "disk_file": Path("disk.qcow2"),
    "iso_file": Path("keios.iso"),
    "disk_size": "8G",
    "arch": "i386",
    "cpu": "n270",
    "memory": "4G",
    "make_target": "all",
    "skip_build": False,
    "skip_run": False,
}

def main() -> None:
    disk_file: Path = config["disk_file"]
    iso_file: Path = config["iso_file"]

    # Check if disk exists, otherwise create it
    if not disk_file.is_file():
        print(f">>> Creating {disk_file} ({config['disk_size']})...")
        try:
            subprocess.run(
                ["qemu-img", "create", "-f", "qcow2", str(disk_file), config["disk_size"]],
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

    # Run the makefile (unless skipped)
    if not config["skip_build"]:
        make_cmd = get_make_command()
        print(f">>> Building project with '{make_cmd} {config['make_target']}'...")
        try:
            subprocess.run([make_cmd, config["make_target"]], check=True)
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
    if config["skip_run"]:
        print(">>> Build finished (skip_run is True). Exiting.")
        return

    # Launch QEMU
    qemu_executable = f"qemu-system-{config["arch"]}"
    qemu_cmd = [
        qemu_executable,
        "-cpu", config["cpu"],
        "-m", config["memory"],
        "-machine", "pc",
        "-rtc", "base=localtime",
        "-vga", "std",
        "-hda", str(disk_file),
        "-net", "nic,model=rtl8139", "-net", "user",
        "-device", "intel-hda",
        "-device", "hda-duplex",
        "-d", "int,cpu_reset", "-D", "qemu.log",
        "-cdrom", str(iso_file),
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
