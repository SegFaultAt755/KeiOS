import subprocess
import sys
import platform
import shutil
from pathlib import Path

def get_make_command() -> str:
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd

    return "make"

def main() -> None:
    disk_file = Path("disk.qcow2")
    iso_file = Path("keios.iso")

    # Check if disk exists, otherwise create it
    if not disk_file.is_file():
        print(">>> Creating disk.qcow2...")
        try:
            subprocess.run(
                ["qemu-img", "create", "-f", "qcow2", str(disk_file), "8G"],
                check=True
            )
        except FileNotFoundError:
            print("Error: 'qemu-img' not found. Ensure QEMU is installed and added to your PATH.", file=sys.stderr)
            sys.exit(1)
        except subprocess.CalledProcessError as e:
            print(f"Error creating disk image: {e}", file=sys.stderr)
            sys.exit(e.returncode)

    # Run the makefile
    make_cmd = get_make_command()
    print(f">>> Building project with '{make_cmd} all'...")
    try:
        subprocess.run([make_cmd, "all"], check=True)
    except FileNotFoundError:
        print(f"Error: '{make_cmd}' command not found. Please install Make (e.g., via MinGW, MSYS2) and add it to your PATH.", file=sys.stderr)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}", file=sys.stderr)
        sys.exit(e.returncode)

    # Launch QEMU
    qemu_cmd = [
        "qemu-system-i386",
        "-cpu", "n270", "-m", "1G",
        "-machine", "pc",
        "-rtc", "base=localtime",
        "-vga", "std",
        "-hda", str(disk_file),
        "-net", "nic,model=rtl8139", "-net", "user",
        "-device", "intel-hda", "-device", "hda-duplex",
        "-d", "int,cpu_reset", "-D", "qemu.log",
        "-cdrom", str(iso_file),
        "-debugcon", "file:debug.log"
    ]

    print(">>> Starting QEMU...")
    try:
        subprocess.run(qemu_cmd, check=True)
    except FileNotFoundError:
        print("Error: 'qemu-system-i386' not found. Ensure QEMU is installed and added to your PATH.", file=sys.stderr)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"QEMU exited with error: {e}", file=sys.stderr)
        sys.exit(e.returncode)

if __name__ == "__main__":
    main()
