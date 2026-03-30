#include "cleanup.h"
#include "state.h"
#include "log.h"
#include "exit_codes.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static long long total_freed = 0;

static int get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

static void delete_file(const char *path, int dry_run) {
    int size = get_file_size(path);
    if (dry_run) {
        printf("  Would delete: %s (%d bytes)\n", path, size);
        total_freed += size;
    } else {
        if (unlink(path) == 0) {
            printf("  Deleted: %s (%d bytes)\n", path, size);
            total_freed += size;
        } else {
            log_error("Failed to delete: %s", path);
        }
    }
}

int run_cleanup(CleanupOptions *opts) {
    printf("\n=== Forge Cleanup ===\n");
    if (opts->dry_run) {
        printf("(Dry Run - No changes will be made)\n\n");
    }
    
    // Initialize state system first
    if (state_init() != 0) {
        log_error("Failed to initialize state system");
        return EXIT_GENERIC;
    }
    
    // Get state directory from config
    const char *state_dir = get_forge_state_dir();
    if (!state_dir) {
        log_error("Failed to get state directory");
        return EXIT_GENERIC;
    }
    
    char deployments_dir[512];
    char current_dir[512];
    
    snprintf(deployments_dir, sizeof(deployments_dir), "%s/deployments", state_dir);
    snprintf(current_dir, sizeof(current_dir), "%s/current", state_dir);
    
    // 1. Clean up old deployment files
    printf("\n[1/4] Cleaning old deployment files...\n");
    
    DIR *dir = opendir(deployments_dir);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", deployments_dir, entry->d_name);
            
            // Check if file is a JSON file
            char *dot = strrchr(entry->d_name, '.');
            if (!dot || strcmp(dot, ".json") != 0) continue;
            
            // Delete if older than specified days
            if (opts->older_than_days > 0) {
                struct stat st;
                if (stat(fullpath, &st) == 0) {
                    time_t now = time(NULL);
                    int days_old = (now - st.st_mtime) / (60 * 60 * 24);
                    if (days_old > opts->older_than_days) {
                        delete_file(fullpath, opts->dry_run);
                    }
                }
            } else if (opts->keep_versions > 0) {
                // TODO: Implement per-project version tracking
                // For now, skip
            }
        }
        closedir(dir);
    } else {
        log_info("No deployments directory found");
    }
    
    // 2. Clean up stopped containers
    printf("\n[2/4] Cleaning stopped containers...\n");
    
    char cmd[512];
    if (opts->dry_run) {
        snprintf(cmd, sizeof(cmd), "docker ps -a -f status=exited -q 2>/dev/null | wc -l");
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char count[32];
            if (fgets(count, sizeof(count), fp)) {
                int num = atoi(count);
                if (num > 0) {
                    printf("  Would remove %d stopped container(s)\n", num);
                } else {
                    printf("  No stopped containers found\n");
                }
            }
            pclose(fp);
        }
    } else {
        snprintf(cmd, sizeof(cmd), "docker container prune -f 2>/dev/null");
        int result = system(cmd);
        if (result == 0) {
            printf("  Removed stopped containers\n");
        }
    }
    
    // 3. Clean up unused images (if requested)
    if (opts->prune_images) {
        printf("\n[3/4] Pruning unused Docker images...\n");
        if (opts->dry_run) {
            snprintf(cmd, sizeof(cmd), "docker images -q 2>/dev/null | wc -l");
            FILE *fp = popen(cmd, "r");
            if (fp) {
                char count[32];
                if (fgets(count, sizeof(count), fp)) {
                    int num = atoi(count);
                    if (num > 0) {
                        printf("  Would prune %d image(s)\n", num);
                    } else {
                        printf("  No images found\n");
                    }
                }
                pclose(fp);
            }
        } else {
            snprintf(cmd, sizeof(cmd), "docker image prune -f 2>/dev/null");
            system(cmd);
            printf("  Pruned unused images\n");
        }
    }
    
    // 4. Clean up empty directories
    printf("\n[4/4] Cleaning empty directories...\n");
    
    DIR *state_dir_ptr = opendir(state_dir);
    if (state_dir_ptr) {
        struct dirent *entry;
        while ((entry = readdir(state_dir_ptr)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            char subpath[512];
            snprintf(subpath, sizeof(subpath), "%s/%s", state_dir, entry->d_name);
            
            struct stat st;
            if (stat(subpath, &st) == 0 && S_ISDIR(st.st_mode)) {
                DIR *subdir = opendir(subpath);
                if (subdir) {
                    int count = 0;
                    struct dirent *subentry;
                    while ((subentry = readdir(subdir)) != NULL) {
                        if (subentry->d_name[0] != '.') count++;
                    }
                    closedir(subdir);
                    
                    if (count == 0) {
                        if (opts->dry_run) {
                            printf("  Would remove empty directory: %s\n", subpath);
                        } else {
                            rmdir(subpath);
                            printf("  Removed empty directory: %s\n", subpath);
                        }
                    }
                }
            }
        }
        closedir(state_dir_ptr);
    }
    
    printf("\n=== Cleanup Complete ===\n");
    if (total_freed > 0) {
        printf("Total space freed: %.2f MB\n", total_freed / (1024.0 * 1024.0));
    } else {
        printf("No space freed\n");
    }
    
    return EXIT_SUCCESS;
}