// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

use super::header::CpioHeader;

const TRAILER_NAME: &str = "TRAILER!!!";

#[derive(Debug, Clone)]
pub struct CpioEntry<'a> {
    pub header: CpioHeader,
    pub name: &'a str,
    pub data: &'a [u8],
}

impl<'a> CpioEntry<'a> {
    pub(crate) fn new(header: CpioHeader, name: &'a str, data: &'a [u8]) -> Self {
        Self { header, name, data }
    }

    #[inline]
    pub(crate) fn is_trailer(&self) -> bool {
        self.name == TRAILER_NAME
    }
}
