// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::archive::CpioArchive;
use super::header::CpioHeader;
use core::ffi::{c_char, c_void};

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CpioInfo {
    pub base_addr: *const u8,
    pub size: usize,
}

pub type CpioCallback = extern "C" fn(
    name: *const c_char,
    header: CpioHeader,
    data: *const u8,
    data_len: usize,
    user_context: *mut c_void,
);

static mut GLOBAL_ARCHIVE: Option<CpioArchive<'static>> = None;

#[inline(always)]
#[allow(unsafe_op_in_unsafe_fn)]
pub unsafe fn get_archive() -> Option<&'static mut CpioArchive<'static>> {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_ARCHIVE);
    (*raw_ptr).as_mut()
}

// Core FFI exports
#[unsafe(no_mangle)]
pub extern "C" fn cpio_initialize(info: CpioInfo) -> i32 {
    if info.base_addr.is_null() || info.size == 0 {
        return 1;
    }

    unsafe {
        GLOBAL_ARCHIVE = Some(CpioArchive::new(info));
    }

    0
}

#[unsafe(no_mangle)]
pub extern "C" fn cpio_parse(callback: CpioCallback, user_context: *mut c_void) -> i32 {
    unsafe {
        if let Some(archive) = get_archive() {
            while let Some(entry) = archive.next_entry() {
                let mut c_name = entry.name.clone();
                c_name.push('\0');

                callback(
                    c_name.as_ptr() as *const c_char,
                    entry.header,
                    entry.data.as_ptr(),
                    entry.data.len(),
                    user_context,
                );
            }

            return 0;
        }
    }

    1
}

// Getters
#[unsafe(no_mangle)]
pub extern "C" fn cpio_get_offset() -> usize {
    unsafe { get_archive().map(|a| a.offset()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn cpio_get_total_size() -> usize {
    unsafe { get_archive().map(|a| a.total_size()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn cpio_get_base_addr() -> *const u8 {
    unsafe {
        get_archive()
            .map(|a| a.raw_buffer())
            .unwrap_or(core::ptr::null())
    }
}

// Setters
#[unsafe(no_mangle)]
pub extern "C" fn cpio_set_offset(new_offset: usize) {
    unsafe {
        if let Some(archive) = get_archive() {
            archive.set_offset(new_offset);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cpio_set_buffer(new_addr: *const u8, size: usize) {
    if new_addr.is_null() || size == 0 {
        return;
    }

    unsafe {
        if let Some(archive) = get_archive() {
            archive.set_buffer(new_addr, size);
        }
    }
}
