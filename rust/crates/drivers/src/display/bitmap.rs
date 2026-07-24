// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

#[allow(unused_imports)]
use alloc::vec;
use alloc::vec::Vec;
use super::display::{Color, Display};

const BMP_MAGIC: u16 = 0x4D42;
const BI_RGB: u32 = 0;
const BI_RLE8: u32 = 1;
const BI_RLE4: u32 = 2;
const BI_BITFIELDS: u32 = 3;

const MAX_IMAGE_DIM: i32 = 8192;
const MAX_BMP_SIZE: usize = 16 * 1024 * 1024;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BmpError {
    InvalidMagic,
    UnsupportedCompression,
    UnsupportedBpp,
    ImageTooLarge,
    InvalidData,
}

#[derive(Debug, Clone)]
pub struct BmpInfo {
    pub width: i32,
    pub height: i32,
    pub bpp: u16,
    pub compression: u32,
    pub palette: [[u8; 4]; 256],
    pub row_stride: u32,
    pub bottom_up: bool,
    pub data_offset: u32,
    pub image_size: u32,
    pub red_mask: u32,
    pub green_mask: u32,
    pub blue_mask: u32,
}

impl BmpInfo {
    fn default_masks_for_bpp(bpp: u16) -> (u32, u32, u32) {
        match bpp {
            32 => (0x00FF0000, 0x0000FF00, 0x000000FF),
            24 => (0x00FF0000, 0x0000FF00, 0x000000FF),
            16 => (0xF800, 0x07E0, 0x001F),
            _ => (0, 0, 0),
        }
    }
}

#[repr(C, packed)]
struct BmpFileHeader {
    magic: u16,
    file_size: u32,
    _reserved: u32,
    data_offset: u32,
}

#[repr(C, packed)]
struct BmpInfoHeader {
    header_size: u32,
    width: i32,
    height: i32,
    planes: u16,
    bpp: u16,
    compression: u32,
    image_size: u32,
    x_ppm: i32,
    y_ppm: i32,
    colors_used: u32,
    colors_important: u32,
}

unsafe fn read_u8(ptr: *const u8, offset: usize) -> u8 {
    unsafe { *ptr.add(offset) }
}

unsafe fn read_u16_unaligned(ptr: *const u8, offset: usize) -> u16 {
    unsafe { core::ptr::read_unaligned(ptr.add(offset) as *const u16) }
}

unsafe fn read_u32_unaligned(ptr: *const u8, offset: usize) -> u32 {
    unsafe { core::ptr::read_unaligned(ptr.add(offset) as *const u32) }
}

unsafe fn read_i32_unaligned(ptr: *const u8, offset: usize) -> i32 {
    unsafe { core::ptr::read_unaligned(ptr.add(offset) as *const i32) }
}

