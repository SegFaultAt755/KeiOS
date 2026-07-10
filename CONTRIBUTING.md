# Contributing to KeiOS

First off, thank you for looking to contribute! We are excited to build a modern, efficient codebase together. 

Please note our core stack rules:
* 💻 **Core Language:** Pure C23.
* 🐍 **Scripting & Tooling:** Python 3.10+.
* 🛑 **Strictly No C++.** Pull Requests introducing C++ code or paradigms will not be accepted.

---

## Technical Requirements

Before you start coding, ensure your local environment supports our modern stack:

### C23 Compiler
Because we use modern C23 features (like `#embed`, `constexpr`, and auto type inference), you will need a very recent compiler:
* **GCC:** Version 13 or newer.
* **Clang:** Version 18 or newer.

### Python Environment
For build scripts, test runners, or automation tools:
* Python 3.10 or newer is required.
* We recommend setting up a virtual environment (`python -m venv .venv`) before installing script dependencies.

---

## How Can I Contribute?

### 1. Reporting Bugs & Feature Ideas
Please open an **Issue** before writing code for major changes. Include:
* A clear description of the issue or feature.
* Steps to reproduce (for bugs).

### 2. Submitting a Pull Request (PR)
1. **Fork** the repo and create your branch from `main` (e.g., `git checkout -b feature/my-cool-feature`).
2. **Write your code.** Ensure your C code adheres to C23 standards and doesn't rely on GNU extensions unless strictly necessary.
3. **Format your code** (see Style Guide below).
4. **Commit and Push** your changes with clear, descriptive commit messages.
5. Open a **Pull Request** against our `main` branch.

---

## Code Style Guide

### C23 Style
* Run `clang-format` before committing. We use the `.clang-format` file provided in the root directory.
* Do not use C++ style casts or object-oriented structures that mimic C++. Keep it clean, idiomatic C.

### Python Style
* Follow PEP 8 guidelines for all scripting files.
* Run `black` or `ruff` on your Python scripts to ensure clean formatting.
