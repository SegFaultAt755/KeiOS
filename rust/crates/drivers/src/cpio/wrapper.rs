// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::archive::{CpioArchive, CpioError};
use super::header::CpioHeader;
use alloc::vec::Vec;
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

#[repr(i32)]
enum CpioStatus {
    Ok = 0,
    InvalidInfo = 1,
    InvalidHeader = 2,
    InvalidName = 3,
    TruncatedArchive = 4,
    DataRange = 5,
}

static mut GLOBAL_ARCHIVE: Option<CpioArchive<'static>> = None;

#[inline(always)]
#[allow(unsafe_op_in_unsafe_fn)]
unsafe fn get_archive() -> Option<&'static mut CpioArchive<'static>> {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_ARCHIVE);
    (*raw_ptr).as_mut()
}

// Core functions exported through the C FFI
#[unsafe(no_mangle)]
pub extern "C" fn cpio_init(info: CpioInfo) -> i32 {
    if info.base_addr.is_null() || info.size == 0 || info.size > isize::MAX as usize {
        return CpioStatus::InvalidInfo as i32;
    }

    unsafe {
        GLOBAL_ARCHIVE = Some(CpioArchive::new(info));
    }

    CpioStatus::Ok as i32
}

#[unsafe(no_mangle)]
pub extern "C" fn cpio_parse(callback: CpioCallback, user_context: *mut c_void) -> i32 {
    unsafe {
        let Some(archive) = get_archive() else {
            return CpioStatus::InvalidInfo as i32;
        };

        loop {
            match archive.next_entry() {
                Ok(Some(entry)) => {
                    let Some(name_capacity) = entry.name.len().checked_add(1) else {
                        return CpioStatus::DataRange as i32;
                    };
                    let mut c_name = Vec::with_capacity(name_capacity);
                    c_name.extend_from_slice(entry.name.as_bytes());
                    c_name.push(0);

                    callback(
                        c_name.as_ptr().cast::<c_char>(),
                        entry.header,
                        entry.data.as_ptr(),
                        entry.data.len(),
                        user_context,
                    );
                }
                Ok(None) => return CpioStatus::Ok as i32,
                Err(CpioError::TruncatedArchive) => return CpioStatus::TruncatedArchive as i32,
                Err(CpioError::InvalidHeader) => return CpioStatus::InvalidHeader as i32,
                Err(CpioError::InvalidName) => return CpioStatus::InvalidName as i32,
                Err(CpioError::DataRangeOverflow) => return CpioStatus::DataRange as i32,
            }
        }
    }
}