pub fn parse_bmp_header(data: *const u8) -> Result<BmpInfo, BmpError> {
    unsafe {
        let magic = read_u16_unaligned(data, 0);
        if magic != BMP_MAGIC {
            return Err(BmpError::InvalidMagic);
        }

        let file_size = read_u32_unaligned(data, 2);
        if file_size as usize > MAX_BMP_SIZE {
            return Err(BmpError::ImageTooLarge);
        }

        let data_offset = read_u32_unaligned(data, 10);
        let header_size = read_u32_unaligned(data, 14);

        if header_size < 40 {
            return Err(BmpError::InvalidData);
        }

        let width = read_i32_unaligned(data, 18);
        let height = read_i32_unaligned(data, 22);
        let planes = read_u16_unaligned(data, 26);
        let bpp = read_u16_unaligned(data, 28);
        let compression = read_u32_unaligned(data, 30);
        let image_size = read_u32_unaligned(data, 34);

        if planes != 1 {
            return Err(BmpError::InvalidData);
        }

        match bpp {
            1 | 4 | 8 | 24 | 32 => {}
            _ => return Err(BmpError::UnsupportedBpp),
        }

        match compression {
            BI_RGB | BI_BITFIELDS => {}
            BI_RLE8 if bpp == 8 => {}
            BI_RLE4 if bpp == 4 => {}
            _ => return Err(BmpError::UnsupportedCompression),
        }

        let abs_height = if height < 0 { -height } else { height };
        if abs_height == 0 || width == 0 {
            return Err(BmpError::InvalidData);
        }
        if abs_height > MAX_IMAGE_DIM || width > MAX_IMAGE_DIM {
            return Err(BmpError::ImageTooLarge);
        }

        let row_stride = ((width * bpp as i32 + 31) / 32) * 4;
        let bottom_up = height > 0;

        let mut palette = [[0u8; 4]; 256];
        if bpp <= 8 {
            let palette_offset = 14 + header_size as usize;
            let num_colors = if compression == BI_BITFIELDS { 0 } else { 1 << bpp };
            for i in 0..num_colors {
                let off = palette_offset + i * 4;
                palette[i][0] = read_u8(data, off);
                palette[i][1] = read_u8(data, off + 1);
                palette[i][2] = read_u8(data, off + 2);
                palette[i][3] = read_u8(data, off + 3);
            }
        }

        let (mut red_mask, mut green_mask, mut blue_mask) =
            BmpInfo::default_masks_for_bpp(bpp);

        if compression == BI_BITFIELDS {
            let mask_offset = 14 + header_size as usize;
            red_mask = read_u32_unaligned(data, mask_offset);
            green_mask = read_u32_unaligned(data, mask_offset + 4);
            blue_mask = read_u32_unaligned(data, mask_offset + 8);
        }

        Ok(BmpInfo {
            width,
            height: abs_height,
            bpp,
            compression,
            palette,
            row_stride: row_stride as u32,
            bottom_up,
            data_offset,
            image_size,
            red_mask,
            green_mask,
            blue_mask,
        })
    }
}

pub fn decompress_rle8(data: *const u8, info: &BmpInfo) -> Option<Vec<u8>> {
    let pixel_count = (info.width * info.height) as usize;
    let mut output = vec![0u8; pixel_count];

    unsafe {
        let rle_data = data.add(info.data_offset as usize);
        let data_size = if info.image_size > 0 {
            info.image_size as usize
        } else {
            MAX_BMP_SIZE
        };

        let mut x: i32 = 0;
        let mut y: i32 = 0;
        let mut pos: usize = 0;

        while pos < data_size {
            let first = *rle_data.add(pos);
            pos += 1;
            let second = *rle_data.add(pos);
            pos += 1;

            if first != 0 {
                for _ in 0..first {
                    if x >= 0 && x < info.width && y >= 0 && y < info.height {
                        let idx = (y as usize) * info.width as usize + x as usize;
                        if idx < pixel_count {
                            output[idx] = second;
                        }
                    }
                    x += 1;
                }
            } else {
                match second {
                    0 => {
                        x = 0;
                        y += 1;
                    }
                    1 => break,
                    2 => {
                        if pos + 2 <= data_size {
                            let dx = *rle_data.add(pos) as i32;
                            let dy = *rle_data.add(pos + 1) as i32;
                            pos += 2;
                            x += dx;
                            y += dy;
                        } else {
                            break;
                        }
                    }
                    n => {
                        let count = n as i32;
                        for i in 0..count {
                            if pos < data_size {
                                let px = x + i;
                                if px >= 0 && px < info.width && y >= 0 && y < info.height {
                                    let idx = (y as usize) * info.width as usize + px as usize;
                                    if idx < pixel_count {
                                        output[idx] = *rle_data.add(pos);
                                    }
                                }
                                pos += 1;
                            }
                        }
                        x += count;
                        if (n % 2) != 0 {
                            pos += 1;
                        }
                    }
                }
            }
        }

        Some(output)
    }
}

