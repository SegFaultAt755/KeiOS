// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::bitmap;
use super::wrapper::DisplayInfo;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Color(pub u32);

impl Color {
    pub const BLACK: Color = Color(0x00000000);
    pub const WHITE: Color = Color(0x00FFFFFF);

    #[inline]
    pub const fn rgb(r: u8, g: u8, b: u8) -> Self {
        Color(((r as u32) << 16) | ((g as u32) << 8) | (b as u32))
    }

    #[inline]
    pub const fn r(self) -> u8 {
        ((self.0 >> 16) & 0xFF) as u8
    }

    #[inline]
    pub const fn g(self) -> u8 {
        ((self.0 >> 8) & 0xFF) as u8
    }

    #[inline]
    pub const fn b(self) -> u8 {
        (self.0 & 0xFF) as u8
    }

    #[inline]
    pub const fn with_alpha(self, a: u8) -> u32 {
        ((a as u32) << 24) | (self.0 & 0x00FFFFFF)
    }
}

pub struct Display {
    lfb: *mut u32,
    flags: u32,
    width: u32,
    height: u32,
    pitch: u32,
    bpp: u8,
}

impl Display {
    pub fn new(info: DisplayInfo) -> Self {
        Self {
            width: info.width,
            height: info.height,
            pitch: info.pitch,
            bpp: info.bpp,
            flags: info.flags,
            lfb: info.lfb_addr,
        }
    }

    // Getters
    #[inline]
    pub fn width(&self) -> u32 {
        self.width
    }
    #[inline]
    pub fn height(&self) -> u32 {
        self.height
    }
    #[inline]
    pub fn dimensions(&self) -> (u32, u32) {
        (self.width, self.height)
    }
    #[inline]
    pub fn pitch(&self) -> u32 {
        self.pitch
    }
    #[inline]
    pub fn bpp(&self) -> u8 {
        self.bpp
    }
    #[inline]
    pub fn flags(&self) -> u32 {
        self.flags
    }
    #[inline]
    pub fn framebuffer(&self) -> *mut u32 {
        self.lfb
    }

    // Setters
    pub unsafe fn set_framebuffer(&mut self, new_lfb: *mut u32) {
        self.lfb = new_lfb;
    }

    pub fn set_resolution(&mut self, width: u32, height: u32, pitch: u32) {
        self.width = width;
        self.height = height;
        self.pitch = pitch;
    }

    // Rendering operations
    #[inline]
    pub fn draw_pixel(&mut self, x: i32, y: i32, color: Color) {
        if x < 0 || y < 0 || x as u32 >= self.width || y as u32 >= self.height {
            return;
        }

        unsafe {
            let pixels_per_row = (self.pitch / 4) as usize;
            let offset = (y as usize * pixels_per_row) + x as usize;
            self.lfb.add(offset).write_volatile(color.0);
        }
    }

    #[inline]
    pub fn get_pixel(&self, x: i32, y: i32) -> Option<Color> {
        if x < 0 || y < 0 || x as u32 >= self.width || y as u32 >= self.height {
            return None;
        }

        unsafe {
            let pixels_per_row = (self.pitch / 4) as usize;
            let offset = (y as usize * pixels_per_row) + x as usize;
            Some(Color(self.lfb.add(offset).read_volatile()))
        }
    }

    pub fn clear(&mut self, color: Color) {
        let count = ((self.pitch / 4) * self.height) as usize;
        unsafe {
            for i in 0..count {
                self.lfb.add(i).write_volatile(color.0);
            }
        }
    }
}

pub trait BitmapEngine {
    fn draw_bitmap(&mut self, bmp_data: *const u8, dst_x: i32, dst_y: i32) -> bool;
    fn draw_bitmap_scaled(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        scale_num: u32,
        scale_den: u32,
    ) -> bool;
    fn draw_bitmap_rotated(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        degrees: i32,
    ) -> bool;
    fn draw_bitmap_alpha(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        alpha: u8,
    ) -> bool;
    fn draw_bitmap_ex(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        scale_num: u32,
        scale_den: u32,
        rotation_deg: i32,
        alpha: u8,
    ) -> bool;
}

impl BitmapEngine for Display {
    fn draw_bitmap(&mut self, bmp_data: *const u8, dst_x: i32, dst_y: i32) -> bool {
        bitmap::draw_bitmap_simple(self, bmp_data, dst_x, dst_y)
    }

    fn draw_bitmap_scaled(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        scale_num: u32,
        scale_den: u32,
    ) -> bool {
        bitmap::draw_bitmap_scaled(self, bmp_data, dst_x, dst_y, scale_num, scale_den)
    }

    fn draw_bitmap_rotated(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        degrees: i32,
    ) -> bool {
        bitmap::draw_bitmap_rotated(self, bmp_data, dst_x, dst_y, degrees)
    }

    fn draw_bitmap_alpha(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        alpha: u8,
    ) -> bool {
        bitmap::draw_bitmap_alpha(self, bmp_data, dst_x, dst_y, alpha)
    }

    fn draw_bitmap_ex(
        &mut self,
        bmp_data: *const u8,
        dst_x: i32,
        dst_y: i32,
        scale_num: u32,
        scale_den: u32,
        rotation_deg: i32,
        alpha: u8,
    ) -> bool {
        bitmap::draw_bitmap_ex(
            self, bmp_data, dst_x, dst_y, scale_num, scale_den, rotation_deg, alpha,
        )
    }
}
