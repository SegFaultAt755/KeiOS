# Processes and User Mode

## Current state

KeiOS does not yet have a functioning process manager or scheduler. The normal
boot path initializes hardware and runs a built-in polling shell in Ring 0.
There is no working transition to a separately mapped Ring 3 process.

The repository contains a userspace init program, a CPIO callback that locates
`bin/init.bin`, and an `execute_init_binary()` function. The call to that
function is disabled in `kernel_entry()`. The function is not a general ELF
loader and does not create independent address spaces.

## Planned process model

A future task subsystem will need to define and test:

- A process/task control block and lifecycle.
- Validated ELF program loading and per-task virtual address spaces.
- Kernel stacks, user stacks, privilege transitions, and saved register frames.
- Scheduling and context switching, including page-directory and TSS updates.
- A policy for granting devices or memory-mapped I/O to services.

These are design goals, not current capabilities. The kernel's current IPC
queues and syscall dispatcher do not by themselves create process isolation or
support user-space drivers.

## Status

| Feature | Status |
| --- | --- |
| Initramfs module scan and CPIO entry callback | Implemented |
| Bundled init program build/package | Present; not launched |
| General ELF32 loader | Not implemented |
| Process control blocks and scheduler | Not implemented |
| Ring 3 startup and task isolation | Not implemented |
| User-space display, input, and filesystem services | Not implemented |
