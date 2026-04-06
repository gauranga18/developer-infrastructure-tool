#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "utils.h"
#include "log.h"

// Global list of child PIDs to clean up
static pid_t child_pids[128];
static int child_count = 0;
volatile sig_atomic_t g_cleaning_up = 0;

// Add a child PID to track
void add_child_pid(pid_t pid) {
    if (child_count < 128 && pid > 0) {
        child_pids[child_count++] = pid;
        log_debug("Tracking child PID: %d", pid);
    }
}

// Remove a child PID from tracking
void remove_child_pid(pid_t pid) {
    for (int i = 0; i < child_count; i++) {
        if (child_pids[i] == pid) {
            for (int j = i; j < child_count - 1; j++) {
                child_pids[j] = child_pids[j + 1];
            }
            child_count--;
            log_debug("Removed child PID: %d", pid);
            break;
        }
    }
}

// Clean up all child processes
void cleanup_child_processes(void) {
    if (g_cleaning_up) return;
    g_cleaning_up = 1;
    
    log_info("Cleaning up child processes...");
    
    for (int i = 0; i < child_count; i++) {
        pid_t pid = child_pids[i];
        if (pid > 0) {
            log_info("Terminating child PID: %d", pid);
            kill(pid, SIGTERM);
            usleep(100000);
            kill(pid, SIGKILL);
            waitpid(pid, NULL, WNOHANG);
        }
    }
    
    child_count = 0;
    log_info("Cleanup complete");
}

// Signal handler for SIGINT (Ctrl+C)
static void sigint_handler(int sig) {
    (void)sig;
    log_info("Received interrupt signal, cleaning up...");
    cleanup_child_processes();
    exit(130);
}

// Signal handler for SIGTERM
static void sigterm_handler(int sig) {
    (void)sig;
    log_info("Received termination signal, cleaning up...");
    cleanup_child_processes();
    exit(143);
}

// Setup signal handlers
void setup_signal_handlers(void) {
    struct sigaction sa_int = {0};
    struct sigaction sa_term = {0};
    
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    
    sa_term.sa_handler = sigterm_handler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = SA_RESTART;
    
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGTERM, &sa_term, NULL);
    
    log_debug("Signal handlers installed");
}

// Acquire exclusive lock on a file
int lock_file(const char *path) {
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        log_error("Failed to open lock file %s: %s", path, strerror(errno));
        return -1;
    }
    
    if (flock(fd, LOCK_EX) == -1) {
        log_error("Failed to acquire lock on %s: %s", path, strerror(errno));
        close(fd);
        return -1;
    }
    
    return fd;
}

// Release lock and close file
void unlock_file(int fd) {
    if (fd != -1) {
        flock(fd, LOCK_UN);
        close(fd);
    }
}

int run_command_fork(const char **argv) {
    pid_t pid = fork();
    if (pid == -1) {
        log_error("Fork failed: %s", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        // Child process - reset signal handlers
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        execvp(argv[0], (char **)argv);
        log_error("exec failed for %s: %s", argv[0], strerror(errno));
        exit(127);
    }
    
    // Parent - track child
    add_child_pid(pid);
    
    int status;
    waitpid(pid, &status, 0);
    
    // Remove from tracking when done
    remove_child_pid(pid);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}
// Copy a file from src to dst
int copy_file(const char *src, const char *dst) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null", src, dst);
    return system(cmd);
}

long file_size_kb(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size / 1024;
    }
    return 0;
}

long dir_size_kb(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "du -sk %s 2>/dev/null | cut -f1", path);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    long size;
    fscanf(fp, "%ld", &size);
    pclose(fp);
    return size;
}

void sha256_file(const char *path, char *out_hex) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sha256sum %s 2>/dev/null | cut -d' ' -f1", path);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        fgets(out_hex, 64, fp);
        pclose(fp);
    }
}

int mkdir_p(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    return system(cmd);
}