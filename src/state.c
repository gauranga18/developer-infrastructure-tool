#include "state.h"
#include "log.h"
#include "exit_codes.h"
#include "utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>

static char base_path[512];

// Helper: Build full path using XDG state directory
static void build_paths(void) {
    const char *state_dir = get_forge_state_dir();
    snprintf(base_path, sizeof(base_path), "%s", state_dir);
}

// Helper: Ensure directory exists
static int ensure_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            log_error("Failed to create directory %s: %s", 
                      path, strerror(errno));
            return -1;
        }
        log_info("Created directory: %s", path);
    }
    return 0;
}

// Get path to project registry file
static void get_project_registry_path(char *path, size_t size, const char *project) {
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/projects", base_path);
    ensure_dir(dir_path);
    snprintf(path, size, "%s/projects/%s.json", base_path, project);
}

// Get path to lock file for a project
void get_lock_path(char *path, size_t size, const char *project) {
    snprintf(path, size, "%s/.%s.lock", base_path, project);
}

// Lock a project
int lock_project(const char *project) {
    char lock_path[512];
    get_lock_path(lock_path, sizeof(lock_path), project);
    
    int lock_fd = lock_file(lock_path);
    if (lock_fd == -1) {
        log_error("Failed to acquire lock for project %s", project);
        return -1;
    }
    
    return lock_fd;
}

// Unlock a project
void unlock_project(int fd) {
    unlock_file(fd);
}

// Get next version number for a project
int get_next_version(const char *project) {
    char registry_path[512];
    get_project_registry_path(registry_path, sizeof(registry_path), project);
    
    FILE *f = fopen(registry_path, "r");
    if (!f) {
        return 1;
    }
    
    char line[256];
    int next_version = 1;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "  \"next_version\": %d", &next_version) == 1) {
            break;
        }
    }
    fclose(f);
    return next_version;
}

// Update the stored next version (locked version)
int update_next_version_locked(const char *project, int new_version) {
    char registry_path[512];
    get_project_registry_path(registry_path, sizeof(registry_path), project);
    
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", registry_path);
    
    FILE *f = fopen(temp_path, "w");
    if (!f) {
        log_error("Failed to create project registry: %s", strerror(errno));
        return -1;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  \"project\": \"%s\",\n", project);
    fprintf(f, "  \"next_version\": %d\n", new_version + 1);
    fprintf(f, "}\n");
    
    fclose(f);
    rename(temp_path, registry_path);
    return 0;
}

// Public version with locking
int update_next_version(const char *project, int new_version) {
    char lock_path[512];
    get_lock_path(lock_path, sizeof(lock_path), project);
    
    int lock_fd = lock_file(lock_path);
    if (lock_fd == -1) {
        log_error("Failed to acquire lock for %s", project);
        return -1;
    }
    
    int result = update_next_version_locked(project, new_version);
    unlock_file(lock_fd);
    return result;
}

// Build versioned name
void build_versioned_name(char *dest, size_t size, const char *project, int version) {
    snprintf(dest, size, "%s-v%d", project, version);
}

