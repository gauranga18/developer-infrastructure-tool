#include "config.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

struct Config g_config = {
    .log_level = LOG_LEVEL_INFO,  // ← Comma
    .registry = NULL,             // ← Comma
    .config_dir = NULL            // ← No semicolon
};

void load_env_defaults() {
    char *val;
    
    // FORGE_REGISTRY environment variable
    val = getenv(ENV_REGISTRY);  // Use string directly for now
    if (val) {
        if (g_config.registry) free(g_config.registry);  // Prevent leak
        g_config.registry = strdup(val);
        log_debug("Loaded registry from env: %s", val);
    }
    
    // FORGE_LOG_LEVEL environment variable
    val = getenv(ENV_LOG_LEVEL);
    if (val) {
        if (strcmp(val, "debug") == 0) {
            g_config.log_level = LOG_LEVEL_DEBUG;
            log_debug("Loaded log level from env: debug");
        } else if (strcmp(val, "error") == 0) {
            g_config.log_level = LOG_LEVEL_ERROR;
            log_debug("Loaded log level from env: error");
        } else if (strcmp(val, "warn") == 0) {
            g_config.log_level = LOG_LEVEL_WARN;
            log_debug("Loaded log level from env: warn");
        } else if (strcmp(val, "info") == 0) {
            g_config.log_level = LOG_LEVEL_INFO;
            log_debug("Loaded log level from env: info");
        }
    }
}