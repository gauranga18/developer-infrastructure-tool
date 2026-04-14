
<div align="center">

<img src="https://img.shields.io/badge/Forge-CLI-000000?style=for-the-badge&logoColor=white" alt="Forge" />

### Lightweight Deployment Tool for Edge Devices

**70KB. Zero Dependencies. Deploy Anywhere.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C-00599C.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-linux-FCC624?logo=linux&logoColor=black)](https://kernel.org)
[![Size](https://img.shields.io/badge/size-70KB-brightgreen)](https://github.com)

[Overview](#overview) • [Features](#features) • [Installation](#installation) • [Commands](#commands) • [Examples](#examples) • [Architecture](#architecture) • [Contributing](#contributing)

</div>

---

## Overview

**Forge** is a 70KB C binary that deploys Docker containers from Git repositories. It works offline, has zero runtime dependencies, and runs on any Linux device — from cloud servers to edge devices with 32MB of flash storage.

No Python. No Node. No Docker Engine. Just a single binary.

---

## Why Forge?

| Feature | Forge | Docker Compose | Kubernetes | Ansible |
|---------|-------|----------------|------------|---------|
| Binary Size | **70KB** | 200MB+ | 1GB+ | 50MB+ |
| Dependencies | **None** | Docker Engine | Python/Go | Python |
| Offline Support | **Yes** | No | No | No |
| Built-in Rollback | **Yes** | No | Yes | No |
| SSH Remote | **Yes** | No | No | Yes |
| Auto Cleanup | **Yes** | No | No | No |

---

## Features

### Core Deployment
- Deploy from any Git repository
- Interactive (`-i`) and detached (`-d`) modes
- Version tracking (auto-incrementing v1, v2, v3...)
- One-command rollback
- Real-time container logs
- Deployment status with container details

### Offline & Caching
- Repository caching — clone once, deploy offline
- Docker image caching — build once, deploy offline
- `--offline` flag for air-gapped environments

### Remote Management
- SSH remote deployment — `forge ssh user@host deploy repo.git -d`
- Remote status, logs, and rollback

### Self-Contained Bundles
- `forge bundle` — create a single `.tar.gz` with everything
- Contains: git bundle, docker image, forge binary, manifest
- Deploy on air-gapped machines with no internet

### Deployment Comparison
- `forge diff` — show what changed between versions
- Git diff, commit count, file statistics
- Metadata diff (deploy time, image size)
- JSON output for scripting

### Maintenance
- Auto cleanup of stopped containers
- Remove old deployments (`--keep N`, `--older-than N`)
- Prune unused Docker images (`--prune-images`)
- Dry-run mode to preview deletions

### Project Scaffolding
- 5 language templates: Python, Node.js, Go, Rust, C
- Dockerfile generation per language
- GitHub Actions CI workflow (`--ci github`)

### User Experience
- Smooth progress bars with auto terminal detection
- Structured logging with levels (DEBUG, INFO, WARN, ERROR)
- XDG-compliant storage (`~/.local/state/forge/`)
- Proper exit codes for scripting

---

## Installation

### From Source

git clone https://github.com/gauranga18/forge.git
cd forge/src
make release
sudo cp forge /usr/local/bin/


### Verify Installation

forge -v
forge --help


### Requirements
| Requirement | Version |
|-------------|---------|
| Linux | Ubuntu 20.04+ / Debian 11+ |
| Docker | 20.10+ |
| Git | 2.25+ |
| SSH | For remote deployment |

---

## Commands

### Initialization

forge init <project_name> [--type python|node|go|rust|c] [--ci github]


### Deployment

forge deploy <repo_url> [-i|-d] [--offline]
forge deploy <bundle.tar.gz> [-i|-d]


### Management

forge --list                    # List all deployments
forge --status <project>        # Show container status
forge --logs <project>          # Stream container logs
forge --rollback <project>      # Rollback to previous version

### Bundles

forge bundle <project> [--version vN] [-o output] [--ship user@host] [--no-binary] [--dry-run]


### Diff

forge diff <project> [--patch] [--meta-only] [--json]
forge diff <project> -v <v1> <v2>


### Cleanup

forge cleanup [--dry-run] [--keep N] [--older-than N] [--prune-images] [--all]


### Remote

forge ssh user@host <command>
forge ssh user@host deploy <url> -d
forge ssh user@host --status <project>
forge ssh user@host --rollback <project>


### Global Flags
| Flag | Description |
|------|-------------|
| -v | Show version |
| -h, --help | Show help |
| -ver, --verbose | Enable debug logging |
| -q, --quiet | Suppress info/warnings |



## Examples

### Initialize and Deploy a Python App

forge init myapp --type python
cd myapp
# Write your code...
forge deploy . -d


### Deploy from GitHub

# First deployment (online)
forge deploy https://github.com/user/repo.git -d

# Second deployment (offline - uses cache)
forge deploy https://github.com/user/repo.git -d --offline


### Rollback a Bad Deployment

forge --list
# myapp-v1 (running), myapp-v2 (crashed)
forge --rollback myapp
# Rolled back to v1


### Create and Deploy a Bundle

forge bundle myapp
scp myapp-*.tar.gz user@offline-server:/tmp/
ssh user@offline-server "forge deploy /tmp/myapp-*.tar.gz -d"


### Compare Deployments

forge diff myapp
forge diff myapp -v 2 3 --patch
forge diff myapp --json


### Clean Up Old Deployments

forge cleanup --dry-run
forge cleanup --keep 3 --prune-images


### Remote Deployment

forge ssh user@server deploy https://github.com/user/repo.git -d
forge ssh user@server --status myapp


---

## Architecture

### Storage (XDG Compliant)

~/.local/state/forge/
├── deployments/          # JSON files per deployment
├── current/              # Symlinks to latest versions
├── projects/             # Project registry (next_version)
├── cache/
│   ├── repos/            # Cloned repositories
│   └── images/           # Docker image tarballs
└── forge.log             # Application logs


### Process Management
Forge uses `fork()` and `execvp()` directly, avoiding `system()` for security and control:

c
pid_t pid = fork();
if (pid == 0) {
    execvp(argv[0], (char **)argv);
    exit(127);
}
waitpid(pid, &status, 0);


### State Persistence
Deployments are stored as JSON with git metadata:

json
{
  "id": "myapp-v1-20260414-120000",
  "project": "myapp-v1",
  "project_base": "myapp",
  "git_sha": "8fb97e9cddb4...",
  "git_branch": "main",
  "image_size_kb": 48320,
  "version": 1,
  "status": 1
}


---

## Environment Variables

| Variable | Purpose | Default |
|----------|---------|---------|
| `FORGE_REGISTRY` | Default Docker registry | docker.io |
| `FORGE_LOG_LEVEL` | Log level (debug, info, warn, error) | info |
| `XDG_STATE_HOME` | Override state directory | ~/.local/state |
| `XDG_CONFIG_HOME` | Override config directory | ~/.config |

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Generic error |
| 2 | Invalid arguments |
| 3 | Network error |
| 4 | Authentication failed |
| 5 | Resource not found |
| 6 | Internal error |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `forge: command not found` | `sudo cp forge /usr/local/bin/` |
| Docker connection error | `sudo systemctl start docker` |
| Offline mode fails | Run once online first to cache |
| Permission denied | `sudo chown -R $USER ~/.local/state/forge` |
| Container name conflict | `docker rm -f <container-name>` |

---

## Contributing

Contributions are welcome.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on Linux
5. Submit a pull request

### Coding Standards
- Linux kernel coding style
- Comment non-obvious logic
- Handle errors explicitly
- Run `valgrind` to check for leaks

---

## License

MIT License — see [LICENSE](LICENSE) file for details.

---

<div align="center">

**Built with C. Made for the edge.**

[GitHub](https://github.com/gauranga18/forge) • [Issues](https://github.com/gauranga18/forge/issues)

</div>
```