// Initialize state system
int state_init(void) {
    build_paths();
    
    char path[512];
    
    // Create main state directory
    snprintf(path, sizeof(path), "%s", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create deployments directory
    snprintf(path, sizeof(path), "%s/deployments", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create current directory
    snprintf(path, sizeof(path), "%s/current", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create projects directory
    snprintf(path, sizeof(path), "%s/projects", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create cache directory
    snprintf(path, sizeof(path), "%s/cache", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create cache/repos directory
    snprintf(path, sizeof(path), "%s/cache/repos", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    // Create cache/images directory
    snprintf(path, sizeof(path), "%s/cache/images", base_path);
    if (ensure_dir(path) != 0) return -1;
    
    log_info("State system initialized at %s", base_path);
    return 0;
}
// Generate deployment ID
void generate_deployment_id(char *id, size_t size, const char *project) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm);
    
    snprintf(id, size, "%s-%s", project, timestamp);
}

// Save deployment to disk
int state_save(const Deployment *dep) {
    if (state_init() != 0) return -1;
    
    char lock_path[512];
    get_lock_path(lock_path, sizeof(lock_path), dep->project);
    
    int lock_fd = lock_file(lock_path);
    if (lock_fd == -1) {
        log_error("Failed to acquire lock for %s", dep->project);
        return -1;
    }
    
    char path[512];
    snprintf(path, sizeof(path), "%s/deployments/%s.json", base_path, dep->id);
    
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);
    
    FILE *f = fopen(temp_path, "w");
    if (!f) {
        log_error("Failed to create deployment file: %s", strerror(errno));
        unlock_file(lock_fd);
        return -1;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  \"id\": \"%s\",\n", dep->id);
    fprintf(f, "  \"project\": \"%s\",\n", dep->project);
    fprintf(f, "  \"url\": \"%s\",\n", dep->url);
    fprintf(f, "  \"image_id\": \"%s\",\n", dep->image_id);
    fprintf(f, "  \"container_id\": \"%s\",\n", dep->container_id);
    fprintf(f, "  \"port\": %d,\n", dep->port);
    fprintf(f, "  \"deployed_at\": %ld,\n", dep->deployed_at);
    fprintf(f, "  \"version\": %d,\n", dep->version);
    fprintf(f, "  \"status\": %d\n", dep->status);
    fprintf(f, "}\n");
    
    fclose(f);
    
    if (rename(temp_path, path) != 0) {
        log_error("Failed to rename deployment file: %s", strerror(errno));
        unlink(temp_path);
        unlock_file(lock_fd);
        return -1;
    }
    
    char link_path[512];
    snprintf(link_path, sizeof(link_path), "%s/current/%s", base_path, dep->project);
    unlink(link_path);
    
    if (symlink(path, link_path) == -1) {
        log_warn("Failed to create current symlink: %s", strerror(errno));
    }
    
    log_info("Saved deployment: %s", dep->id);
    unlock_file(lock_fd);
    return 0;
}

// Parse deployment from JSON file
static Deployment *parse_deployment_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    
    Deployment *dep = malloc(sizeof(Deployment));
    if (!dep) {
        fclose(f);
        return NULL;
    }
    memset(dep, 0, sizeof(Deployment));
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], value[256];
        
        if (sscanf(line, "  \"%[^\"]\": \"%[^\"]\",", key, value) == 2) {
            if (strcmp(key, "id") == 0) strncpy(dep->id, value, sizeof(dep->id)-1);
            else if (strcmp(key, "project") == 0) strncpy(dep->project, value, sizeof(dep->project)-1);
            else if (strcmp(key, "url") == 0) strncpy(dep->url, value, sizeof(dep->url)-1);
            else if (strcmp(key, "image_id") == 0) strncpy(dep->image_id, value, sizeof(dep->image_id)-1);
            else if (strcmp(key, "container_id") == 0) strncpy(dep->container_id, value, sizeof(dep->container_id)-1);
        }
        else if (sscanf(line, "  \"port\": %d,", &dep->port) == 1);
        else if (sscanf(line, "  \"deployed_at\": %ld,", &dep->deployed_at) == 1);
        else if (sscanf(line, "  \"version\": %d,", &dep->version) == 1);
        else if (sscanf(line, "  \"status\": %d", &dep->status) == 1);
    }
    
    fclose(f);
    return dep;
}

// Find latest deployment for a project
Deployment *state_find_latest(const char *project) {
    if (strlen(base_path) == 0) {
        build_paths();
    }
    
    char link_path[512];
    snprintf(link_path, sizeof(link_path), "%s/current/%s", base_path, project);
    
    char target[512];
    ssize_t len = readlink(link_path, target, sizeof(target)-1);
    if (len == -1) return NULL;
    
    target[len] = '\0';
    return parse_deployment_file(target);
}

// Compare function for sorting
static int compare_versions(const void *a, const void *b) {
    const Deployment *da = *(const Deployment**)a;
    const Deployment *db = *(const Deployment**)b;
    return db->version - da->version;
}

// Get all deployments for a project
Deployment **state_get_history(const char *project, int *count) {
    char deploy_path[512];
    snprintf(deploy_path, sizeof(deploy_path), "%s/deployments", base_path);
    
    DIR *dir = opendir(deploy_path);
    if (!dir) {
        *count = 0;
        return NULL;
    }
    
    int max = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, project) == entry->d_name) {
            max++;
        }
    }
    
    if (max == 0) {
        closedir(dir);
        *count = 0;
        return NULL;
    }
    
    Deployment **result = malloc(max * sizeof(Deployment*));
    if (!result) {
        closedir(dir);
        *count = 0;
        return NULL;
    }
    
    rewinddir(dir);
    int idx = 0;
    while ((entry = readdir(dir)) != NULL && idx < max) {
        if (strstr(entry->d_name, project) == entry->d_name) {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", deploy_path, entry->d_name);
            
            Deployment *dep = parse_deployment_file(fullpath);
            if (dep) {
                result[idx++] = dep;
            }
        }
    }
    
    closedir(dir);
    qsort(result, idx, sizeof(Deployment*), compare_versions);
    *count = idx;
    return result;
}

// Free history array
void state_free_history(Deployment **history, int count) {
    for (int i = 0; i < count; i++) {
        free(history[i]);
    }
    free(history);
}

// List all deployments
int list_deployments(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s/current", base_path);
    
    DIR *dir = opendir(path);
    if (!dir) {
        printf("No deployments found\n");
        return 0;
    }
    
    struct dirent *entry;
    printf("\n%-25s %-12s %-20s %s\n", 
           "PROJECT", "STATUS", "DEPLOYED", "VERSION");
    printf("--------------------------------------------------------------------------------\n");
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char link_path[512];
        snprintf(link_path, sizeof(link_path), "%s/%s", path, entry->d_name);
        
        char target[512];
        ssize_t len = readlink(link_path, target, sizeof(target)-1);
        if (len != -1) {
            target[len] = '\0';
            Deployment *dep = parse_deployment_file(target);
            if (dep) {
                char time_str[20];
                struct tm *tm = localtime(&dep->deployed_at);
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm);
                
                printf("%-25s %-12s %-20s v%d\n", 
                       dep->project, 
                       dep->status == 1 ? "running" : "exited",
                       time_str, 
                       dep->version);
                free(dep);
            }
        }
    }
    
    closedir(dir);
    printf("\n");
    return 0;
}

