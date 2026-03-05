<div align="center">

# Developer Infrastructure Tool

### Lightweight CLI for Automating Dockerized Deployments

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Status](https://img.shields.io/badge/status-early--stage-yellow)](https://github.com)
[![Platform](https://img.shields.io/badge/platform-linux-lightgrey)](https://github.com)

[Features](#features) • [Installation](#installation) • [Usage](#usage) • [Roadmap](#roadmap) • [Contributing](#contributing)

</div>

---

## Overview

A command-line interface built from scratch in **C** for deploying containerized applications with minimal configuration. This tool automates the heavy lifting of modern DevOps workflows — cloning repositories, building Docker images, managing containers, and tracking deployment state.

**Why this exists:** To deeply understand low-level system interactions, process management, and infrastructure automation without relying on high-level frameworks or abstractions.

> **Project Status:** Early-stage (V1) — Active development. Core functionality is being implemented. Not production-ready.

---

## Who Is This For?

| Audience | What You'll Gain |
|----------|------------------|
| Linux Users | Lightweight deployment automation for personal and team projects |
| DevOps Beginners | Hands-on exposure to real infrastructure concepts |
| Systems Programmers | Practical low-level C development in a DevOps context |
| Students | A real-world application of systems programming principles |

---

## Features

### Currently Implemented (V1)

- Robust CLI argument parsing
- Shell command execution via `system()`
- Exit code handling and comprehensive error reporting
- Modular, maintainable C project structure
- Basic logging framework

### Planned

- Git repository cloning and validation
- Docker image building from Dockerfiles
- Container lifecycle management (start, stop, restart)
- Deployment state persistence (JSON-based)
- Structured logging to file
- Rollback to previous deployments
- CI/CD configuration templates
- Remote deployment via SSH
- Multi-container orchestration

---

## Project Structure

```
mytool/
├── src/
│   ├── main.c              # CLI entry point and argument parsing
│   ├── deploy.c            # Deployment orchestration logic
│   ├── deploy.h            # Deployment function declarations
│   ├── utils.c             # Command execution and helper functions
│   ├── utils.h             # Utility function declarations
│   └── config.c            # Configuration file handling
├── scripts/
│   └── docker.sh           # Shell scripts for Docker operations
├── state/
│   └── deployments.json    # Deployment state tracking
├── logs/
│   └── mytool.log          # Application logs
├── Makefile                # Build automation
├── README.md
└── .gitignore
```

---

## Prerequisites

| Requirement | Minimum Version | Purpose |
|-------------|-----------------|---------|
| Linux | Ubuntu 20.04+ / Debian 11+ | Operating system |
| GCC | 7.0+ | C compiler |
| Git | 2.0+ | Version control |
| Docker | 20.0+ | Containerization |
| Make | Any recent | Build automation |

---

## Installation

### Quick Start

```bash
# Clone the repository
git clone https://github.com/yourusername/mytool.git
cd mytool

# Build the project
make
```

### Manual Compilation

```bash
gcc -o mytool src/main.c src/utils.c src/deploy.c -Wall -Wextra
```

---

## Usage

### Deploy an Application

```bash
./mytool deploy https://github.com/user/app.git
```

### Check Version

```bash
./mytool --version
```

### Get Help

```bash
./mytool --help
```

---

## Development Philosophy

### Why C?

C provides direct access to system calls and process management. Building this tool in C serves to:

- Develop practical understanding of memory management and resource allocation
- Learn POSIX APIs and Linux system programming
- Build skills in performance-critical infrastructure code
- Gain experience with compiled languages in a DevOps context

### Why `system()` in V1?

The current implementation uses `system()` for command execution as a pragmatic starting point. This enables rapid prototyping of core workflows before implementing proper process forking, piping, and signal handling with `fork()`, `exec()`, and related syscalls. Future versions will replace `system()` with lower-level process management for improved security, error handling, and control.

### Linux-First Approach

Designed exclusively for Linux environments where Docker and Git are standard tooling. This focus enables deep integration with Linux-specific features without cross-platform complexity.

---

## Current Limitations

| Limitation | Impact |
|------------|--------|
| Early Development | Core features still under implementation |
| Local Only | No remote server or cloud support yet |
| No Kubernetes | Single-host Docker deployments only |
| Limited Recovery | Basic error handling without sophisticated rollback |
| Security Concerns | `system()` usage carries known security implications |
| Not Production-Ready | Suitable for learning and experimentation only |

---

## Roadmap

### V1 — Current

- [x] CLI argument parser
- [x] Command execution framework
- [ ] Git clone integration
- [ ] Docker build automation
- [ ] Container run/stop commands

### V2 — Next

- [ ] Deployment state persistence
- [ ] Structured logging
- [ ] Configuration file support
- [ ] Environment variable management
- [ ] Basic rollback functionality

### V3 — Future

- [ ] SSH-based remote deployment
- [ ] Multi-container application support
- [ ] Health check integration
- [ ] CI/CD template generation
- [ ] Replace `system()` with `fork()`/`exec()`

---

## Contributing

Contributions are welcome. This is a learning-oriented project, and beginner-friendly contributions are encouraged.

### How to Contribute

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Make your changes with clear, descriptive commit messages
4. Test thoroughly on Linux
5. Submit a pull request with a summary of changes

### Coding Standards

- Follow the Linux kernel coding style
- Use meaningful variable and function names
- Comment complex logic clearly
- Handle all errors explicitly
- Avoid memory leaks — use `valgrind` during development

---

## License

**MIT License** — Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## Contact

Questions or suggestions? [Open an issue](https://github.com/yourusername/mytool/issues) on GitHub.
