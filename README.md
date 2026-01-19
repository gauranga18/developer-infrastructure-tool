# Developer Infrastructure Tool

A lightweight CLI tool for automating deployment of Dockerized applications from Git repositories on Linux systems. Written in C for learning systems programming, Linux internals, and DevOps workflows.

## Project Status

**Early-stage (V1)** - Active development. Core functionality is being implemented. Not production-ready.

## Overview

This project provides a command-line interface for deploying containerized applications with minimal configuration. The tool automates common DevOps tasks: cloning repositories, building Docker images, running containers, and tracking deployment state.

Built from scratch in C to understand low-level system interactions, process management, and infrastructure automation without relying on high-level frameworks.

## Target Audience

- Linux users seeking lightweight deployment automation
- DevOps beginners learning infrastructure concepts
- Systems programming learners building practical tools
- Students interested in real-world C applications

## Features

### Current (V1)

- CLI argument parsing
- Shell command execution via `system()`
- Exit code handling and error reporting
- Modular C project structure
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

## Project Structure
```
mytool/
├── src/
│   ├── main.c        # CLI entry point and argument parsing
│   ├── deploy.c      # Deployment orchestration logic
│   ├── deploy.h      # Deployment function declarations
│   ├── utils.c       # Command execution and helper functions
│   ├── utils.h       # Utility function declarations
│   └── config.c      # Configuration file handling
├── scripts/
│   └── docker.sh     # Shell scripts for Docker operations
├── state/
│   └── deployments.json  # Deployment state tracking
├── logs/
│   └── mytool.log    # Application logs
├── Makefile          # Build automation
├── README.md
└── .gitignore
```

## Prerequisites

- Linux operating system (Ubuntu 20.04+, Debian 11+, or similar)
- GCC compiler (version 7.0 or higher)
- Git (version 2.0+)
- Docker (version 20.0+)
- Make utility

## Installation

### Building from Source

Clone the repository:
```bash
git clone https://github.com/yourusername/mytool.git
cd mytool
```

Compile the project:
```bash
make
```

The binary will be created as `mytool` in the project root.

### Manual Compilation

If Make is unavailable:
```bash
gcc -o mytool src/main.c src/utils.c src/deploy.c -Wall -Wextra
```

## Usage

### Basic Deployment

Deploy an application from a Git repository:
```bash
./mytool deploy https://github.com/user/app.git
```

### Check Version
```bash
./mytool --version
```

### Help
```bash
./mytool --help
```

## Development Philosophy

### Why C?

C provides direct access to system calls and process management without abstractions. This project uses C to:

- Understand memory management and resource allocation
- Learn POSIX APIs and Linux system programming
- Build skills in performance-critical infrastructure code
- Gain experience with compiled languages in DevOps contexts

### Why `system()` in V1?

The current implementation uses `system()` for command execution as a pragmatic starting point. This allows rapid prototyping of core workflows before implementing proper process forking, piping, and signal handling with `fork()`, `exec()`, and related syscalls.

Future versions will replace `system()` with lower-level process management for better security, error handling, and control.

### Linux-First Approach

The tool is designed for Linux environments where Docker and Git are standard. This focus allows deep integration with Linux-specific features without cross-platform complexity.

## Limitations

- **Early Development**: Core features still under implementation
- **Local Deployment Only**: No remote server or cloud support yet
- **No Kubernetes**: Single-host Docker deployments only
- **Limited Error Recovery**: Basic error handling without sophisticated rollback
- **Security**: Uses `system()` which has security implications
- **Not Production-Ready**: Suitable for learning and experimentation only

## Roadmap

### V1 (Current)

- [x] CLI argument parser
- [x] Command execution framework
- [x] Git clone integration
- [ ] Docker build automation
- [ ] Container run/stop commands

### V2

- [ ] Deployment state persistence
- [ ] Structured logging
- [ ] Configuration file support
- [ ] Environment variable management
- [ ] Basic rollback functionality

### V3

- [ ] SSH-based remote deployment
- [ ] Multi-container applications
- [ ] Health check integration
- [ ] CI/CD template generation
- [ ] Replace `system()` with `fork()`/`exec()`

## Contributing

Contributions are welcome. This is a learning project, so beginner-friendly contributions are encouraged.

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Make your changes with clear commit messages
4. Test thoroughly on Linux
5. Submit a pull request with description of changes

### Coding Standards

- Follow Linux kernel coding style
- Use meaningful variable names
- Comment complex logic
- Handle errors explicitly
- Avoid memory leaks (use `valgrind` for testing)

## License

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Contact

For questions or suggestions, open an issue on GitHub.
