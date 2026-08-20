#![no_std]
#![feature(alloc_error_handler)]

// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

extern crate alloc;

use core::alloc::{GlobalAlloc, Layout};
use core::panic::PanicInfo;

// Rust modules used by the drivers crate
pub mod cpio;
pub mod display;

// Functions provided by the C kernel
unsafe extern "C" {
    fn runtime_panic(reason: *const i8, desc: *const i8, file: *const i8, line: u32);

    fn kmalloc(size: usize) -> *mut u8;
    fn kfree(ptr: *mut u8);
}

// Connect Rust allocations to the kernel heap
struct KernelAllocator;

unsafe impl GlobalAlloc for KernelAllocator {
    #![allow(unsafe_op_in_unsafe_fn)]
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        kmalloc(layout.size())
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        kfree(ptr);
    }
}

#[global_allocator]
static ALLOCATOR: KernelAllocator = KernelAllocator;

#[alloc_error_handler]
fn alloc_error_handler(_layout: Layout) -> ! {
    unsafe {
        // Building a dynamic error message here would trigger another panic
        runtime_panic(
            c"Kernel heap exhaustion".as_ptr() as *const i8,
            c"kmalloc failed to allocate requested layout size".as_ptr() as *const i8,
            concat!(file!(), '\0').as_ptr() as *const i8,
            line!(),
        );
    }
    loop {}
}

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    #[allow(deprecated)]
    let msg = match info.payload().downcast_ref::<&str>() {
        Some(s) => s, // Use the custom message when one was provided
        None => match info.message().as_str() {
            // Otherwise, use the standard panic message
            Some(s) => s,
            None => "Rust runtime exception occurred",
        },
    };

    let mut c_buffer = [0u8; 128];
    let bytes = msg.as_bytes();
    let len = core::cmp::min(bytes.len(), c_buffer.len() - 1); // Leave room for the final '\0'

    // Copy the bytes into the buffer and add the null terminator
    c_buffer[..len].copy_from_slice(&bytes[..len]);
    c_buffer[len] = b'\0';

    unsafe {
        runtime_panic(
            c"Rust panic".as_ptr() as *const i8,
            c_buffer.as_ptr() as *const i8,
            concat!(file!(), '\0').as_ptr() as *const i8,
            line!(),
        );
    }

    loop {}
}