// Show status of a specific project
int show_status(const char *project) {
    Deployment *dep = state_find_latest(project);
    if (!dep) {
        log_error("No deployment found for project: %s", project);
        return EXIT_NOT_FOUND;
    }
    
    printf("\n=== Project: %s ===\n", dep->project);
    printf("Version: %d\n", dep->version);
    printf("Deployed: %s", ctime(&dep->deployed_at));
    printf("Image: %s\n", dep->image_id);
    printf("Container ID: %s\n", dep->container_id);
    
    if (strlen(dep->container_id) > 0 && strcmp(dep->container_id, "interactive") != 0) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), 
                 "docker inspect -f 'Status: {{.State.Status}}\nIP: {{.NetworkSettings.IPAddress}}\nPort: {{(index .NetworkSettings.Ports \"8080/tcp\" 0).HostPort}}' %s 2>/dev/null",
                 dep->container_id);
        
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                printf("%s", line);
            }
            pclose(fp);
        }
    } else {
        printf("\nContainer: interactive mode (no persistent container)\n");
    }
    
    printf("\n");
    free(dep);
    return 0;
}

// Show logs for a project
int show_logs(const char *project) {
    Deployment *dep = state_find_latest(project);
    if (!dep) {
        log_error("No deployment found for project: %s", project);
        return EXIT_NOT_FOUND;
    }
    
    if (strlen(dep->container_id) == 0 || strcmp(dep->container_id, "interactive") == 0) {
        log_error("No logs available for interactive session");
        free(dep);
        return EXIT_GENERIC;
    }
    
    const char *args[] = {"docker", "logs", dep->container_id, NULL};
    int result = run_command_fork(args);
    
    free(dep);
    return result;
}

// Rollback to previous version
int rollback_deployment(const char *project) {
    Deployment *current = state_find_latest(project);
    if (!current) {
        log_error("No deployment found for project: %s", project);
        return EXIT_NOT_FOUND;
    }
    
    int count;
    Deployment **history = state_get_history(project, &count);
    if (!history || count < 2) {
        log_error("No previous version to rollback to");
        if (history) state_free_history(history, count);
        free(current);
        return EXIT_GENERIC;
    }
    
    Deployment *previous = history[1];
    
    log_info("Rolling back %s from v%d to v%d", 
             project, current->version, previous->version);
    
    if (strlen(current->container_id) > 0 && 
        strcmp(current->container_id, "interactive") != 0) {
        
        log_info("Stopping current container: %s", current->container_id);
        const char *stop_args[] = {"docker", "stop", current->container_id, NULL};
        run_command_fork(stop_args);
        
        const char *rm_args[] = {"docker", "rm", current->container_id, NULL};
        run_command_fork(rm_args);
    }
    
    log_info("Starting previous version (v%d)", previous->version);
    
    const char *run_args[20];
    int i = 0;
    run_args[i++] = "docker";
    run_args[i++] = "run";
    run_args[i++] = "-d";
    run_args[i++] = "--name";
    run_args[i++] = project;
    run_args[i++] = previous->image_id;
    run_args[i] = NULL;
    
    if (run_command_fork(run_args) != 0) {
        log_error("Failed to start previous version");
        state_free_history(history, count);
        free(current);
        return EXIT_GENERIC;
    }
    
    char container_id[256] = "";
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "docker ps -l -q --filter name=%s", project);
    
    FILE *fp = popen(cmd, "r");
    if (fp) {
        if (fgets(container_id, sizeof(container_id), fp) != NULL) {
            container_id[strcspn(container_id, "\n")] = 0;
        }
        pclose(fp);
    }
    
    Deployment new_dep = {0};
    generate_deployment_id(new_dep.id, sizeof(new_dep.id), project);
    strncpy(new_dep.project, project, sizeof(new_dep.project) - 1);
    strncpy(new_dep.url, previous->url, sizeof(new_dep.url) - 1);
    strncpy(new_dep.image_id, previous->image_id, sizeof(new_dep.image_id) - 1);
    strncpy(new_dep.container_id, container_id, sizeof(new_dep.container_id) - 1);
    new_dep.port = previous->port;
    new_dep.deployed_at = time(NULL);
    new_dep.version = current->version + 1;
    new_dep.status = 1;
    
    if (state_save(&new_dep) == 0) {
        log_info("Rollback complete: %s (v%d)", new_dep.id, new_dep.version);
    } else {
        log_error("Rollback failed to save state");
    }
    
    state_free_history(history, count);
    free(current);
    
    return EXIT_SUCCESS;
}