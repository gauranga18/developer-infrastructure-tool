#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// ── Terminal detection ────────────────────────────────────────────────────────

static int g_is_terminal = -1;

int is_terminal(void) {
    if (g_is_terminal == -1)
        g_is_terminal = isatty(STDOUT_FILENO);
    return g_is_terminal;
}

// ── Time formatting ───────────────────────────────────────────────────────────

void format_time(char *buffer, size_t size, time_t start_time) {
    // difftime gives a double, so "%.1fs" is now accurate
    double elapsed = difftime(time(NULL), start_time);
    if (elapsed < 60.0) {
        snprintf(buffer, size, "%.1fs", elapsed);
    } else {
        int m = (int)elapsed / 60;
        int s = (int)elapsed % 60;
        snprintf(buffer, size, "%dm %ds", m, s);
    }
}

// ── Internal: draw one bar frame (call with lock held) ───────────────────────

static void draw_bar(ProgressBar *pb) {
    int percent = pb->percent;
    int filled  = (percent * pb->bar_width) / 100;

    printf("%s", CLEAR_LINE);

    if (pb->step_number > 0)
        printf("[%d/%d] ", pb->step_number, pb->total_steps);

    printf("%s [", pb->message);

    for (int i = 0; i < pb->bar_width; i++)
        fputs(i < filled ? "█" : "░", stdout);

    printf("] %3d%%", percent);

    if (pb->extra_info[0]) {
        printf(" (%s)", pb->extra_info);
    } else {
        char elapsed[32];
        format_time(elapsed, sizeof(elapsed), pb->start_time);
        printf(" (%s)", elapsed);
    }

    fflush(stdout);
}

// ── Background render thread: redraws the bar ~15fps ─────────────────────────

static void *render_thread_fn(void *arg) {
    ProgressBar *pb = (ProgressBar *)arg;
    struct timespec frame = { 0, 66666667L }; // ~15 fps

    while (1) {
        pthread_mutex_lock(&pb->lock);
        bool done = pb->completed;
        if (!done)
            draw_bar(pb);
        pthread_mutex_unlock(&pb->lock);

        if (done) break;
        nanosleep(&frame, NULL);
    }
    return NULL;
}

// ── Progress bar public API ───────────────────────────────────────────────────

void progress_init(ProgressBar *pb, const char *message, int step, int total_steps) {
    memset(pb, 0, sizeof(*pb));
    pb->message       = strdup(message);   // own a copy so caller can free theirs
    pb->percent       = 0;
    pb->start_time    = time(NULL);
    pb->step_number   = step;
    pb->total_steps   = total_steps;
    pb->bar_width     = 40;
    pb->completed     = false;
    pb->thread_running = false;

    pthread_mutex_init(&pb->lock, NULL);

    if (!is_terminal()) return;

    printf("%s", HIDE_CURSOR);
    fflush(stdout);

    pb->thread_running = true;
    pthread_create(&pb->render_thread, NULL, render_thread_fn, pb);
}

void progress_update(ProgressBar *pb, int percent, const char *extra_info) {
    if (pb->completed) return;

    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;

    pthread_mutex_lock(&pb->lock);
    pb->percent = percent;
    if (extra_info && extra_info[0])
        snprintf(pb->extra_info, sizeof(pb->extra_info), "%s", extra_info);
    else
        pb->extra_info[0] = '\0';
    pthread_mutex_unlock(&pb->lock);
}

void progress_auto(ProgressBar *pb, int current, int total, const char *extra_info) {
    if (pb->completed || total == 0) return;
    // Cast to long to avoid overflow before the multiply
    int percent = (int)((long)current * 100L / total);
    progress_update(pb, percent, extra_info);
}

