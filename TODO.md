# TODO

Follow-up items that must be addressed before enabling concurrency or adding
new execution backends, followed by the feature roadmap.

## Recheck before enabling concurrency or adding backends

- **Make IPC safe before enabling scheduling.** The PS/2 interrupt handler publishes directly to the IPC queue, and the current queue has no synchronization, blocking/wakeup semantics, or defined full-queue policy beyond returning an error. Add an IRQ-safe queue design and test producer/consumer races, queue-full behavior, and wakeups before enabling concurrent tasks.
- **Avoid unsound shared Rust state when adding tasks.** `get_display()` and `get_archive()` return mutable `'static` references to global mutable state. That is only safe under strict single-threaded, non-reentrant use; scheduling, callbacks, or interrupt-time access can create aliased mutable references. Move state behind a kernel synchronization/ownership design before those paths become concurrent.
- **Harden process loading before re-enabling `execute_init_binary()`.** The current loader copies raw bytes at virtual address zero, makes those pages writable and executable, and does not parse an executable format or establish a protected null page. Add validated segment mapping, permissions, BSS handling, stack setup, and a deliberate address-space layout before enabling the disabled handoff.
- **Define the user syscall trust boundary before launching Ring 3.** Syscalls 4 and 5 pass user-provided addresses straight into kernel `ipc_send()`/`ipc_receive()` without validating readable/writable ranges, and all IPC endpoints are globally addressable without ownership checks. Validate/copy user buffers, bind endpoints to principals, and define message sender identity before enabling user programs.
- **Make display pixel formats explicit before adding hardware backends.** `framebuffer_init()` only accepts 32-bpp and assumes the bootloader's channel layout matches the kernel's packed color value; it does not validate Multiboot RGB field positions/masks. Define a pixel-format/backend contract and test channel order, pitch, and bounds for each backend before adding another.
- **Add checksums and structural validation to firmware/archive parsers.** ACPI tables are followed using declared lengths without checksum validation, and the CPIO header parser accepts both `070701` and `070702` without verifying the latter's checksum. Validate checksums before callbacks or hardware actions; add malformed-input tests.
- **Test the new paths adversarially, not only on the happy boot.** Add tests or emulator checks for malformed/truncated CPIO and ACPI data, framebuffer initialization failure, full IPC queues, font generation from a clean checkout, and invalid executable headers. Ensure parsers return explicit errors rather than silently treating malformed input as end-of-file or defaults.
- **Preserve an explicit idle policy.** The current shell polls input and executes `hlt` between polls; revisit this when scheduling replaces the polling loop.
- **Bound hardware waits and reject invalid configuration.** Add timeout/error handling for device/controller waits and validate public parameters such as PIT frequency before division or port programming.

## Core

- Make the codebase and workflow clearer.
- Update README and docs as features land.
- Convert remaining drivers to use IPC and add a driver manager.
- Implement an EXT2 filesystem driver.
- Add scheduler support for multiple programs.
- Add PE executable support.
- Build a stable release with simple programs for shell testing.

## Syscalls

- Add driver-module syscalls.
- Add filesystem and file syscalls.
- Add scheduler syscalls.

## IPC and drivers

- Improve IPC safety checks.
- Add IPC support for restart, schedule, and shutdown.
- Convert CMOS, Terminal, VGA, Framebuffer, and PIT initialization to the intended driver/IPC model where appropriate.

## Later

- Add atomic support.
- Make drivers dynamic files.

## Project

- Create a wiki.
- Create a project video.
