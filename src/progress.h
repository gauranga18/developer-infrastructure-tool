#ifndef PROGRESS_H
#define PROGRESS_H

#include <time.h>
#include <stdbool.h>

// ANSI color codes
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"
#define CLEAR_LINE    "\033[2K\r"

// Check if terminal supports ANSI colors
int is_terminal(void);

typedef struct {
    char *message;
    int current;
    int total;
    time_t start_time;
    bool completed;
    int step_number;
    int total_steps;
    int bar_width;
    struct timespec last_update;
} ProgressBar;

// Initialize a progress bar
void progress_init(ProgressBar *pb, const char *message, int step, int total_steps);

// Update progress (0-100) - smooth animation
void progress_update(ProgressBar *pb, int percent, const char *extra_info);

// Mark as completed
void progress_complete(ProgressBar *pb, const char *extra_info);

// Mark as failed
void progress_fail(ProgressBar *pb, const char *error);

// Simple spinner for indeterminate operations
void progress_spinner(const char *message, int done);

// Format time (seconds -> "2.3s" or "1m 23s")
void format_time(char *buffer, size_t size, time_t start_time);

// Auto progress - call this in loops to automatically update
void progress_auto(ProgressBar *pb, int current, int total, const char *extra_info);

#endif