void progress_complete(ProgressBar *pb, const char *extra_info) {
    if (pb->completed) return;

    pthread_mutex_lock(&pb->lock);
    pb->completed = true;
    pb->percent   = 100;
    pthread_mutex_unlock(&pb->lock);

    if (pb->thread_running) {
        pthread_join(pb->render_thread, NULL);
        pb->thread_running = false;
    }

    if (!is_terminal()) return;

    printf("%s", CLEAR_LINE);
    if (pb->step_number > 0)
        printf("[%d/%d] ", pb->step_number, pb->total_steps);

    printf("%s✓%s %s complete", COLOR_GREEN, COLOR_RESET, pb->message);

    if (extra_info && extra_info[0]) {
        printf(" (%s)", extra_info);
    } else {
        char elapsed[32];
        format_time(elapsed, sizeof(elapsed), pb->start_time);
        printf(" (%s)", elapsed);
    }

    printf("\n%s", SHOW_CURSOR);
    fflush(stdout);
}

void progress_fail(ProgressBar *pb, const char *error) {
    if (pb->completed) return;   // guard against double-fail (was missing before)

    pthread_mutex_lock(&pb->lock);
    pb->completed = true;
    pthread_mutex_unlock(&pb->lock);

    if (pb->thread_running) {
        pthread_join(pb->render_thread, NULL);
        pb->thread_running = false;
    }

    if (!is_terminal()) return;

    printf("%s", CLEAR_LINE);
    if (pb->step_number > 0)
        printf("[%d/%d] ", pb->step_number, pb->total_steps);

    printf("%s✗%s %s failed", COLOR_RED, COLOR_RESET, pb->message);

    if (error && error[0])
        printf(": %s", error);

    printf("\n%s", SHOW_CURSOR);
    fflush(stdout);
}

void progress_destroy(ProgressBar *pb) {
    // If caller forgot to complete/fail, clean up gracefully
    if (!pb->completed)
        progress_fail(pb, "destroyed before completion");

    free(pb->message);
    pb->message = NULL;
    pthread_mutex_destroy(&pb->lock);
}

// ── Spinner ───────────────────────────────────────────────────────────────────

// Braille dot spinner — smooth and doesn't require wide chars
static const char *SPINNER_FRAMES[] = {
    "⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"
};
#define SPINNER_FRAME_COUNT 10

static void *spinner_thread_fn(void *arg) {
    Spinner *sp = (Spinner *)arg;
    int idx = 0;
    struct timespec frame = { 0, 80000000L }; // 80ms per frame

    while (1) {
        pthread_mutex_lock(&sp->lock);
        bool done   = sp->done;
        bool failed = sp->failed;
        pthread_mutex_unlock(&sp->lock);

        if (done || failed) break;

        printf("%s%s %s...", CLEAR_LINE,
               SPINNER_FRAMES[idx % SPINNER_FRAME_COUNT],
               sp->message);
        fflush(stdout);
        idx++;

        nanosleep(&frame, NULL);
    }
    return NULL;
}

void spinner_start(Spinner *sp, const char *message) {
    memset(sp, 0, sizeof(*sp));
    snprintf(sp->message, sizeof(sp->message), "%s", message);
    pthread_mutex_init(&sp->lock, NULL);

    if (!is_terminal()) return;

    printf("%s", HIDE_CURSOR);
    fflush(stdout);
    pthread_create(&sp->thread, NULL, spinner_thread_fn, sp);
}

void spinner_complete(Spinner *sp, const char *extra_info) {
    pthread_mutex_lock(&sp->lock);
    sp->done = true;
    pthread_mutex_unlock(&sp->lock);

    pthread_join(sp->thread, NULL);

    printf("%s%s✓%s %s", CLEAR_LINE, COLOR_GREEN, COLOR_RESET, sp->message);
    if (extra_info && extra_info[0])
        printf(" (%s)", extra_info);
    printf("\n%s", SHOW_CURSOR);
    fflush(stdout);

    pthread_mutex_destroy(&sp->lock);
}

void spinner_fail(Spinner *sp, const char *error) {
    pthread_mutex_lock(&sp->lock);
    sp->failed = true;
    pthread_mutex_unlock(&sp->lock);

    pthread_join(sp->thread, NULL);

    printf("%s%s✗%s %s", CLEAR_LINE, COLOR_RED, COLOR_RESET, sp->message);
    if (error && error[0])
        printf(": %s", error);
    printf("\n%s", SHOW_CURSOR);
    fflush(stdout);

    pthread_mutex_destroy(&sp->lock);
}