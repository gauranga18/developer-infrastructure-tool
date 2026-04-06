#ifndef UTILS_H
#define UTILS_H

#include <signal.h>

int run_command(const char **argv);
int run_command_fork(const char **argv);

// File locking helpers
int lock_file(const char *path);
void unlock_file(int fd);

// Signal handling
void setup_signal_handlers(void);
void cleanup_child_processes(void);
void add_child_pid(pid_t pid);
void remove_child_pid(pid_t pid);
void set_cleanup_flag(int flag);

// Global flag for cleanupf
extern volatile sig_atomic_t g_cleaning_up;

//copy a file from src to dst
int copy_file(const char *src, const char *dst);

// File utilities
int copy_file(const char *src, const char *dst);
long file_size_kb(const char *path);
long dir_size_kb(const char *path);
void sha256_file(const char *path, char *out_hex);
int mkdir_p(const char *path);

#endif