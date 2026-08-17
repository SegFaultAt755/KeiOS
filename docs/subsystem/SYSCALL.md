# System Calls Interface

---

## Overview

This specification documents the system call dispatch table and interface for process interactions. System calls are invoked using standard register parameters via software interrupt `int 0x80`

> **Warning:** Executing specific low-level system commands, such as a **hard reboot** or **hard shutdown**, bypasses standard process cleanup and may lead to filesystem or device corruption

---

## System Call Table

> **Note:** if `eax` (output) has value `0xDEADC0DE`, it means invalid syscall index, either not implemented or not found

| **Index (eax)** | **Name**        | **Description**                                | **eax (Output)**                                          | **ebx**                   | **ecx**      | **edx**      |
| --------------- | --------------- | ---------------------------------------------- | --------------------------------------------------------- | ------------------------- | ------------ | ------------ |
| `0`             | `k_get_version` | Retrieves the active kernel version            | `0x00(patch)(minor)(major)` in hexadecimal format         | `NULL` / `0`              | `NULL` / `0` | `NULL` / `0` |
| `1`             | `k_reboot`      | Initiates a soft system reboot from the kernel | Success status (may fail if the caller lacks permissions) | Hard reboot (`int 0/1`)   | `NULL` / `0` | `NULL` / `0` |
| `2`             | `k_shutdown`    | Initiates a soft system shutdown               | Success status (may fail if the caller lacks permissions) | Hard shutdown (`int 0/1`) | `NULL` / `0` | `NULL` / `0` |