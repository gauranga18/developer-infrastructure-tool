#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

// Environment variable names
#define ENV_REGISTRY    "FORGE_REGISTRY"
#define ENV_LOG_LEVEL   "FORGE_LOG_LEVEL"
#define ENV_CONFIG_DIR  "FORGE_CONFIG_DIR"
#define ENV_STATE_DIR   "XDG_STATE_HOME"
#define ENV_CONFIG_HOME "XDG_CONFIG_HOME"

// Log levels
enum LogLevel {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3
};

// Configuration structure
struct Config {
    enum LogLevel log_level;
    char *registry;
    char *config_dir;
};

extern struct Config g_config;

// Path resolution functions
const char* get_forge_state_dir(void);
const char* get_forge_config_dir(void);
const char* get_forge_log_path(void);
// Cache functions
const char* get_forge_cache_dir(void);
const char* get_repo_cache_path(const char *project);
const char* get_image_cache_path(const char *image_name);
int ensure_forge_dirs(void);

// Environment functions
void load_env_defaults(void);

#endif