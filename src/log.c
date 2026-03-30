#include "log.h"
#include "config.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

static int get_level_value(const char *level) {
    if (strcmp(level, "ERROR") == 0) return LOG_LEVEL_ERROR;
    if (strcmp(level, "WARN") == 0) return LOG_LEVEL_WARN;
    if (strcmp(level, "INFO") == 0) return LOG_LEVEL_INFO;
    if (strcmp(level, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
    return LOG_LEVEL_INFO;
}

static void write_log(const char *level, const char *message) {
    int msg_level = get_level_value(level);
    if (msg_level > g_config.log_level) {
        return;
    }
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Always print to terminal
    if (strcmp(level, "ERROR") == 0) {
        fprintf(stderr, "[%s] %s: %s\n", timestamp, level, message);
    } else {
        printf("[%s] %s: %s\n", timestamp, level, message);
    }
    
    // Check if we're in a container - skip file logging
    if (getenv("container") != NULL || access("/.dockerenv", F_OK) == 0) {
        return;
    }
    
    // Write to file using XDG path
    const char *log_path = get_forge_log_path();
    FILE *log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        return;  // Silent fail
    }
    
    fprintf(log_file, "[%s] %s: %s\n", timestamp, level, message);
    fclose(log_file);
}

void log_info(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    write_log("INFO", message);
}

void log_warn(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    write_log("WARN", message);
}

void log_error(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    write_log("ERROR", message);
}

void log_debug(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    write_log("DEBUG", message);
}