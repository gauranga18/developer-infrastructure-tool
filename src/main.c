#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // for getenv()
#include <sys/stat.h>
#include "help.h"
#include "init.h"
#include "deploy.h"
#include "version.h"
#include "log.h"
#include "config.h"
#include "exit_codes.h"
void load_config(void) {
    const char *home = getenv("HOME");
    if (!home) {
        log_error("Cannot find home directory");
        return;
    }
    
    char path[256];
    snprintf(path, sizeof(path), "%s/.config/forge/config", home);
    
    FILE *f = fopen(path, "r");
    if (!f) return;  // No config file, use defaults
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "log_level = debug")) {
            g_config.log_level = LOG_LEVEL_DEBUG;
        } else if (strstr(line, "log_level = error")) {
            g_config.log_level = LOG_LEVEL_ERROR;
        } else if (strstr(line, "log_level = warn")) {
            g_config.log_level = LOG_LEVEL_WARN;
        } else if (strstr(line, "log_level = info")) {
            g_config.log_level = LOG_LEVEL_INFO;
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    // Call environment variable from config.c
    load_env_defaults();
    // Load config FIRST
    load_config();
    
    int verbose_mode = 0;
    int quiet_mode = 0;
    char *command = NULL;
    int command_index = -1;
    
    // Parse flags
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-ver") == 0) {
            verbose_mode = 1;
        }
        else if(strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet_mode = 1;
        }
        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[i], "-v") == 0) {
            printf("Forge version %s\n", FORGE_VERSION);
            return EXIT_SUCCESS;
        }
        else {
            if (command == NULL) {
                command = argv[i];
                command_index = i;
            }
        }
    }
    
    // Apply flags (override config)
    if (verbose_mode) {
        g_config.log_level = LOG_LEVEL_DEBUG;
    } else if (quiet_mode) {
        g_config.log_level = LOG_LEVEL_ERROR;
    }
    
    // Log startup
    log_info("Forge started");
    if (verbose_mode) log_debug("Verbose mode enabled");
    if (quiet_mode) log_debug("Quiet mode enabled");
    
    // No arguments?
    if (argc == 1 || command == NULL) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    
    // Handle commands
    if (strcmp(command, "help") == 0) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    
    if (strcmp(command, "deploy") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: %s deploy <repo_url> [-i|-d]", argv[0]);
            return EXIT_BAD_ARGS;
        }
        
        run_mode_t mode = RUN_DEFAULT;
        char *repo_url = argv[command_index + 1];
        
        for (int i = command_index + 2; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) {
                mode = RUN_INTERACTIVE;
            } else if (strcmp(argv[i], "-d") == 0) {
                mode = RUN_DETACHED;
            }
        }
        
        return deploy_repo(repo_url, mode);
    }
    
    if (strcmp(command, "init") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: %s init <project_name>", argv[0]);
            return EXIT_BAD_ARGS;
        }
        return init_project(argv[command_index + 1]);
    }
    
    // Unknown command
    log_error("Unknown command '%s'", command);
    printf("Run '%s help' for more information.\n", argv[0]);
    return EXIT_BAD_ARGS;
}