pub fn decompress_rle4(data: *const u8, info: &BmpInfo) -> Option<Vec<u8>> {
    let pixel_count = (info.width * info.height) as usize;
    let mut output = vec![0u8; pixel_count];

    unsafe {
        let rle_data = data.add(info.data_offset as usize);
        let data_size = if info.image_size > 0 {
            info.image_size as usize
        } else {
            MAX_BMP_SIZE
        };

        let mut x: i32 = 0;
        let mut y: i32 = 0;
        let mut pos: usize = 0;

        while pos < data_size {
            let first = *rle_data.add(pos);
            pos += 1;
            let second = *rle_data.add(pos);
            pos += 1;

            if first != 0 {
                for i in 0..first {
                    let nibble = if i % 2 == 0 {
                        (second >> 4) & 0xF
                    } else {
                        second & 0xF
                    };
                    if x >= 0 && x < info.width && y >= 0 && y < info.height {
                        let idx = (y as usize) * info.width as usize + x as usize;
                        if idx < pixel_count {
                            output[idx] = nibble;
                        }
                    }
                    x += 1;
                }
            } else {
                match second {
                    0 => {
                        x = 0;
                        y += 1;
                    }
                    1 => break,
                    2 => {
                        if pos + 2 <= data_size {
                            let dx = *rle_data.add(pos) as i32;
                            let dy = *rle_data.add(pos + 1) as i32;
                            pos += 2;
                            x += dx;
                            y += dy;
                        } else {
                            break;
                        }
                    }
                    n => {
                        let count = n as i32;
                        let bytes_needed = (count as usize + 1) / 2;
                        for i in 0..count {
                            if pos < data_size {
                                let byte_val = *rle_data.add(pos);
                                let nibble = if i % 2 == 0 {
                                    (byte_val >> 4) & 0xF
                                } else {
                                    byte_val & 0xF
                                };
                                let px = x + i;
                                if px >= 0 && px < info.width && y >= 0 && y < info.height {
                                    let idx = (y as usize) * info.width as usize + px as usize;
                                    if idx < pixel_count {
                                        output[idx] = nibble;
                                    }
                                }
                                if i % 2 == 1 {
                                    pos += 1;
                                }
                            }
                        }
                        pos += 1;
                        x += count;
                        if bytes_needed % 2 != 0 {
                            pos += 1;
                        }
                    }
                }
            }
        }

        Some(output)
    }
}

fn extract_mask_channel(value: u32, mask: u32) -> u8 {
    if mask == 0 {
        return 0;
    }
    let shift = mask.trailing_zeros();
    let bits = mask.count_ones();
    let masked = (value & mask) >> shift;
    if bits >= 8 {
        (masked >> (bits - 8)) as u8
    } else {
        (masked << (8 - bits)) as u8
    }
}

fn read_pixel_flat(
    data: &[u8],
    info: &BmpInfo,
    x: i32,
    y: i32,
    _width: i32,
) -> u32 {
    let offset = (y as usize) * info.row_stride as usize;

    match info.bpp {
        1 => {
            let byte_idx = offset + (x as usize / 8);
            let bit_idx = 7 - (x as usize % 8);
            if byte_idx >= data.len() {
                return 0;
            }
            let byte = data[byte_idx];
            let index = (byte >> bit_idx) & 1;
            if index == 0 {
                0x00000000
            } else {
                0x00FFFFFF
            }
        }
        4 => {
            let byte_idx = offset + (x as usize / 2);
            if byte_idx >= data.len() {
                return 0;
            }
            let byte = data[byte_idx];
            let index = if x % 2 == 0 {
                ((byte >> 4) & 0xF) as usize
            } else {
                (byte & 0xF) as usize
            };
            let entry = &info.palette[index];
            ((entry[2] as u32) << 16) | ((entry[1] as u32) << 8) | (entry[0] as u32)
        }
        8 => {
            let byte_idx = offset + x as usize;
            if byte_idx >= data.len() {
                return 0;
            }
            let index = data[byte_idx] as usize;
            let entry = &info.palette[index];
            ((entry[2] as u32) << 16) | ((entry[1] as u32) << 8) | (entry[0] as u32)
        }
        24 => {
            let byte_idx = offset + x as usize * 3;
            if byte_idx + 2 >= data.len() {
                return 0;
            }
            let b = data[byte_idx] as u32;
            let g = data[byte_idx + 1] as u32;
            let r = data[byte_idx + 2] as u32;
            (r << 16) | (g << 8) | b
        }
        32 => {
            let byte_idx = offset + x as usize * 4;
            if byte_idx + 3 >= data.len() {
                return 0;
            }
            let pixel = (data[byte_idx] as u32)
                | ((data[byte_idx + 1] as u32) << 8)
                | ((data[byte_idx + 2] as u32) << 16)
                | ((data[byte_idx + 3] as u32) << 24);

            if info.compression == BI_BITFIELDS {
                let r = extract_mask_channel(pixel, info.red_mask) as u32;
                let g = extract_mask_channel(pixel, info.green_mask) as u32;
                let b = extract_mask_channel(pixel, info.blue_mask) as u32;
                (r << 16) | (g << 8) | b
            } else {
                pixel & 0x00FFFFFF
            }
        }
        _ => 0,
    }
}

