# System Call Interface

## Current state

The kernel installs a software interrupt gate at vector `0x80` and dispatches
the syscall number in `eax`. The current table contains indices 0–5. The
register interface is a kernel implementation detail under development; it is
not yet a stable userspace ABI. The presence of an IDT gate does not imply that
Ring 3 applications are currently started or isolated.

## Current syscall table

| `eax` | Operation | Inputs | Result / behavior |
| --- | --- | --- | --- |
| `0` | Get version | None | Packed version: major in bits 0–7, minor in 8–15, patch in 16–23 |
| `1` | Reboot | `ebx != 0` requests raw reboot | Reboots the machine; does not return |
| `2` | Shutdown | `ebx != 0` requests raw shutdown | Shuts down the machine; does not return |
| `3` | Create IPC endpoint | None | Endpoint identifier, or `0` on failure |
| `4` | Send IPC message | `ebx`: endpoint, `ecx`: data pointer, `edx`: length | IPC status code |
| `5` | Receive IPC message | `ebx`: endpoint, `ecx`: message pointer | IPC status code |

Unknown syscall numbers currently return `0xDEADC0DE`. IPC messages are handled
by a fixed-size in-kernel queue; this is not yet a complete cross-process
messaging or capability system.

## Not implemented

There is no Windows/Linux dual syscall ABI, executable-format compatibility
layer, stable public userspace calling convention, or complete user-pointer
validation policy. Those features require process isolation and a hardened
syscall boundary first.
