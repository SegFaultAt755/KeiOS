use super::display::{Color, Display};

// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

pub trait ShapeEngine {
    fn draw_line(&mut self, x0: i32, y0: i32, x1: i32, y1: i32, color: Color);
    fn draw_rect(&mut self, x: i32, y: i32, width: u32, height: u32, color: Color);
    fn fill_rect(&mut self, x: i32, y: i32, width: u32, height: u32, color: Color);
}

impl ShapeEngine for Display {
    // Draw a line using Bresenham algorithm
    fn draw_line(&mut self, mut x0: i32, mut y0: i32, x1: i32, y1: i32, color: Color) {
        let dx = (x1 - x0).abs();
        let dy = -(y1 - y0).abs();
        let sx = if x0 < x1 { 1 } else { -1 };
        let sy = if y0 < y1 { 1 } else { -1 };
        let mut err = dx + dy;

        loop {
            self.draw_pixel(x0, y0, color);
            if x0 == x1 && y0 == y1 {
                break;
            }
            let e2 = 2 * err;
            if e2 >= dy {
                err += dy;
                x0 += sx;
            }

            if e2 <= dx {
                err += dx;
                y0 += sy;
            }
        }
    }

    // Draw a rectangle outline
    fn draw_rect(&mut self, x: i32, y: i32, width: u32, height: u32, color: Color) {
        let w = width as i32;
        let h = height as i32;
        self.draw_line(x, y, x + w - 1, y, color);
        self.draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
        self.draw_line(x, y, x, y + h - 1, color);
        self.draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
    }

    // Draw a fill rectangle
    fn fill_rect(&mut self, x: i32, y: i32, width: u32, height: u32, color: Color) {
        for row in 0..height as i32 {
            for col in 0..width as i32 {
                self.draw_pixel(x + col, y + row, color);
            }
        }
    }
}
