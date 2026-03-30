#include "config.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>

struct Config g_config = {
    .log_level = LOG_LEVEL_INFO,
    .registry = NULL,
    .config_dir = NULL
};

// Get home directory with fallback
static const char* get_home_dir(void) {
    const char *home = getenv("HOME");
    if (home) return home;
    
    struct passwd *pw = getpwuid(getuid());
    if (pw) return pw->pw_dir;
    
    return "/tmp";
}

// Get XDG state directory
const char* get_xdg_state_dir(void) {
    const char *xdg_state = getenv(ENV_STATE_DIR);
    static char state_path[512];
    
    if (xdg_state && xdg_state[0] == '/') {
        return xdg_state;
    }
    
    snprintf(state_path, sizeof(state_path), "%s/.local/state", get_home_dir());
    return state_path;
}

// Get XDG config directory
const char* get_xdg_config_dir(void) {
    const char *xdg_config = getenv(ENV_CONFIG_HOME);
    static char config_path[512];
    
    if (xdg_config && xdg_config[0] == '/') {
        return xdg_config;
    }
    
    snprintf(config_path, sizeof(config_path), "%s/.config", get_home_dir());
    return config_path;
}

// Get Forge state directory
const char* get_forge_state_dir(void) {
    static char forge_state[512];
    const char *xdg_state = get_xdg_state_dir();
    snprintf(forge_state, sizeof(forge_state), "%s/forge", xdg_state);
    return forge_state;
}

// Get Forge cache directory
const char* get_forge_cache_dir(void) {
    static char cache_path[512];
    const char *state_dir = get_forge_state_dir();
    snprintf(cache_path, sizeof(cache_path), "%s/cache", state_dir);
    return cache_path;
}

// Get repo cache path for a project
const char* get_repo_cache_path(const char *project) {
    static char repo_path[512];
    const char *cache_dir = get_forge_cache_dir();
    snprintf(repo_path, sizeof(repo_path), "%s/repos/%s", cache_dir, project);
    return repo_path;
}

// Get image cache path
const char* get_image_cache_path(const char *image_name) {
    static char image_path[512];
    const char *cache_dir = get_forge_cache_dir();
    snprintf(image_path, sizeof(image_path), "%s/images/%s.tar", cache_dir, image_name);
    return image_path;
}

// Get Forge config directory
const char* get_forge_config_dir(void) {
    static char forge_config[512];
    const char *xdg_config = get_xdg_config_dir();
    snprintf(forge_config, sizeof(forge_config), "%s/forge", xdg_config);
    return forge_config;
}

// Get Forge log file path
const char* get_forge_log_path(void) {
    static char log_path[512];
    const char *state_dir = get_forge_state_dir();
    snprintf(log_path, sizeof(log_path), "%s/forge.log", state_dir);
    return log_path;
}

// Ensure directory exists (non-static for use in other files)
int ensure_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            log_error("Failed to create directory %s: %s", path, strerror(errno));
            return -1;
        }
        log_info("Created directory: %s", path);
    }
    return 0;
}

// Ensure all required directories exist
int ensure_forge_dirs(void) {
    const char *state_dir = get_forge_state_dir();
    const char *config_dir = get_forge_config_dir();
    
    // Create main state directory
    ensure_dir(state_dir);
    
    // Create deployments subdirectory
    char deployments_path[512];
    snprintf(deployments_path, sizeof(deployments_path), "%s/deployments", state_dir);
    ensure_dir(deployments_path);
    
    // Create current subdirectory
    char current_path[512];
    snprintf(current_path, sizeof(current_path), "%s/current", state_dir);
    ensure_dir(current_path);
    
    // Create projects subdirectory
    char projects_path[512];
    snprintf(projects_path, sizeof(projects_path), "%s/projects", state_dir);
    ensure_dir(projects_path);
    
    // Create cache directories
    char cache_path[512];
    snprintf(cache_path, sizeof(cache_path), "%s/cache", state_dir);
    ensure_dir(cache_path);
    
    char repos_path[512];
    snprintf(repos_path, sizeof(repos_path), "%s/cache/repos", state_dir);
    ensure_dir(repos_path);
    
    char images_path[512];
    snprintf(images_path, sizeof(images_path), "%s/cache/images", state_dir);
    ensure_dir(images_path);
    
    // Create config directory
    ensure_dir(config_dir);
    
    log_info("Forge directories initialized at %s", state_dir);
    return 0;
}

// Load environment variables
void load_env_defaults(void) {
    char *val;
    
    val = getenv(ENV_REGISTRY);
    if (val) {
        if (g_config.registry) free(g_config.registry);
        g_config.registry = strdup(val);
        log_debug("Loaded registry from env: %s", val);
    }
    
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