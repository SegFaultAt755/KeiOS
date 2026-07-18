#![no_std]

// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use core::panic::PanicInfo;

unsafe extern "C" {
    fn runtime_panic(reason: *const i8, desc: *const i8, file: *const i8, line: u32);
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_validate_magic(magic: u32) -> bool {
    magic == 0x2BADB002
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    unsafe {
        runtime_panic(
            c"Rust panic".as_ptr(),
            c"No description".as_ptr(),
            concat!(file!(), "\0").as_ptr() as *const i8,
            line!(),
        );
    };

    loop {}
}
