// SPDX-License-Identifier: GPLv3
// Copyright (C) 2026 KeiOS Developers

pub const IPC_MAX_PAYLOAD: usize = 64;
pub const IPC_QUEUE_CAPACITY: usize = 16;
pub const IPC_MAX_ENDPOINTS: usize = 32;

pub const IPC_OK: i32 = 0;
pub const IPC_INVALID: i32 = 1;
pub const IPC_FULL: i32 = 2;
pub const IPC_EMPTY: i32 = 3;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct IpcMessage {
    pub sender: u32,
    pub length: u32,
    pub data: [u8; IPC_MAX_PAYLOAD],
}

#[derive(Clone, Copy)]
struct Endpoint {
    active: bool,
    head: usize,
    tail: usize,
    count: usize,
    messages: [IpcMessage; IPC_QUEUE_CAPACITY],
}

impl Endpoint {
    const EMPTY_MESSAGE: IpcMessage = IpcMessage {
        sender: 0,
        length: 0,
        data: [0; IPC_MAX_PAYLOAD],
    };

    const fn new() -> Self {
        Self {
            active: false,
            head: 0,
            tail: 0,
            count: 0,
            messages: [Self::EMPTY_MESSAGE; IPC_QUEUE_CAPACITY],
        }
    }

    fn reset(&mut self) {
        *self = Self::new();
    }
}

struct IpcEngine {
    initialized: bool,
    endpoints: [Endpoint; IPC_MAX_ENDPOINTS],
}

impl IpcEngine {
    const fn new() -> Self {
        Self {
            initialized: false,
            endpoints: [Endpoint::new(); IPC_MAX_ENDPOINTS],
        }
    }

    fn reset(&mut self) {
        self.initialized = true;
        for endpoint in &mut self.endpoints {
            endpoint.reset();
        }
    }

    fn endpoint_mut(&mut self, endpoint: u32) -> Option<&mut Endpoint> {
        if endpoint == 0 {
            return None;
        }

        self.endpoints
            .get_mut(endpoint as usize)
            .filter(|endpoint| endpoint.active)
    }
}

static mut ENGINE: IpcEngine = IpcEngine::new();

unsafe fn engine() -> &'static mut IpcEngine {
    unsafe { &mut *core::ptr::addr_of_mut!(ENGINE) }
}

#[unsafe(no_mangle)]
pub extern "C" fn ipc_init() -> i32 {
    unsafe { engine().reset() }
    IPC_OK
}

#[unsafe(no_mangle)]
pub extern "C" fn ipc_endpoint_create() -> u32 {
    unsafe {
        let ipc = engine();
        if !ipc.initialized {
            return 0;
        }

        for (index, endpoint) in ipc.endpoints.iter_mut().enumerate().skip(1) {
            if !endpoint.active {
                endpoint.active = true;
                return index as u32;
            }
        }
    }

    0
}

#[unsafe(no_mangle)]
pub extern "C" fn ipc_send(endpoint: u32, sender: u32, data: *const u8, length: usize) -> i32 {
    if length > IPC_MAX_PAYLOAD || (length != 0 && data.is_null()) {
        return IPC_INVALID;
    }

    unsafe {
        let Some(queue) = engine().endpoint_mut(endpoint) else {
            return IPC_INVALID;
        };
        if queue.count == IPC_QUEUE_CAPACITY {
            return IPC_FULL;
        }

        let message = &mut queue.messages[queue.tail];
        message.sender = sender;
        message.length = length as u32;
        if length != 0 {
            core::ptr::copy_nonoverlapping(data, message.data.as_mut_ptr(), length);
        }

        queue.tail = (queue.tail + 1) % IPC_QUEUE_CAPACITY;
        queue.count += 1;
    }

    IPC_OK
}

#[unsafe(no_mangle)]
pub extern "C" fn ipc_receive(endpoint: u32, message: *mut IpcMessage) -> i32 {
    if message.is_null() {
        return IPC_INVALID;
    }

    unsafe {
        let Some(queue) = engine().endpoint_mut(endpoint) else {
            return IPC_INVALID;
        };
        if queue.count == 0 {
            return IPC_EMPTY;
        }

        *message = queue.messages[queue.head];
        queue.head = (queue.head + 1) % IPC_QUEUE_CAPACITY;
        queue.count -= 1;
    }

    IPC_OK
}