#[inline]
fn blend_pixel(dst: u32, src: u32, alpha: u8) -> u32 {
    let a = alpha as u32;
    let inv_a = 255 - a;

    let dr = (dst >> 16) & 0xFF;
    let dg = (dst >> 8) & 0xFF;
    let db = dst & 0xFF;

    let sr = (src >> 16) & 0xFF;
    let sg = (src >> 8) & 0xFF;
    let sb = src & 0xFF;

    let r = (sr * a + dr * inv_a) / 255;
    let g = (sg * a + dg * inv_a) / 255;
    let b = (sb * a + db * inv_a) / 255;

    (r << 16) | (g << 8) | b
}

fn fixed_sin(deg: i32) -> i32 {
    let deg = ((deg % 360) + 360) % 360;
    let rad = (deg as f32) * core::f32::consts::PI / 180.0;
    let x = rad;
    let x2 = x * x;
    let x3 = x2 * x;
    let x5 = x3 * x2;
    let x7 = x5 * x2;
    let x9 = x7 * x2;
    let result = x - x3 / 6.0 + x5 / 120.0 - x7 / 5040.0 + x9 / 362880.0;
    (result * 65536.0) as i32
}

fn fixed_cos(deg: i32) -> i32 {
    fixed_sin(deg + 90)
}

#[inline]
fn compute_source_pixel(
    ox: i32,
    oy: i32,
    out_w: i32,
    out_h: i32,
    src_w: i32,
    src_h: i32,
    cos_q16: i32,
    sin_q16: i32,
) -> (i32, i32) {
    let dx = 2 * ox - out_w;
    let dy = 2 * oy - out_h;

    let rx_q16 = (cos_q16 as i64) * (dx as i64) + (sin_q16 as i64) * (dy as i64);
    let ry_q16 = -(sin_q16 as i64) * (dx as i64) + (cos_q16 as i64) * (dy as i64);

    let half_w = (out_w as i64) << 16;
    let half_h = (out_h as i64) << 16;

    let sx_num = (rx_q16 + half_w) * (src_w as i64);
    let sy_num = (ry_q16 + half_h) * (src_h as i64);

    let sx = (sx_num / (out_w as i64 * 2)) as i32;
    let sy = (sy_num / (out_h as i64 * 2)) as i32;

    (sx, sy)
}

