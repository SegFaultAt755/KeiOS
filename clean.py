import subprocess
import sys
import platform
import shutil
from pathlib import Path

def get_make_command():
    if platform.system() == "Windows":
        for cmd in ["make", "mingw32-make"]:
            if shutil.which(cmd):
                return cmd

    return "make"

def main():
    # Run the make clean command
    make_cmd = get_make_command()
    print(f">>> Running '{make_cmd} clean'...")
    try:
        subprocess.run([make_cmd, "clean"], check=True)
    except FileNotFoundError:
        print(f"Error: '{make_cmd}' command not found. Skipping 'make clean'. Ensure Make is in your PATH if needed.", file=sys.stderr)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"'{make_cmd} clean' failed: {e}", file=sys.stderr)
        sys.exit(e.returncode)

    # Remove disk.qcow2
    disk_file = Path("disk.qcow2")
    print(f">>> Removing {disk_file}...")
    try:
        disk_file.unlink(missing_ok=False)
    except FileNotFoundError:
        print(f"Error: Cannot remove '{disk_file}' because it does not exist.", file=sys.stderr)
        sys.exit(1)
    except PermissionError:
        print(f"Error: Cannot remove '{disk_file}'. The file is currently locked or open by another process.", file=sys.stderr)
        sys.exit(1)
    except OSError as e:
        print(f"Error removing file: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
