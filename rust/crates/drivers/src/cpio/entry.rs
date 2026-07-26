// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::header::CpioHeader;
use alloc::string::String;

const TRAILER_NAME: &str = "TRAILER!!!";

#[derive(Debug, Clone)]
pub struct CpioEntry<'a> {
    pub header: CpioHeader,
    pub name: String,
    pub data: &'a [u8],
}

impl<'a> CpioEntry<'a> {
    pub fn new(header: CpioHeader, name: String, data: &'a [u8]) -> Self {
        Self { header, name, data }
    }

    // Getters
    #[inline]
    pub fn header(&self) -> CpioHeader {
        self.header
    }

    #[inline]
    pub fn name(&self) -> &str {
        &self.name
    }

    #[inline]
    pub fn data(&self) -> &'a [u8] {
        self.data
    }

    #[inline]
    pub fn size(&self) -> u32 {
        self.header.filesize
    }
}

pub trait EntryParser {
    fn is_trailer(&self) -> bool;
    fn is_empty(&self) -> bool;
}

impl<'a> EntryParser for CpioEntry<'a> {
    #[inline]
    fn is_trailer(&self) -> bool {
        self.name == TRAILER_NAME
    }

    #[inline]
    fn is_empty(&self) -> bool {
        self.header.filesize == 0 || self.data.is_empty()
    }
}