pub fn draw_bitmap_ex(
    display: &mut Display,
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
    scale_num: u32,
    scale_den: u32,
    rotation_deg: i32,
    alpha: u8,
) -> bool {
    if bmp_data.is_null() {
        return false;
    }

    let info = match parse_bmp_header(bmp_data) {
        Ok(i) => i,
        Err(_) => return false,
    };

    let src_w = info.width;
    let src_h = info.height;

    let out_w = if scale_den != 0 && scale_den != 1 {
        ((src_w as i64) * (scale_num as i64) / (scale_den as i64)) as i32
    } else {
        src_w
    };
    let out_h = if scale_den != 0 && scale_den != 1 {
        ((src_h as i64) * (scale_num as i64) / (scale_den as i64)) as i32
    } else {
        src_h
    };

    if out_w <= 0 || out_h <= 0 {
        return false;
    }

    let is_rle = info.compression == BI_RLE8 || info.compression == BI_RLE4;

    let decompressed: Vec<u8>;
    let raw_slice: &[u8];

    if is_rle {
        decompressed = if info.compression == BI_RLE8 {
            match decompress_rle8(bmp_data, &info) {
                Some(d) => d,
                None => return false,
            }
        } else {
            match decompress_rle4(bmp_data, &info) {
                Some(d) => d,
                None => return false,
            }
        };
        raw_slice = &decompressed;
    } else {
        let total_bytes = info.row_stride as usize * src_h as usize;
        unsafe {
            raw_slice = core::slice::from_raw_parts(
                bmp_data.add(info.data_offset as usize),
                total_bytes,
            );
        }
    }

    let has_rotation = rotation_deg % 360 != 0;
    let has_scale = scale_num != scale_den;

    let cos_q16 = if has_rotation { fixed_cos(rotation_deg) } else { 65536 };
    let sin_q16 = if has_rotation { fixed_sin(rotation_deg) } else { 0 };

    let screen_w = display.width() as i32;
    let screen_h = display.height() as i32;
    let opaque = alpha == 255;

    for oy in 0..out_h {
        for ox in 0..out_w {
            let (sx, sy) = if has_rotation {
                compute_source_pixel(
                    ox, oy, out_w, out_h, src_w, src_h, cos_q16, sin_q16,
                )
            } else if has_scale {
                let sx = ((ox as i64) * (src_w as i64) / (out_w as i64)) as i32;
                let sy = ((oy as i64) * (src_h as i64) / (out_h as i64)) as i32;
                (sx, sy)
            } else {
                (ox, oy)
            };

            if sx < 0 || sx >= src_w || sy < 0 || sy >= src_h {
                continue;
            }

            let read_y = if is_rle {
                sy
            } else if info.bottom_up {
                src_h - 1 - sy
            } else {
                sy
            };

            let pixel = read_pixel_flat(raw_slice, &info, sx, read_y, src_w);

            let fb_x = dst_x + ox;
            let fb_y = dst_y + oy;

            if fb_x < 0 || fb_x >= screen_w || fb_y < 0 || fb_y >= screen_h {
                continue;
            }

            if opaque {
                display.draw_pixel(fb_x, fb_y, Color(pixel));
            } else {
                if let Some(dst_color) = display.get_pixel(fb_x, fb_y) {
                    let blended = blend_pixel(dst_color.0, pixel, alpha);
                    display.draw_pixel(fb_x, fb_y, Color(blended));
                }
            }
        }
    }

    true
}

pub fn draw_bitmap_simple(
    display: &mut Display,
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
) -> bool {
    draw_bitmap_ex(display, bmp_data, dst_x, dst_y, 1, 1, 0, 255)
}

pub fn draw_bitmap_scaled(
    display: &mut Display,
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
    scale_num: u32,
    scale_den: u32,
) -> bool {
    draw_bitmap_ex(display, bmp_data, dst_x, dst_y, scale_num, scale_den, 0, 255)
}

pub fn draw_bitmap_rotated(
    display: &mut Display,
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
    degrees: i32,
) -> bool {
    draw_bitmap_ex(display, bmp_data, dst_x, dst_y, 1, 1, degrees, 255)
}

pub fn draw_bitmap_alpha(
    display: &mut Display,
    bmp_data: *const u8,
    dst_x: i32,
    dst_y: i32,
    alpha: u8,
) -> bool {
    draw_bitmap_ex(display, bmp_data, dst_x, dst_y, 1, 1, 0, alpha)
}
