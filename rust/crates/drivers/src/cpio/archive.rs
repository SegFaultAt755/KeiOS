// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::entry::{CpioEntry, EntryParser};
use super::header::{HEADER_SIZE, HeaderParser, NewcParser};
use super::wrapper::CpioInfo;

use alloc::string::String;
use core::slice;
use core::str;

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

    // Getters
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

    // Setters
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

    // Navigation operations
    #[inline]
    fn align_to_4(&self, val: usize) -> usize {
        (val + 3) & !3
    }

    #[inline]
    fn has_remaining_bytes(&self) -> bool {
        self.offset + HEADER_SIZE <= self.data.len()
    }

    fn read_filename(&self, offset: usize, namesize: usize) -> Option<String> {
        let end = offset.checked_add(namesize)?;
        if end > self.data.len() {
            return None;
        }

        let name_bytes = &self.data[offset..end];
        let clean_bytes = name_bytes.strip_suffix(&[0]).unwrap_or(name_bytes);
        let name_str = str::from_utf8(clean_bytes).ok()?;

        Some(String::from(name_str))
    }

    pub fn next_entry(&mut self) -> Option<CpioEntry<'a>> {
        if !self.has_remaining_bytes() {
            return None;
        }

        let parser = NewcParser;
        let header_slice = &self.data[self.offset..self.offset + HEADER_SIZE];
        let header = parser.parse(header_slice)?;
        self.offset += HEADER_SIZE;

        let name = self.read_filename(self.offset, header.namesize as usize)?;
        self.offset = self.align_to_4(self.offset + header.namesize as usize);

        let data_start = self.offset;
        let data_end = data_start.checked_add(header.filesize as usize)?;
        if data_end > self.data.len() {
            return None;
        }

        let file_data = &self.data[data_start..data_end];
        self.offset = self.align_to_4(data_end);

        let entry = CpioEntry::new(header, name, file_data);
        if entry.is_trailer() {
            return None;
        }

        Some(entry)
    }
}
