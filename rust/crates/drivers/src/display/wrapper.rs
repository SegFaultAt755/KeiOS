// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::display::{Color, Display};
use super::shapes::ShapeEngine;
use super::text::TextEngine;
use core::ffi::CStr;

#[repr(i32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DisplayStatus {
    Ok = 0,
    AlreadyInitialized = 1,
    NotInitialized = 2,
}

#[repr(i32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DisplayTextStatus {
    Ok = 0,
    InvalidArgument = 1,
    NotInitialized = 2,
}

static mut GLOBAL_DISPLAY: Option<Display> = None;

#[inline(always)]
#[allow(unsafe_op_in_unsafe_fn)]
pub unsafe fn get_display() -> Result<&'static mut Display, DisplayStatus> {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
    unsafe { (*raw_ptr).as_mut().ok_or(DisplayStatus::NotInitialized) }
}

// Core and shape functions exported through the C FFI
#[unsafe(no_mangle)]
pub extern "C" fn display_init() -> DisplayStatus {
    unsafe {
        let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
        if (*raw_ptr).is_some() {
            return DisplayStatus::AlreadyInitialized;
        }

        *raw_ptr = Some(Display);
    }

    DisplayStatus::Ok
}

#[unsafe(no_mangle)]
pub extern "C" fn display_is_initialized() -> bool {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
    unsafe { (*raw_ptr).is_some() }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_clear(color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.clear(Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_scroll_up(height: u32, color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.scroll_up(height, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_pixel(x: i32, y: i32, color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.draw_pixel(x, y, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_line(x0: i32, y0: i32, x1: i32, y1: i32, color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.draw_line(x0, y0, x1, y1, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_rect(x: i32, y: i32, width: u32, height: u32, color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.draw_rect(x, y, width, height, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_fill_rect(x: i32, y: i32, width: u32, height: u32, color: u32) {
    unsafe {
        if let Ok(display) = get_display() {
            display.fill_rect(x, y, width, height, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_text_static(
    x: i32,
    y: i32,
    text: *const core::ffi::c_char,
    color: u32,
) -> DisplayTextStatus {
    if text.is_null() {
        return DisplayTextStatus::InvalidArgument;
    }

    let text = unsafe { CStr::from_ptr(text) };
    let Ok(text) = text.to_str() else {
        return DisplayTextStatus::InvalidArgument;
    };

    unsafe {
        match get_display() {
            Ok(display) => {
                display.draw_text_static(x, y, text, Color(color));
                DisplayTextStatus::Ok
            }
            Err(_) => DisplayTextStatus::NotInitialized,
        }
    }
}
