# TODO

Cleanup and architecture follow-ups from the codebase review, followed by the existing feature backlog.

## Cleanup and architecture decisions

- **Keep `display_*` as the drawing API.** The display API owns drawing operations; the framebuffer is its current memory-backed implementation. Add backend selection only when a second real backend exists.
- **Remove or justify the single-implementation Rust traits.** `ShapeEngine`, `TextEngine`, `HeaderParser`, and `EntryParser` each have one implementation and no generic or dynamic-dispatch consumers. Use inherent methods until a second implementation or test seam makes the trait valuable.
- **Reduce redundant CPIO name ownership and parser surface.** `CpioArchive::read_filename()` allocates a `String`, then `cpio_parse()` clones it to append a NUL for the C callback. Prefer one bounded conversion at the FFI boundary, and keep only accessors/traits that callers actually use.
- **Prune unused display state FFI.** The display width/height/pitch/bpp/flags/framebuffer getters, pixel getter, and setters are declared and exported but have no call sites. Remove them or add a real consumer; avoid maintaining a second, unused display-state API.
- **Make the generated font reproducible.** Resolve the ignored generated-source build failure in `FIXME.md` by choosing one source of truth for the font and wiring its generation into the normal build.
- **Consolidate tool orchestration.** `Makefile`, `build.py`, `run.py`, and `clean.py` overlap in build/clean responsibility. Keep the Makefile as the build/clean source of truth and make the Python scripts thin front ends; avoid the separate `.gitignore`-pattern deletion pass in `clean.py` unless it has a clearly documented need.
- **Make architecture documentation match the implementation.** `docs/ARCHITECTURE.md` describes isolated Ring 3 drivers and a microkernel, but current boot initializes PS/2/display handling in the kernel and has no scheduler. Update the claims as services move out of the kernel.
- **Remove misleading success checks.** `src/kernel/boot/cpu.c` passes constant `1` to three `KERNEL_ASSERT()` calls after void initialization functions. Either have those initializers report failure or remove the assertions.
- **Keep display naming consistent.** Callers use `display_*`; framebuffer-specific setup and operations stay in the framebuffer implementation.

## Recheck after the current FIXME items are fixed

- **Make IPC safe before enabling scheduling.** The PS/2 interrupt handler publishes directly to the IPC queue, and the current queue has no synchronization, blocking/wakeup semantics, or defined full-queue policy beyond returning an error. Add an IRQ-safe queue design and test producer/consumer races, queue-full behavior, and wakeups before enabling concurrent tasks.
- **Avoid unsound shared Rust state when adding tasks.** `get_display()` and `get_archive()` return mutable `'static` references to global mutable state. That is only safe under strict single-threaded, non-reentrant use; scheduling, callbacks, or interrupt-time access can create aliased mutable references. Move state behind a kernel synchronization/ownership design before those paths become concurrent.
- **Harden process loading before re-enabling `execute_init_binary()`.** The current loader copies raw bytes at virtual address zero, makes those pages writable and executable, and does not parse an executable format or establish a protected null page. Add validated segment mapping, permissions, BSS handling, stack setup, and a deliberate address-space layout before enabling the disabled handoff.
- **Define the user syscall trust boundary before launching Ring 3.** Syscalls 4 and 5 pass user-provided addresses straight into kernel `ipc_send()`/`ipc_receive()` without validating readable/writable ranges, and all IPC endpoints are globally addressable without ownership checks. Validate/copy user buffers, bind endpoints to principals, and define message sender identity before enabling user programs.
- **Make display pixel formats explicit before adding hardware backends.** `framebuffer_init()` only accepts 32-bpp and assumes the bootloader's channel layout matches the kernel's packed color value; it does not validate Multiboot RGB field positions/masks. Define a pixel-format/backend contract and test channel order, pitch, and bounds for each backend before adding another.
- **Add checksums and structural validation to firmware/archive parsers.** ACPI tables are followed using declared lengths without checksum validation, and `NewcParser` accepts both `070701` and `070702` without verifying the latter's checksum. Validate lengths, checksums, and arithmetic overflow before callbacks or hardware actions; add malformed-input tests.
- **Test the new paths adversarially, not only on the happy boot.** Add tests or emulator checks for malformed/truncated CPIO and ACPI data, framebuffer initialization failure, full IPC queues, font generation from a clean checkout, and invalid executable headers. Ensure parsers return explicit errors rather than silently treating malformed input as end-of-file or defaults.
- **Give the idle path an explicit policy.** The current post-boot loop is empty and spins continuously. Once input polling/scheduling is added, use an idle task that halts until an interrupt and make shutdown/halt behavior explicit.
- **Bound hardware waits and reject invalid configuration.** Add timeout/error handling for device/controller waits and validate public parameters such as PIT frequency before division or port programming.

## Unused code candidates (verified no in-repository callers)

Before deleting public headers or exported ABI symbols, confirm there are no downstream users; for internal-only code, remove both declaration and implementation together.

- **Unused display drawing ABI:** `display_draw_line()` and `display_draw_rect()` currently have no in-repository callers. Keep them only if they are intended public operations; otherwise remove them from both the C header and Rust FFI.
- **Unused PS/2 and VGA APIs:** `ps2_disable()`, `ps2_get_modifiers()`, and `vga_clear()` are declared and implemented but have no callers. `vga_disable_cursor()` also has no caller. Keep only if a planned lifecycle or terminal feature will use them soon.
- **Unused CPIO state API:** the C declarations and Rust exports for `cpio_get_offset()`, `cpio_get_total_size()`, `cpio_get_base_addr()`, `cpio_set_offset()`, and `cpio_set_buffer()` have no C callers. Remove the FFI/state mutation surface if the parser remains a one-shot archive walk.
- **Unused CPIO entry surface:** `CpioEntry::is_empty()` and the `header()`, `name()`, `data()`, and `size()` accessors have no callers; the fields are already public. `CpioHeader::EMPTY` is also unused. Remove redundant methods/constants, or make fields private if the accessors are intended to be the sole API.
- **Unused string/stdio helpers:** `itoa()` is implemented and declared but unused. `kprint_uint()` and `kprint_int()` are declared in `include/libkern/stdio.h` but have no definitions or callers; remove those stale declarations. Check whether `vga_clear()` is redundant with `terminal_clear()` before deleting either implementation.
- **Unused memory-management helpers:** `pmm_free_frame()`, `vmm_unmap_page()`, and `vmm_get_phys()` have declarations and implementations but no in-repository call sites. `memmove()` and `strdup()` likewise appear to have no callers. Remove these only if no imminent allocator, process teardown, or library work depends on them; otherwise keep the API scoped to the feature that will consume it.
- **Dead init-program capture path:** `exec_init` and `exec_init_size` are populated during CPIO parsing but only read inside the disabled `#if 0` block in `kernel_entry()`. Either complete and enable the executable-loading path after its safety work, or stop capturing the init entry and remove its declarations until it is usable.
- **Unused parser traits:** `HeaderParser`, `EntryParser`, `ShapeEngine`, and `TextEngine` each have one implementation and no generic/dynamic-dispatch use. Replace with inherent methods unless a real second implementation or test seam is being added.

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

## Small tasks

- Make rust to use Result and Option
- Update README and write AGENTS.md
- Make CI show the previous result if there is a fatal error.
