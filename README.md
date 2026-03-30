<div align="center">

<img src="https://img.shields.io/badge/Forge-CLI-000000?style=for-the-badge&logoColor=white" alt="Forge" />

### Developer Infrastructure Tool

A lightweight CLI written in C for automating Dockerized deployments — from project scaffolding to remote deployment and rollback.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C-00599C.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-linux-FCC624?logo=linux&logoColor=black)](https://kernel.org)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com)

[Overview](#overview) • [Features](#features) • [Installation](#installation) • [Usage](#usage) • [Philosophy](#development-philosophy) • [Contributing](#contributing)

</div>

---

## Overview

**Forge** is a command-line deployment tool built from scratch in C. It handles the full lifecycle of containerized applications — initializing project scaffolding, cloning repositories, building and running Docker containers, tracking deployment state, viewing logs, rolling back versions, and deploying to remote servers over SSH.

Built without high-level frameworks or abstractions, Forge is a ground-up systems project designed for developers who want direct control over their infrastructure tooling.

---

## Who Is This For?

| Audience | What You Get |
|----------|--------------|
| Linux Developers | Fast, scriptable deployment automation with no runtime dependencies |
| DevOps Engineers | A self-contained CLI covering the full deploy-manage-rollback loop |
| Systems Programmers | A real-world C project using POSIX, process management, and SSH |
| Students | A complete, working reference for low-level infrastructure tooling |

---

## Features

- Project initialization with language templates (Python, Node, Go, Rust, C)
- GitHub Actions CI workflow generation
- Git repository cloning and Docker image building
- Container lifecycle management — start, stop, restart, interactive and detached modes
- Deployment state persistence and version tracking
- Structured logging with verbosity control
- Rollback to previous deployment versions
- Cleanup tooling with dry-run, age-based, and count-based filtering
- Remote deployment over SSH
- Quiet and verbose output modes

---

## Project Structure
```
forge/
├── src/
│   ├── main.c              # CLI entry point and argument parsing
│   ├── deploy.c            # Deployment orchestration logic
│   ├── deploy.h
│   ├── utils.c             # Command execution and helpers
│   ├── utils.h
│   ├── help.c              # Help output
│   ├── help.h
│   └── config.c            # Configuration handling
├── scripts/
│   └── docker.sh           # Docker operation scripts
├── state/
│   └── deployments.json    # Deployment state tracking
├── logs/
│   └── forge.log           # Application logs
├── Makefile
├── README.md
└── .gitignore
```

---

## Prerequisites

| Requirement | Minimum Version | Purpose |
|-------------|-----------------|---------|
| Linux | Ubuntu 20.04+ / Debian 11+ | Operating system |
| GCC | 7.0+ | C compiler |
| Git | 2.0+ | Repository cloning |
| Docker | 20.0+ | Containerization |
| Make | Any recent | Build automation |
| OpenSSH | Any recent | Remote deployment |

---

## Installation

### Quick Start
```bash
git clone https://github.com/yourusername/forge.git
cd forge
make
```

### Manual Compilation
```bash
gcc -o forge src/main.c src/utils.c src/deploy.c src/help.c src/config.c -Wall -Wextra
```

---

## Usage

### Initialize a Project
```bash
forge init <project_name>
forge init myapp --type node
forge init myapp --type python --ci github
```

Supported languages: `python`, `node`, `go`, `rust`, `c`

### Deploy a Repository
```bash
forge deploy https://github.com/user/repo.git
forge deploy https://github.com/user/repo.git -d    # detached mode
forge deploy https://github.com/user/repo.git -i    # interactive mode
```

### Manage Deployments
```bash
forge --list                      # List all deployments
forge --status <project>          # Show deployment status
forge --logs <project>            # View container logs
forge --rollback <project>        # Rollback to previous version
```

### Cleanup
```bash
forge cleanup --dry-run                      # Preview what would be removed
forge cleanup --keep 3                       # Keep last 3 versions per project
forge cleanup --older-than 7                 # Remove deployments older than 7 days
forge cleanup --prune-images                 # Remove unused Docker images
forge cleanup --all                          # Remove everything except current deployment
forge cleanup --keep 3 --dry-run             # Combined
forge cleanup --older-than 7 --prune-images  # Combined
```

### Remote Deployment via SSH
```bash
forge ssh user@host deploy https://github.com/user/repo.git
forge ssh user@host --status myapp
forge ssh user@host --rollback myapp
```

### Global Flags

| Flag | Description |
|------|-------------|
| `-v` | Show Forge version |
| `-h`, `--help` | Show help message |
| `-ver`, `--verbose` | Enable debug logging |
| `-q`, `--quiet` | Suppress info and warnings, show errors only |

---

## Development Philosophy

### Why C ?

C gives direct access to system calls, process management, and memory — exactly what a deployment tool needs. There are no runtime dependencies, no garbage collector pauses, and no framework abstractions hiding what the tool actually does. Every operation is explicit.

### Linux-First

Forge targets Linux environments where Docker, Git, and SSH are standard. This focus allows deep integration with Linux-specific features — `fork`, `exec`, POSIX signals, and process groups — without cross-platform compromises.

### Process Management

Early versions used `system()` for rapid prototyping of core workflows. The current implementation moves to proper `fork()`/`exec()` based process management for improved security, signal handling, and output control.

---

## Roadmap

### V1 — Foundation

- [x] CLI argument parser
- [x] Command execution framework
- [x] Git clone integration
- [x] Docker build automation
- [x] Container run/stop commands

### V2 — State and Observability

- [x] Deployment state persistence (JSON)
- [x] Structured logging with log levels
- [x] Configuration file support
- [x] Environment variable management
- [x] Rollback functionality

### V3 — Remote and Orchestration

- [x] SSH-based remote deployment
- [x] Multi-container application support
- [x] Health check integration
- [x] CI/CD template generation (GitHub Actions)
- [x] Replaced `system()` with `fork()`/`exec()`

---

## Contributing

Contributions are welcome. This project is built to be readable and approachable — beginner contributions are encouraged.

**Steps to contribute:**

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Make your changes with clear commit messages
4. Test on Linux
5. Open a pull request with a description of your changes

**Coding standards:**

- Linux kernel coding style
- Meaningful variable and function names
- Comment non-obvious logic
- Explicit error handling on every syscall
- Run `valgrind` to catch memory leaks before submitting

---

## License

MIT License — Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

---

<div align="center">

Questions or suggestions? [Open an issue](https://github.com/yourusername/forge/issues)

</div>
