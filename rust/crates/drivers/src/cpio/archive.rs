// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::entry::{CpioEntry, EntryParser};
use super::header::{HEADER_SIZE, HeaderParser, NewcParser};
use super::wrapper::CpioInfo;

use alloc::string::String;
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
}

impl<'a> CpioArchive<'a> {
    pub unsafe fn new(info: CpioInfo) -> Self {
        let data = if info.base_addr.is_null() || info.size == 0 {
            &[]
        } else {
            unsafe { slice::from_raw_parts(info.base_addr, info.size) }
        };

        Self { data, offset: 0 }
    }

    // Functions that read archive state
    #[inline]
    pub fn offset(&self) -> usize {
        self.offset
    }

    #[inline]
    pub fn total_size(&self) -> usize {
        self.data.len()
    }

    #[inline]
    pub fn raw_buffer(&self) -> *const u8 {
        self.data.as_ptr()
    }

    // Functions that change archive state
    pub fn set_offset(&mut self, new_offset: usize) {
        if new_offset <= self.data.len() {
            self.offset = new_offset;
        }
    }

    pub unsafe fn set_buffer(&mut self, new_addr: *const u8, size: usize) {
        if !new_addr.is_null() && size > 0 {
            self.data = unsafe { slice::from_raw_parts(new_addr, size) };
            self.offset = 0;
        }
    }

    // Functions that move through archive entries
    #[inline]
    fn align_to_4(&self, val: usize) -> usize {
        (val + 3) & !3
    }

    fn read_filename(&self, offset: usize, namesize: usize) -> Result<String, CpioError> {
        let end = offset.checked_add(namesize).ok_or(CpioError::InvalidName)?;
        if end > self.data.len() {
            return Err(CpioError::InvalidName);
        }

        let name_bytes = &self.data[offset..end];
        let clean_bytes = name_bytes.strip_suffix(&[0]).unwrap_or(name_bytes);
        let name_str = str::from_utf8(clean_bytes).map_err(|_| CpioError::InvalidName)?;

        Ok(String::from(name_str))
    }

    pub fn next_entry(&mut self) -> Result<Option<CpioEntry<'a>>, CpioError> {
        if self.offset >= self.data.len() {
            return Ok(None);
        }

        if self.data.len().saturating_sub(self.offset) < HEADER_SIZE {
            return Err(CpioError::TruncatedArchive);
        }

        let parser = NewcParser;
        let header_slice = &self.data[self.offset..self.offset + HEADER_SIZE];
        let header = parser.parse(header_slice).ok_or(CpioError::InvalidHeader)?;
        self.offset += HEADER_SIZE;

        let name = self.read_filename(self.offset, header.namesize as usize)?;
        self.offset = self.align_to_4(self.offset + header.namesize as usize);

        let data_start = self.offset;
        let data_end = data_start
            .checked_add(header.filesize as usize)
            .ok_or(CpioError::DataRangeOverflow)?;
        if data_end > self.data.len() {
            return Err(CpioError::DataRangeOverflow);
        }

        let file_data = &self.data[data_start..data_end];
        self.offset = self.align_to_4(data_end);

        let entry = CpioEntry::new(header, name, file_data);
        if entry.is_trailer() {
            return Ok(None);
        }

        Ok(Some(entry))
    }
}
