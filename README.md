# 🛠️ Developer Infrastructure Tool

![Build Status](https://img.shields.io/badge/build-passing-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-blue?style=for-the-badge)
![Language](https://img.shields.io/badge/language-C-orange?style=for-the-badge)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey?style=for-the-badge)

> A lightweight CLI tool for automating deployment of Dockerized applications from Git repositories on Linux systems. 
> Built from scratch in **C** to explore systems programming, Linux internals, and DevOps workflows.

---

## 🚀 Overview

This project provides a command-line interface for deploying containerized applications with **minimal configuration**. It automates common DevOps tasks without relying on high-level frameworks, giving you a peek under the hood of infrastructure automation.

### 🎯 Target Audience
- **Linux Users** seeking lightweight automation.
- **DevOps Beginners** learning infrastructure concepts.
- **Systems Programmers** building practical tools.
- **Students** interested in real-world C applications.

---

## ✨ Features

### ⚡ Current (V1)
| Feature | Description |
| :--- | :--- |
| **CLI Argument Parsing** | robust command-line interface using C. |
| **Command Execution** | wrapper around `system()` for shell commands. |
| **Error Handling** | clean exit codes and error reporting. |
| **Modular Structure** | organized C project with header/source separation. |
| **Logging** | basic logging framework for debugging. |

### 🔮 Planned
- [ ] **Git Integration**: Clone and validate repositories.
- [ ] **Docker Builds**: Automate `docker build` from Dockerfiles.
- [ ] **Lifecycle Management**: Start, stop, and restart containers.
- [ ] **State Persistence**: JSON-based deployment tracking.
- [ ] **Rollback**: Revert to previous stable deployments.
- [ ] **Remote**: SSH-based remote deployment.

---

## 📂 Project Structure

```bash
mytool/
├── src/
│   ├── main.c        # CLI entry point
│   ├── deploy.c      # Deployment logic
│   ├── deploy.h      # Deployment headers
│   ├── utils.c       # Helpers
│   ├── utils.h       # Utility headers
│   └── config.c      # Config handling
├── scripts/
│   └── docker.sh     # Docker scripts
├── state/
│   └── deployments.json # State tracking
├── logs/
│   └── mytool.log    # Application logs
├── Makefile          # Build system
├── README.md
└── .gitignore
```

---

## 🛠️ Prerequisites

Before you begin, ensure you have the following installed on your **Linux** system (Ubuntu 20.04+, Debian 11+):

- **GCC Compiler** (v7.0+)
- **Git** (v2.0+)
- **Docker** (v20.0+)
- **Make** utility

---

## 📦 Installation

### Option 1: Build from Source

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/yourusername/mytool.git
    cd mytool
    ```

2.  **Compile the project:**
    ```bash
    make
    ```
    *The binary `mytool` will be created in the project root.*

### Option 2: Manual Compilation

If `make` is unavailable:
```bash
gcc -o mytool src/main.c src/utils.c src/deploy.c -Wall -Wextra
```

---

## 💻 Usage

### Basic Deployment
Deploy an application directly from a Git repository:

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

---

## 🧠 Development Philosophy

### Why C?
We chose **C** to gain direct access to system calls and process management. It forces a deeper understanding of memory management, POSIX APIs, and performance-critical infrastructure code.

### Why `system()` in V1?
V1 uses `system()` for rapid prototyping. Future versions will migrate to `fork()`, `exec()`, and signal handling for robust process control and security.

### Linux-First
Designed for Linux environments where Docker and Git are native citizens, avoiding cross-platform compatibility layers to focus on depth.

---

## 🗺️ Roadmap

- **V1 (Current)**: CLI, Command Execution
- **V2**: State Persistence, JSON logging, Config files, Rollbacks
- **V3**: SSH Remote deploy, Multi-container, Health checks, `fork()`/`exec()`

---

## 🤝 Contributing

This is a learning project! Contributions are welcome.

1.  Fork the repo.
2.  Create a feature branch: `git checkout -b feature/cool-feature`
3.  Commit changes.
4.  Test with `valgrind` for memory leaks.
5.  Open a Pull Request.

---

## 📄 License

Distributed under the MIT License. See `LICENSE` for more information.

---

<p align="center">
  Made with ❤️ for Systems Programming
</p>
