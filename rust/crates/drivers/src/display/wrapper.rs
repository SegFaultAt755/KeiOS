// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::display::{Color, Display};
use super::shapes::ShapeEngine;
use super::text::TextEngine;
use core::ffi::CStr;

static mut GLOBAL_DISPLAY: Option<Display> = None;

#[inline(always)]
#[allow(unsafe_op_in_unsafe_fn)]
pub unsafe fn get_display() -> Option<&'static mut Display> {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
    (*raw_ptr).as_mut()
}

// Core and shape functions exported through the C FFI
#[unsafe(no_mangle)]
pub extern "C" fn display_init() -> bool {
    unsafe {
        GLOBAL_DISPLAY = Some(Display);
    }

    true
}

#[unsafe(no_mangle)]
pub extern "C" fn display_is_initialized() -> bool {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
    unsafe { (*raw_ptr).is_some() }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_clear(color: u32) {
    unsafe {
        if let Some(display) = get_display() {
            display.clear(Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_scroll_up(height: u32, color: u32) {
    unsafe {
        if let Some(display) = get_display() {
            display.scroll_up(height, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_pixel(x: i32, y: i32, color: u32) {
    unsafe {
        if let Some(display) = get_display() {
            display.draw_pixel(x, y, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_line(x0: i32, y0: i32, x1: i32, y1: i32, color: u32) {
    unsafe {
        if let Some(display) = get_display() {
            display.draw_line(x0, y0, x1, y1, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_rect(x: i32, y: i32, width: u32, height: u32, color: u32) {
    unsafe {
        if let Some(display) = get_display() {
            display.draw_rect(x, y, width, height, Color(color));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_fill_rect(x: i32, y: i32, width: u32, height: u32, color: u32) {
    unsafe {
        if let Some(display) = get_display() {
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
) -> i32 {
    if text.is_null() {
        return 1;
    }

    let text = unsafe { CStr::from_ptr(text) };
    let Ok(text) = text.to_str() else {
        return 1;
    };

    unsafe {
        if let Some(display) = get_display() {
            display.draw_text_static(x, y, text, Color(color));
            return 0;
        }
    }

    1
}
