#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "help.h"
#include "init.h"
#include "deploy.h"
#include "version.h"
#include "log.h"
#include "config.h"
#include "exit_codes.h"
#include "state.h"
#include "cleanup.h"
#include "bundle.h"
#include "diff.h"

void load_config(void) {
    const char *home = getenv("HOME");
    if (!home) {
        log_error("Cannot find home directory");
        return;
    }
    
    char path[256];
    snprintf(path, sizeof(path), "%s/.config/forge/config", home);
    
    FILE *f = fopen(path, "r");
    if (!f) return;
    
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

// Helper to check if a string ends with a suffix
static int str_ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (str_len < suffix_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int main(int argc, char *argv[]) {
    // Load environment and config
    load_env_defaults();
    load_config();
    setup_signal_handlers();
    
    int verbose_mode = 0;
    int quiet_mode = 0;
    int offline_mode = 0;
    int show_progress = isatty(STDOUT_FILENO);
    char *command = NULL;
    int command_index = -1;
    
    // Cleanup options
    CleanupOptions cleanup_opts = {
        .dry_run = 0,
        .keep_versions = 0,
        .older_than_days = 0,
        .prune_images = 0,
        .remove_all = 0
    };
    
    // STEP 1: Parse flags ONLY (find the command)
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
        else if(strcmp(argv[i], "--offline") == 0) {
            offline_mode = 1;
        }
        else {
            if (command == NULL) {
                command = argv[i];
                command_index = i;
            }
        }
    }
    
    // Parse cleanup-specific flags if command is cleanup
    if (command != NULL && (strcmp(command, "cleanup") == 0 || strcmp(command, "--cleanup") == 0)) {
        for (int i = command_index + 1; i < argc; i++) {
            if (strcmp(argv[i], "--dry-run") == 0) {
                cleanup_opts.dry_run = 1;
            }
            else if (strcmp(argv[i], "--keep") == 0 && i + 1 < argc) {
                i++;
                cleanup_opts.keep_versions = atoi(argv[i]);
            }
            else if (strcmp(argv[i], "--older-than") == 0 && i + 1 < argc) {
                i++;
                char *endptr;
                cleanup_opts.older_than_days = strtol(argv[i], &endptr, 10);
                if (*endptr == 'd') {
                    cleanup_opts.older_than_days = strtol(argv[i], &endptr, 10);
                }
            }
            else if (strcmp(argv[i], "--prune-images") == 0) {
                cleanup_opts.prune_images = 1;
            }
            else if (strcmp(argv[i], "--all") == 0) {
                cleanup_opts.remove_all = 1;
            }
        }
    }
    
    // Quiet mode disables progress
    if (quiet_mode) {
        show_progress = 0;
    }
    
    // STEP 2: Apply flags
    if (verbose_mode) {
        g_config.log_level = LOG_LEVEL_DEBUG;
    } else if (quiet_mode) {
        g_config.log_level = LOG_LEVEL_ERROR;
    }
    
    // STEP 3: Log startup
    log_info("Forge started");
    if (verbose_mode) log_debug("Verbose mode enabled");
    if (quiet_mode) log_debug("Quiet mode enabled");
    if (offline_mode) log_info("Offline mode enabled - using cached assets only");
    
    // STEP 4: No arguments?
    if (argc == 1 || command == NULL) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    
    // STEP 5: Handle commands
    if (strcmp(command, "--list") == 0) {
        return list_deployments();
    }
    else if (strcmp(command, "--status") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge --status <project>");
            return EXIT_BAD_ARGS;
        }
        return show_status(argv[command_index + 1]);
    }
    else if (strcmp(command, "--logs") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge --logs <project>");
            return EXIT_BAD_ARGS;
        }
        return show_logs(argv[command_index + 1]);
    }
    else if (strcmp(command, "--rollback") == 0 || strcmp(command, "rollback") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge --rollback <project>");
            return EXIT_BAD_ARGS;
        }
        return rollback_deployment(argv[command_index + 1]);
    }
    else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    else if (strcmp(command, "cleanup") == 0 || strcmp(command, "--cleanup") == 0) {
        return run_cleanup(&cleanup_opts);
    }
    else if (strcmp(command, "bundle") == 0 || strcmp(command, "--bundle") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge bundle <project> [--version vN] [-o output] [--ship user@host] [--no-binary] [--dry-run]");
            return EXIT_BAD_ARGS;
        }
        return cmd_bundle(argv[command_index + 1], argc - command_index - 2, argv + command_index + 2);
    }
    else if (strcmp(command, "diff") == 0 || strcmp(command, "--diff") == 0) {
    if (argc < command_index + 2) {
        log_error("Usage: forge diff <project> [--patch] [--meta-only] [--json]");
        log_error("       forge diff <project> -v <v1> <v2>");
        return EXIT_BAD_ARGS;
    }
    // argv[command_index + 1] is the project name
    // Remaining args start at command_index + 2
    return cmd_diff(argv[command_index + 1], 
                    argc - command_index - 2, 
                    argv + command_index + 2);
    }
    else if (strcmp(command, "deploy") == 0 || strcmp(command, "--deploy") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge deploy <repo_url|bundle.tar.gz> [-i|-d] [--offline]");
            return EXIT_BAD_ARGS;
        }
        
        run_mode_t mode = RUN_DEFAULT;
        char *target = argv[command_index + 1];
        
        for (int i = command_index + 2; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) {
                mode = RUN_INTERACTIVE;
            } else if (strcmp(argv[i], "-d") == 0) {
                mode = RUN_DETACHED;
            }
        }
        
        // Check if target is a bundle file (.tar.gz)
        if (str_ends_with(target, ".tar.gz")) {
            return deploy_from_bundle(target, mode == RUN_DETACHED);
        }
        
        // Normal deployment from URL
        return deploy_repo(target, mode, offline_mode, show_progress);
    }
    else if (strcmp(command, "init") == 0 || strcmp(command, "--init") == 0) {
        if (argc < command_index + 2) {
            log_error("Usage: forge init <project_name> [--type python|node|go|rust|c] [--ci github]");
            return EXIT_BAD_ARGS;
        }
        
        InitOptions opts = {0};
        opts.project_name = argv[command_index + 1];
        opts.language = LANG_PYTHON;
        opts.ci = CI_NONE;
        opts.interactive = 0;
        
        for (int i = command_index + 2; i < argc; i++) {
            if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
                i++;
                if (strcmp(argv[i], "python") == 0) opts.language = LANG_PYTHON;
                else if (strcmp(argv[i], "node") == 0) opts.language = LANG_NODE;
                else if (strcmp(argv[i], "go") == 0) opts.language = LANG_GO;
                else if (strcmp(argv[i], "rust") == 0) opts.language = LANG_RUST;
                else if (strcmp(argv[i], "c") == 0) opts.language = LANG_C;
                else {
                    log_error("Unknown language: %s (python, node, go, rust, c)", argv[i]);
                    return EXIT_BAD_ARGS;
                }
            }
            else if (strcmp(argv[i], "--ci") == 0 && i + 1 < argc) {
                i++;
                if (strcmp(argv[i], "github") == 0) opts.ci = CI_GITHUB;
                else if (strcmp(argv[i], "gitlab") == 0) opts.ci = CI_GITLAB;
                else {
                    log_error("Unknown CI: %s (github, gitlab)", argv[i]);
                    return EXIT_BAD_ARGS;
                }
            }
        }
        
        return init_project(&opts);
    }
    else if (strcmp(command, "ssh") == 0 || strcmp(command, "--ssh") == 0) {
        return handle_remote_command(argc - command_index, argv + command_index, show_progress);
    }
    
    // Unknown command
    log_error("Unknown command '%s'", command);
    printf("Run '%s help' for more information.\n", argv[0]);
    return EXIT_BAD_ARGS;
}