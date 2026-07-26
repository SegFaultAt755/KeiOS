// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

pub const HEADER_SIZE: usize = 110;

const CPIO_NEWC_MAGIC: &[u8; 6] = b"070701";
const CPIO_CRC_MAGIC: &[u8; 6] = b"070702";

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CpioHeader {
    pub mode: u32,
    pub uid: u32,
    pub gid: u32,
    pub mtime: u32,
    pub filesize: u32,
    pub namesize: u32,
}

impl CpioHeader {
    pub const EMPTY: Self = Self {
        mode: 0,
        uid: 0,
        gid: 0,
        mtime: 0,
        filesize: 0,
        namesize: 0,
    };
}

pub trait HeaderParser {
    fn parse_hex_u32(&self, bytes: &[u8]) -> Option<u32>;
    fn validate_magic(&self, magic: &[u8]) -> bool;
    fn parse(&self, raw_data: &[u8]) -> Option<CpioHeader>;
}

pub struct NewcParser;
impl HeaderParser for NewcParser {
    fn parse_hex_u32(&self, bytes: &[u8]) -> Option<u32> {
        let mut result: u32 = 0;
        for &byte in bytes {
            let digit = match byte {
                b'0'..=b'9' => byte - b'0',
                b'a'..=b'f' => byte - b'a' + 10,
                b'A'..=b'F' => byte - b'A' + 10,
                _ => return None,
            };

            result = result.checked_shl(4)?.checked_add(digit as u32)?;
        }

        Some(result)
    }

    #[inline]
    fn validate_magic(&self, magic: &[u8]) -> bool {
        magic == CPIO_NEWC_MAGIC || magic == CPIO_CRC_MAGIC
    }

    fn parse(&self, raw_data: &[u8]) -> Option<CpioHeader> {
        if raw_data.len() < HEADER_SIZE {
            return None;
        }

        if !self.validate_magic(&raw_data[0..6]) {
            return None;
        }

        Some(CpioHeader {
            mode: self.parse_hex_u32(&raw_data[14..22])?,
            uid: self.parse_hex_u32(&raw_data[22..30])?,
            gid: self.parse_hex_u32(&raw_data[30..38])?,
            mtime: self.parse_hex_u32(&raw_data[46..54])?,
            filesize: self.parse_hex_u32(&raw_data[54..62])?,
            namesize: self.parse_hex_u32(&raw_data[94..102])?,
        })
    }
}
