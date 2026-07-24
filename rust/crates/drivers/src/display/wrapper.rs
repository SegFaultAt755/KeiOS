// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::display::{Color, Display};
use super::shapes::ShapeEngine;

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct DisplayInfo {
    pub lfb_addr: *mut u32,
    pub flags: u32,
    pub width: u32,
    pub height: u32,
    pub pitch: u32,
    pub bpp: u8,
}

static mut GLOBAL_DISPLAY: Option<Display> = None;

#[inline(always)]
#[allow(unsafe_op_in_unsafe_fn)]
pub unsafe fn get_display() -> Option<&'static mut Display> {
    let raw_ptr = core::ptr::addr_of_mut!(GLOBAL_DISPLAY);
    (*raw_ptr).as_mut()
}

// Core & Shape FFI exports
#[unsafe(no_mangle)]
pub extern "C" fn display_initialize(info: DisplayInfo) -> i32 {
    if info.lfb_addr.is_null() || info.width == 0 || info.height == 0 {
        return 1;
    }

    unsafe {
        GLOBAL_DISPLAY = Some(Display::new(info));
    }

    0
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

// Getters
#[unsafe(no_mangle)]
pub extern "C" fn display_get_width() -> u32 {
    unsafe { get_display().map(|d| d.width()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_height() -> u32 {
    unsafe { get_display().map(|d| d.height()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_pitch() -> u32 {
    unsafe { get_display().map(|d| d.pitch()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_bpp() -> u8 {
    unsafe { get_display().map(|d| d.bpp()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_flags() -> u32 {
    unsafe { get_display().map(|d| d.flags()).unwrap_or(0) }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_lfb_addr() -> *mut u32 {
    unsafe {
        get_display()
            .map(|d| d.framebuffer())
            .unwrap_or(core::ptr::null_mut())
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_get_pixel(x: i32, y: i32, out_color: *mut u32) -> i32 {
    if out_color.is_null() {
        return 1;
    }

    unsafe {
        if let Some(display) = get_display() {
            if let Some(color) = display.get_pixel(x, y) {
                *out_color = color.0;
                return 0;
            }
        }

        1
    }
}

// Setters
#[unsafe(no_mangle)]
pub extern "C" fn display_set_lfb_addr(new_lfb: *mut u32) {
    if new_lfb.is_null() {
        return;
    }

    unsafe {
        if let Some(display) = get_display() {
            display.set_framebuffer(new_lfb);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn display_set_resolution(width: u32, height: u32, pitch: u32) {
    if width == 0 || height == 0 || pitch == 0 {
        return;
    }

    unsafe {
        if let Some(display) = get_display() {
            display.set_resolution(width, height, pitch);
        }
    }
}

// Bitmap operations
#[unsafe(no_mangle)]
pub extern "C" fn display_draw_bitmap(bmp_data: *const u8, x: i32, y: i32) -> bool {
    unsafe {
        if let Some(display) = get_display() {
            return super::bitmap::draw_bitmap_simple(display, bmp_data, x, y);
        }
    }
    false
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_bitmap_ex(
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
    scale_num: u32,
    scale_den: u32,
    rotation_deg: i32,
    alpha: u8,
) -> bool {
    unsafe {
        if let Some(display) = get_display() {
            return super::bitmap::draw_bitmap_ex(
                display,
                bmp_data,
                dst_x,
                dst_y,
                scale_num,
                scale_den,
                rotation_deg,
                alpha,
            );
        }
    }
    false
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_bitmap_scaled(
    bmp_data: *const u8,
    x: i32,
    y: i32,
    scale_num: u32,
    scale_den: u32,
) -> bool {
    unsafe {
        if let Some(display) = get_display() {
            return super::bitmap::draw_bitmap_scaled(display, bmp_data, x, y, scale_num, scale_den);
        }
    }
    false
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_bitmap_rotated(
    bmp_data: *const u8,
    x: i32,
    y: i32,
    degrees: i32,
) -> bool {
    unsafe {
        if let Some(display) = get_display() {
            return super::bitmap::draw_bitmap_rotated(display, bmp_data, x, y, degrees);
        }
    }
    false
}

#[unsafe(no_mangle)]
pub extern "C" fn display_draw_bitmap_alpha(
    bmp_data: *const u8,
    x: i32,
    y: i32,
    alpha: u8,
) -> bool {
    unsafe {
        if let Some(display) = get_display() {
            return super::bitmap::draw_bitmap_alpha(display, bmp_data, x, y, alpha);
        }
    }
    false
}
