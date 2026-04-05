#ifndef PROGRESS_H
#define PROGRESS_H

#include <time.h>
#include <stdbool.h>
#include <pthread.h>

// ANSI escape codes
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"
#define CLEAR_LINE    "\033[2K\r"
#define HIDE_CURSOR   "\033[?25l"
#define SHOW_CURSOR   "\033[?25h"

int  is_terminal(void);
void format_time(char *buffer, size_t size, time_t start_time);

// ── Progress Bar (determinate) ───────────────────────────────────────────────

typedef struct {
    char        *message;
    int          percent;           // 0–100, written under lock
    time_t       start_time;
    bool         completed;
    int          step_number;
    int          total_steps;
    int          bar_width;
    char         extra_info[128];   // shown after the bar, updated under lock

    pthread_t       render_thread;
    pthread_mutex_t lock;
    bool            thread_running;
} ProgressBar;

// Create bar and start its background render thread.
// Pass step=0 / total_steps=0 to hide the "[n/m]" prefix.
void progress_init    (ProgressBar *pb, const char *message, int step, int total_steps);

// Thread-safe: set percent (0-100) and an optional status string.
void progress_update  (ProgressBar *pb, int percent, const char *extra_info);

// Convenience wrapper: progress_update(pb, current*100/total, extra_info).
void progress_auto    (ProgressBar *pb, int current, int total, const char *extra_info);

// Stop the thread, print final "✓ … complete" line.
void progress_complete(ProgressBar *pb, const char *extra_info);

// Stop the thread, print final "✗ … failed" line.
void progress_fail    (ProgressBar *pb, const char *error);

// Free resources.  Always call this when done with the bar.
void progress_destroy (ProgressBar *pb);


// ── Spinner (indeterminate) ──────────────────────────────────────────────────

typedef struct {
    char            message[256];
    bool            done;
    bool            failed;
    pthread_t       thread;
    pthread_mutex_t lock;
} Spinner;

// Start spinning immediately in a background thread.
void spinner_start   (Spinner *sp, const char *message);

// Stop thread, print "✓ message (extra_info)".
void spinner_complete(Spinner *sp, const char *extra_info);

// Stop thread, print "✗ message: error".
void spinner_fail    (Spinner *sp, const char *error);

#endif // PROGRESS_H