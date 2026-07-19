# Contributing to KeiOS

First off, thank you for looking to contribute! We are excited to build a modern, efficient, and well architected codebase together. Whether you are fixing a quick bug or proposing a major feature, this guide will help you get started easily

Please note our core stack rules:
* 💻 **Core Language:** C23 + Rust 1.96
* 🐍 **Scripting & Tooling:** Python 3.10+
* 🛑 **Strictly No C++.** Pull Requests introducing C++ code or paradigms will not be accepted

---

## Architecture & Engineering Principles

To keep KeiOS clean, maintainable, and fast, all contributions must adhere to the following architectural guidelines:

* **Single Responsibility Principle:** Keep files small and focused. Each file should do only one thing, but do it exceptionally well. Design modules with minimal dependencies on other files to reduce tight coupling
* **Avoid Premature Abstraction:** Do not create abstract layers, wrappers, or complex interfaces preemptively. Create abstractions only when they are strictly necessary, or when the exact same code repeats at least **3 to 4 times**
* **Dependency & Crate Discipline:** Rely on the standard library whenever possible. Only introduce new third party Rust crates when they solve a significant, widespread problem across the codebase. Never pull in a external crate or library just to simplify a single line of code or perform a trivial task

---

## Interoperability: Rust & C FFI

KeiOS leverages both modern C23 and Rust. When bridging the two languages, prioritize memory safety and clean API boundaries:

### Calling Rust from C
* Use `#[no_mangle]` and `extern "C"` on your public Rust functions so the C linker can resolve them
* Represent shared data structures using `#[repr(C)]` in Rust to guarantee memory layout compatibility
* Generate and maintain clean C header files (`.h`) corresponding to your exposed Rust APIs

### Calling C from Rust
* Enclose C function declarations inside `extern "C" { ... }` blocks in Rust
* Wrap raw FFI calls in safe, idiomatic Rust abstractions as close to the boundary as possible to prevent unsafe code from leaking into the broader business logic
* Never pass unvalidated pointers or uninitialized memory across the FFI boundary

---

## AI Assistant Policy

We embrace modern development tools, including AI coding assistants (e.g., Copilot, ChatGPT, Claude), but **you remain entirely responsible for the code you submit**

If you use AI to assist in writing code, you must ensure:
1. **Full Understanding:** You know exactly how every line of generated code works. Never submit code you cannot verbally explain and defend during review
2. **Zero Issues:** The code must be fully tested, bug free, and introduce no security vulnerabilities or performance bottlenecks
3. **Architectural Alignment:** The AI generated code must cleanly match the existing project architecture, naming conventions, and style guidelines without introducing unnecessary bloat or weird patterns

---

## Technical Requirements

Before you start coding, ensure your local environment supports our modern stack:

* **C23 Compiler:** GCC 13+ or Clang 18+ (required for modern features like `#embed`, `constexpr`, and auto type inference)
* **Rust Compiler:** Rustc & Cargo v1.96 is preferred (v1.95 is minimally accepted)
* **Python Environment:** Python 3.10+ (recommend using a virtual environment: `python -m venv .venv`)

---

## How Can I Contribute?

We designed our workflow so everyone can add features and fix bugs easily without unnecessary red tape

### 1. Fixing Bugs
If you find a typo, broken link, or a clear, reproducible bug:
1. **Fork** the repository and create a bugfix branch from `main` (e.g., `git checkout -b fix/memory-leak-in-driver-display`)
2. Write your fix and add a test case that proves the bug is resolved
3. Open a **Pull Request (PR)** directly against `main`. You do not need to open an issue first for straightforward bug fixes

### 2. Adding Features & Major Changes
To ensure your effort aligns with the roadmap and doesn't get wasted:
1. **Open an Issue first** describing the proposed feature, the problem it solves, and your high level implementation plan
2. Once discussed and approved, **Fork** the repo and create a feature branch from `main` (e.g., `git checkout -b feature/new-scheduler`)
3. Write clean, modular code adhering to our architectural principles and FFI rules
4. Format your code using our project linters (see Style Guide below)
5. Open a **Pull Request** referencing your original issue (e.g., `Closes #123`)

---

## Code Style Guide

* **C23 Style:** Run `clang-format` using the `.clang-format` file in the root directory before committing. Keep it clean, idiomatic C. Don't attempt to mimic C++ object oriented structures
* **Rust Style:** Run `cargo fmt` to apply standard formatting default rules (no `rustfmt.toml` available yet). Use `cargo clippy` to catch common mistakes
* **Python Style:** Follow PEP 8 guidelines. Run `black` or `ruff` on all scripting files to guarantee clean formatting
