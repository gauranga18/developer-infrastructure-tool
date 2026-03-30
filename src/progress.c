#include "progress.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int g_use_colors = 1;
static int g_is_terminal = -1;

int is_terminal(void) {
    if (g_is_terminal == -1) {
        g_is_terminal = isatty(STDOUT_FILENO);
    }
    return g_is_terminal;
}

void progress_init(ProgressBar *pb, const char *message, int step, int total_steps) {
    pb->message = (char*)message;
    pb->current = 0;
    pb->total = 100;
    pb->start_time = time(NULL);
    pb->completed = false;
    pb->step_number = step;
    pb->total_steps = total_steps;
    pb->bar_width = 40;
    clock_gettime(CLOCK_MONOTONIC, &pb->last_update);
    
    if (is_terminal()) {
        printf("%s", CLEAR_LINE);
        if (step > 0) {
            printf("[%d/%d] %s... ", step, total_steps, message);
        } else {
            printf("%s... ", message);
        }
        fflush(stdout);
    }
}

void progress_auto(ProgressBar *pb, int current, int total, const char *extra_info) {
    if (pb->completed || !is_terminal()) return;
    
    int percent = (current * 100) / total;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // Update at most 30 times per second for smooth animation
    long diff_ns = (now.tv_sec - pb->last_update.tv_sec) * 1000000000L +
                   (now.tv_nsec - pb->last_update.tv_nsec);
    
    if (diff_ns < 33000000 && percent != 100) {  // ~30 FPS max
        return;
    }
    
    pb->last_update = now;
    progress_update(pb, percent, extra_info);
}

void progress_update(ProgressBar *pb, int percent, const char *extra_info) {
    if (pb->completed || !is_terminal()) return;
    
    // Clamp percent between 0 and 100
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    pb->current = percent;
    int filled = (percent * pb->bar_width) / 100;
    
    printf("%s", CLEAR_LINE);
    
    if (pb->step_number > 0) {
        printf("[%d/%d] ", pb->step_number, pb->total_steps);
    }
    
    printf("%s... [", pb->message);
    
    // Draw smooth progress bar
    for (int i = 0; i < pb->bar_width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf("░");
        }
    }
    
    printf("] %3d%%", percent);
    
    if (extra_info && strlen(extra_info) > 0) {
        printf(" (%s)", extra_info);
    } else {
        char elapsed[32];
        format_time(elapsed, sizeof(elapsed), pb->start_time);
        printf(" (%s)", elapsed);
    }
    
    fflush(stdout);
}

void progress_complete(ProgressBar *pb, const char *extra_info) {
    if (pb->completed) return;
    pb->completed = true;
    
    if (is_terminal()) {
        printf("%s", CLEAR_LINE);
        
        if (pb->step_number > 0) {
            printf("[%d/%d] ", pb->step_number, pb->total_steps);
        }
        
        if (g_use_colors) {
            printf("%s✓%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf("✓");
        }
        
        printf(" %s complete", pb->message);
        
        if (extra_info && strlen(extra_info) > 0) {
            printf(" (%s)", extra_info);
        } else {
            char elapsed[32];
            format_time(elapsed, sizeof(elapsed), pb->start_time);
            printf(" (%s)", elapsed);
        }
        
        printf("\n");
        fflush(stdout);
    }
}

void progress_fail(ProgressBar *pb, const char *error) {
    pb->completed = true;
    
    if (is_terminal()) {
        printf("%s", CLEAR_LINE);
        
        if (pb->step_number > 0) {
            printf("[%d/%d] ", pb->step_number, pb->total_steps);
        }
        
        if (g_use_colors) {
            printf("%s✗%s", COLOR_RED, COLOR_RESET);
        } else {
            printf("✗");
        }
        
        printf(" %s failed", pb->message);
        
        if (error && strlen(error) > 0) {
            printf(": %s", error);
        }
        
        printf("\n");
        fflush(stdout);
    }
}

void progress_spinner(const char *message, int done) {
    static const char spinner[] = {'|', '/', '-', '\\'};
    static int idx = 0;
    
    if (!is_terminal()) {
        if (done) {
            printf("%s\n", message);
        }
        return;
    }
    
    printf("%s", CLEAR_LINE);
    
    if (done) {
        if (g_use_colors) {
            printf("%s✓%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf("✓");
        }
        printf(" %s\n", message);
    } else {
        printf("[%c] %s...", spinner[idx % 4], message);
        idx++;
    }
    
    fflush(stdout);
}

void format_time(char *buffer, size_t size, time_t start_time) {
    time_t now = time(NULL);
    int elapsed = (int)(now - start_time);
    
    if (elapsed < 60) {
        snprintf(buffer, size, "%.1fs", (float)elapsed);
    } else {
        int minutes = elapsed / 60;
        int seconds = elapsed % 60;
        snprintf(buffer, size, "%dm %ds", minutes, seconds);
    }
}