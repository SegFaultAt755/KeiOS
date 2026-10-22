use super::fonts::{ASCII_FONT_DEFAULT, ASCII_FONT_HEIGHT, ASCII_FONT_WIDTH};
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
                    cursor_y = cursor_y.saturating_add(ASCII_FONT_HEIGHT as i32);
                }
                '\r' => cursor_x = x,
                '\t' => {
                    cursor_x = cursor_x.saturating_add((ASCII_FONT_WIDTH as i32).saturating_mul(4))
                }
                character => {
                    let glyph_index = match character {
                        ' '..='~' => character as usize - ' ' as usize,
                        _ => '?' as usize - ' ' as usize,
                    };
                    let glyph = &ASCII_FONT_DEFAULT[glyph_index];

                    for (row_index, row_bytes) in glyph.iter().enumerate() {
                        for column in 0..ASCII_FONT_WIDTH {
                            if row_bytes[column / 8] & (1 << (7 - column % 8)) != 0 {
                                self.draw_pixel(
                                    cursor_x.saturating_add(column as i32),
                                    cursor_y.saturating_add(row_index as i32),
                                    color,
                                );
                            }
                        }
                    }

                    cursor_x = cursor_x.saturating_add(ASCII_FONT_WIDTH as i32);
                }
            }
        }
    }
}
