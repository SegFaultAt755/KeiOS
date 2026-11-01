extern crate alloc;

mod wrapper {
    #[derive(Clone, Copy)]
    pub struct CpioInfo {
        pub base_addr: *const u8,
        pub size: usize,
    }
}

#[path = "../rust/crates/drivers/src/cpio/header.rs"]
mod header;
#[path = "../rust/crates/drivers/src/cpio/entry.rs"]
mod entry;
#[path = "../rust/crates/drivers/src/cpio/archive.rs"]
mod archive;
