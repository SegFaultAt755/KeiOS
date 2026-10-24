// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

unsafe extern "C" {
    fn framebuffer_clear(color: u32);
    fn framebuffer_draw_pixel(color: u32, x: u32, y: u32);
    fn framebuffer_fill_rect(color: u32, x: u32, y: u32, width: u32, height: u32);
    fn framebuffer_scroll_up(height: u32, color: u32);
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Color(pub u32);

impl Color {
    pub const BLACK: Color = Color(0x00000000);
    pub const WHITE: Color = Color(0x00FFFFFF);

    #[inline]
    pub const fn rgb(r: u8, g: u8, b: u8) -> Self {
        Color(((r as u32) << 16) | ((g as u32) << 8) | (b as u32))
    }
}

pub struct Display;

impl Display {
    #[inline]
    pub fn draw_pixel(&mut self, x: i32, y: i32, color: Color) {
        if x < 0 || y < 0 {
            return;
        }

        unsafe {
            framebuffer_draw_pixel(color.0, x as u32, y as u32);
        }
    }

    pub fn clear(&mut self, color: Color) {
        unsafe {
            framebuffer_clear(color.0);
        }
    }

    pub fn scroll_up(&mut self, height: u32, color: Color) {
        unsafe {
            framebuffer_scroll_up(height, color.0);
        }
    }

    pub fn fill_rect(&mut self, x: i32, y: i32, width: u32, height: u32, color: Color) {
        if x < 0 || y < 0 {
            return;
        }

        unsafe {
            framebuffer_fill_rect(color.0, x as u32, y as u32, width, height);
        }
    }
}
