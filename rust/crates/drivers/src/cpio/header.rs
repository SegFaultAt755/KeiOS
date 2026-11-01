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
    fn parse_hex_u32(bytes: &[u8]) -> Option<u32> {
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

    pub(crate) fn parse(raw_data: &[u8]) -> Option<Self> {
        if raw_data.len() < HEADER_SIZE
            || (&raw_data[0..6] != CPIO_NEWC_MAGIC && &raw_data[0..6] != CPIO_CRC_MAGIC)
        {
            return None;
        }

        Some(Self {
            mode: Self::parse_hex_u32(&raw_data[14..22])?,
            uid: Self::parse_hex_u32(&raw_data[22..30])?,
            gid: Self::parse_hex_u32(&raw_data[30..38])?,
            mtime: Self::parse_hex_u32(&raw_data[46..54])?,
            filesize: Self::parse_hex_u32(&raw_data[54..62])?,
            namesize: Self::parse_hex_u32(&raw_data[94..102])?,
        })
    }
}
