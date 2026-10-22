use super::fonts::ASCII_FONT_16X16;
use crate::display::display::{Color, Display};

pub trait TextEngine {
    // Uses default static font (ASCII 16x16)
    fn draw_text_static(&mut self, x: i32, y: i32, text: &str, color: Color);
}

impl TextEngine for Display {
    fn draw_text_static(&mut self, x: i32, y: i32, text: &str, color: Color) {
        let mut cursor_x = x;
        let mut cursor_y = y;

        for character in text.chars() {
            match character {
                '\n' => {
                    cursor_x = x;
                    cursor_y = cursor_y.saturating_add(16);
                }
                '\r' => cursor_x = x,
                '\t' => cursor_x = cursor_x.saturating_add(64),
                character => {
                    let glyph_index = match character {
                        ' '..='~' => character as usize - ' ' as usize,
                        _ => '?' as usize - ' ' as usize,
                    };
                    let glyph = &ASCII_FONT_16X16[glyph_index];

                    for (row, bits) in glyph.iter().enumerate() {
                        for column in 0..16 {
                            if bits & (1 << (15 - column)) != 0 {
                                self.draw_pixel(
                                    cursor_x.saturating_add(column),
                                    cursor_y.saturating_add(row as i32),
                                    color,
                                );
                            }
                        }
                    }

                    cursor_x = cursor_x.saturating_add(16);
                }
            }
        }
    }
}
