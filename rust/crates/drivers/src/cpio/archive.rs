// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::entry::CpioEntry;
use super::header::{CpioHeader, HEADER_SIZE};
use super::wrapper::CpioInfo;

use core::slice;
use core::str;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CpioError {
    TruncatedArchive,
    InvalidHeader,
    InvalidName,
    DataRangeOverflow,
}

pub struct CpioArchive<'a> {
    data: &'a [u8],
    offset: usize,
    trailer_seen: bool,
}

impl<'a> CpioArchive<'a> {
    pub unsafe fn new(info: CpioInfo) -> Self {
        let data = if info.base_addr.is_null() || info.size == 0 {
            &[]
        } else {
            unsafe { slice::from_raw_parts(info.base_addr, info.size) }
        };

        Self {
            data,
            offset: 0,
            trailer_seen: false,
        }
    }

    fn align_to_4(val: usize) -> Result<usize, CpioError> {
        val.checked_add(3)
            .map(|aligned| aligned & !3)
            .ok_or(CpioError::DataRangeOverflow)
    }

    fn read_filename(&self, offset: usize, namesize: usize) -> Result<&'a str, CpioError> {
        if namesize == 0 {
            return Err(CpioError::InvalidName);
        }
        let end = offset.checked_add(namesize).ok_or(CpioError::InvalidName)?;
        if end > self.data.len() {
            return Err(CpioError::InvalidName);
        }

        let name_bytes = &self.data[offset..end];
        let clean_bytes = name_bytes
            .strip_suffix(&[0])
            .ok_or(CpioError::InvalidName)?;
        if clean_bytes.is_empty() || clean_bytes.contains(&0) {
            return Err(CpioError::InvalidName);
        }
        let name_str = str::from_utf8(clean_bytes).map_err(|_| CpioError::InvalidName)?;

        Ok(name_str)
    }

    pub fn next_entry(&mut self) -> Result<Option<CpioEntry<'a>>, CpioError> {
        if self.trailer_seen {
            return Ok(None);
        }
        if self.offset >= self.data.len() {
            return Err(CpioError::TruncatedArchive);
        }

        if self.data.len().saturating_sub(self.offset) < HEADER_SIZE {
            return Err(CpioError::TruncatedArchive);
        }

        let header_slice = &self.data[self.offset..self.offset + HEADER_SIZE];
        let header = CpioHeader::parse(header_slice).ok_or(CpioError::InvalidHeader)?;
        let name_start = self
            .offset
            .checked_add(HEADER_SIZE)
            .ok_or(CpioError::DataRangeOverflow)?;
        let name = self.read_filename(name_start, header.namesize as usize)?;
        let name_end = name_start
            .checked_add(header.namesize as usize)
            .ok_or(CpioError::DataRangeOverflow)?;
        let data_start = Self::align_to_4(name_end)?;
        if data_start > self.data.len() {
            return Err(CpioError::TruncatedArchive);
        }

        let data_end = data_start
            .checked_add(header.filesize as usize)
            .ok_or(CpioError::DataRangeOverflow)?;
        if data_end > self.data.len() {
            return Err(CpioError::TruncatedArchive);
        }

        let file_data = &self.data[data_start..data_end];
        let next_offset = Self::align_to_4(data_end)?;
        if next_offset > self.data.len() {
            return Err(CpioError::TruncatedArchive);
        }
        self.offset = next_offset;

        let entry = CpioEntry::new(header, name, file_data);
        if entry.is_trailer() {
            if header.filesize != 0 {
                return Err(CpioError::InvalidHeader);
            }
            self.trailer_seen = true;
            return Ok(None);
        }

        Ok(Some(entry))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec::Vec;

    fn append_entry(archive: &mut Vec<u8>, name: &[u8], data: &[u8]) {
        let mut header = [b'0'; HEADER_SIZE];
        header[0..6].copy_from_slice(b"070701");
        header[6..14].copy_from_slice(b"00000000");
        header[14..22].copy_from_slice(b"000081A4");
        header[22..38].copy_from_slice(b"0000000000000000");
        header[38..54].copy_from_slice(b"0000000000000000");
        header[54..62].copy_from_slice(format_hex(data.len() as u32).as_bytes());
        header[62..94].copy_from_slice(b"00000000000000000000000000000000");
        header[94..102].copy_from_slice(format_hex(name.len() as u32).as_bytes());
        header[102..110].copy_from_slice(b"00000000");
        archive.extend_from_slice(&header);
        archive.extend_from_slice(name);
        while archive.len() % 4 != 0 {
            archive.push(0);
        }
        archive.extend_from_slice(data);
        while archive.len() % 4 != 0 {
            archive.push(0);
        }
    }

    fn format_hex(value: u32) -> alloc::string::String {
        alloc::format!("{value:08X}")
    }

    fn archive_from(bytes: &[u8]) -> CpioArchive<'_> {
        unsafe {
            CpioArchive::new(CpioInfo {
                base_addr: bytes.as_ptr(),
                size: bytes.len(),
            })
        }
    }

    #[test]
    fn reports_truncated_header() {
        let mut archive = archive_from(&[0; 10]);
        assert!(matches!(
            archive.next_entry(),
            Err(CpioError::TruncatedArchive)
        ));
    }

    #[test]
    fn rejects_names_without_a_terminator() {
        let mut bytes = Vec::new();
        append_entry(&mut bytes, b"file", b"");
        let mut archive = archive_from(&bytes);
        assert!(matches!(archive.next_entry(), Err(CpioError::InvalidName)));
    }

    #[test]
    fn rejects_empty_names_and_embedded_nulls() {
        for name in [&b"\0"[..], &b"fi\0le\0"[..]] {
            let mut bytes = Vec::new();
            append_entry(&mut bytes, name, b"");
            let mut archive = archive_from(&bytes);
            assert!(matches!(archive.next_entry(), Err(CpioError::InvalidName)));
        }
    }

    #[test]
    fn requires_a_trailer_after_entries() {
        let mut bytes = Vec::new();
        append_entry(&mut bytes, b"file\0", b"");
        let mut archive = archive_from(&bytes);
        assert!(archive.next_entry().unwrap().is_some());
        assert!(matches!(
            archive.next_entry(),
            Err(CpioError::TruncatedArchive)
        ));
    }

    #[test]
    fn accepts_the_trailer_as_the_normal_end() {
        let mut bytes = Vec::new();
        append_entry(&mut bytes, b"TRAILER!!!\0", b"");
        let mut archive = archive_from(&bytes);
        assert!(matches!(archive.next_entry(), Ok(None)));
        assert!(matches!(archive.next_entry(), Ok(None)));
    }
}
