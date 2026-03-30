#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include "deploy.h"
#include "clone.h"
#include "repo.h"
#include "log.h"
#include "exit_codes.h"
#include "utils.h"
#include "state.h"
#include "config.h"
#include "progress.h"

// Helper function to convert string to lowercase
static void str_tolower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Helper to copy directory recursively (simplified)
static int copy_dir(const char *src, const char *dst) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp -r %s %s", src, dst);
    return system(cmd);
}

int deploy_repo(const char *repo_url, run_mode_t mode, int offline_mode, int show_progress) {
    char project_base[128];
    char versioned_name[256];
    char image_id[256] = "forge_app";
    char container_id[256] = "";
    int version;
    int lock_fd = -1;
    int total_steps = 4;
    int current_step = 0;
    ProgressBar pb;

    // Setup signal handlers for this process
    setup_signal_handlers();

    if (show_progress) {
        progress_spinner("Starting deployment", 0);
    }

    log_info("Deploying repository from URL: %s", repo_url);
    if (offline_mode) {
        log_info("Offline mode enabled - using cached assets only");
    }

    state_init();

    if (repo_extract_name(repo_url, project_base, sizeof(project_base)) != 0) {
        log_error("Failed to extract name from repo URL");
        if (show_progress) progress_spinner("Failed to extract repo name", 1);
        return EXIT_GENERIC;
    }

    log_info("Project name detected: %s", project_base);

    // --- CRITICAL SECTION: Lock the project ---
    lock_fd = lock_project(project_base);
    if (lock_fd == -1) {
        log_error("Failed to lock project %s", project_base);
        if (show_progress) progress_spinner("Failed to lock project", 1);
        return EXIT_GENERIC;
    }

    // Get next version number
    version = get_next_version(project_base);
    build_versioned_name(versioned_name, sizeof(versioned_name), project_base, version);
    log_info("Deploying as: %s (version %d)", versioned_name, version);

    // Check if directory already exists
    struct stat st;
    if (stat(versioned_name, &st) == 0) {
        log_error("Directory %s already exists! This should not happen.", versioned_name);
        unlock_project(lock_fd);
        if (show_progress) progress_spinner("Directory already exists", 1);
        return EXIT_GENERIC;
    }

    // Update registry to reserve this version
    if (update_next_version_locked(project_base, version) != 0) {
        log_error("Failed to update next version");
        unlock_project(lock_fd);
        if (show_progress) progress_spinner("Failed to update version", 1);
        return EXIT_GENERIC;
    }

    // Release the lock
    unlock_project(lock_fd);
    // --- END CRITICAL SECTION ---

    // --- STEP 1: CHECK CACHE OR CLONE ---
    current_step++;
    if (show_progress) {
        progress_init(&pb, "Cloning repository", current_step, total_steps);
    }

    const char *repo_cache = get_repo_cache_path(project_base);
    int repo_cached = (stat(repo_cache, &st) == 0);

    if (repo_cached) {
        log_info("Using cached repository from: %s", repo_cache);
        if (show_progress) progress_update(&pb, 50, "from cache");
        
        // Copy cached repo instead of cloning
        if (copy_dir(repo_cache, versioned_name) != 0) {
            log_error("Failed to copy cached repository");
            if (show_progress) progress_fail(&pb, "copy failed");
            return EXIT_GENERIC;
        }
        if (show_progress) progress_update(&pb, 100, "done");
    } else {
        if (offline_mode) {
            log_error("Offline mode enabled but repository not cached. Run once online first.");
            if (show_progress) progress_fail(&pb, "not cached");
            return EXIT_GENERIC;
        }
        
        if (show_progress) progress_update(&pb, 30, "cloning...");
        
        // Clone fresh
        if (clone_project_to(repo_url, versioned_name) != 0) {
            if (show_progress) progress_fail(&pb, "clone failed");
            return EXIT_GENERIC;
        }
        
        if (show_progress) progress_update(&pb, 80, "caching...");
        
        // Save to cache for future offline use
        char cache_repo_parent[512];
        snprintf(cache_repo_parent, sizeof(cache_repo_parent), "%s/cache/repos", get_forge_state_dir());
        ensure_dir(cache_repo_parent);
        
        if (copy_dir(versioned_name, repo_cache) == 0) {
            log_info("Repository cached for offline use at: %s", repo_cache);
        } else {
            log_warn("Failed to cache repository (offline mode will not work for this repo)");
        }
        if (show_progress) progress_update(&pb, 100, "done");
    }
    if (show_progress) progress_complete(&pb, NULL);

    if (chdir(versioned_name) != 0) {
        log_error("Failed to enter project directory: %s", versioned_name);
        if (show_progress) progress_spinner("Failed to enter directory", 1);
        return EXIT_GENERIC;
    }

    if (access("Dockerfile", F_OK) == 0) {
        // --- STEP 2: BUILD DOCKER IMAGE ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Building Docker image", current_step, total_steps);
        }
        
        // Create lowercase version for Docker tag
        char project_lower[128];
        strncpy(project_lower, project_base, sizeof(project_lower) - 1);
        project_lower[sizeof(project_lower) - 1] = '\0';
        str_tolower(project_lower);
        
        char image_name[256];
        snprintf(image_name, sizeof(image_name), "forge_%s", project_lower);
        
        // --- OFFLINE MODE: CHECK IMAGE CACHE ---
        const char *image_cache = get_image_cache_path(image_name);
        int image_cached = (stat(image_cache, &st) == 0);
        
        if (image_cached) {
            log_info("Using cached Docker image from: %s", image_cache);
            if (show_progress) progress_update(&pb, 50, "loading from cache");
            
            char load_cmd[512];
            snprintf(load_cmd, sizeof(load_cmd), "docker load < %s 2>&1", image_cache);
            if (system(load_cmd) != 0) {
                log_error("Failed to load cached image");
                if (show_progress) progress_fail(&pb, "load failed");
                return EXIT_GENERIC;
            }
            log_info("Docker image loaded from cache: %s", image_name);
            if (show_progress) progress_update(&pb, 100, "loaded");
        } else {
            if (offline_mode) {
                log_error("Offline mode enabled but Docker image not cached. Run once online first.");
                if (show_progress) progress_fail(&pb, "not cached");
                return EXIT_GENERIC;
            }
            
            if (show_progress) progress_update(&pb, 30, "building...");
            
            // Build fresh
            const char *build_args[] = {"docker", "build", "-t", image_name, ".", NULL};
            if (run_command_fork(build_args) != 0) {
                log_error("Docker build failed for image: %s", image_name);
                if (show_progress) progress_fail(&pb, "build failed");
                return EXIT_GENERIC;
            }
            log_info("Docker image created successfully: %s", image_name);
            
            if (show_progress) progress_update(&pb, 70, "saving to cache...");
            
            // --- SAVE IMAGE TO CACHE ---
            char cache_image_parent[512];
            snprintf(cache_image_parent, sizeof(cache_image_parent), "%s/cache/images", get_forge_state_dir());
            ensure_dir(cache_image_parent);
            
            // Remove existing cache file if it exists
            unlink(image_cache);
            
            // Save directly to cache directory
            char save_cmd[512];
            snprintf(save_cmd, sizeof(save_cmd), "docker save %s -o %s 2>&1", image_name, image_cache);
            log_debug("Saving image: %s", save_cmd);
            
            if (system(save_cmd) == 0 && access(image_cache, F_OK) == 0) {
                log_info("Docker image cached for offline use at: %s", image_cache);
                if (show_progress) progress_update(&pb, 100, "cached");
            } else {
                log_warn("Failed to cache Docker image (offline mode will not work for this image)");
                if (show_progress) progress_update(&pb, 100, "build complete (cache failed)");
            }
        }
        if (show_progress) progress_complete(&pb, NULL);
        
        strncpy(image_id, image_name, sizeof(image_id) - 1);

        // --- STEP 3: RUN DOCKER CONTAINER ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Starting container", current_step, total_steps);
        }
        
        if (show_progress) progress_update(&pb, 30, "preparing...");

        // Build docker run command with output capture
        char run_cmd[512];
        const char *mode_flag = "";

        switch (mode) {
            case RUN_INTERACTIVE:
                mode_flag = "-it";
                break;
            case RUN_DETACHED:
                mode_flag = "-d";
                break;
            default:
                mode_flag = "";
                break;
        }

        snprintf(run_cmd, sizeof(run_cmd), 
                 "docker run %s --name %s %s 2>&1",
                 mode_flag, versioned_name, image_name);

        log_info("Running: %s", run_cmd);

        // Execute and capture output directly
        FILE *fp = popen(run_cmd, "r");
        if (!fp) {
            log_error("Failed to execute docker run");
            if (show_progress) progress_fail(&pb, "execution failed");
            return EXIT_GENERIC;
        }

        char output[512] = "";
        if (fgets(output, sizeof(output), fp) != NULL) {
            output[strcspn(output, "\n")] = 0;
            
            if (strstr(output, "Error") != NULL || strstr(output, "error") != NULL) {
                log_error("Docker error: %s", output);
                if (show_progress) progress_fail(&pb, output);
                pclose(fp);
                return EXIT_GENERIC;
            }
            
            if (strlen(output) > 0 && strchr(output, ' ') == NULL) {
                strncpy(container_id, output, sizeof(container_id) - 1);
                log_info("Container ID: %s", container_id);
                if (show_progress) progress_update(&pb, 100, "running");
            }
        }

        int status = pclose(fp);
        if (status != 0) {
            log_error("Docker run failed with exit code: %d", status);
            if (show_progress) progress_fail(&pb, "run failed");
            return EXIT_GENERIC;
        }
        if (show_progress) progress_complete(&pb, NULL);

        // --- STEP 4: SAVE DEPLOYMENT STATE ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Saving deployment state", current_step, total_steps);
        }
        
        if (show_progress) progress_update(&pb, 50, "creating record...");

        // Create deployment record
        Deployment dep = {0};
        
        strncpy(dep.project, versioned_name, sizeof(dep.project) - 1);
        strncpy(dep.url, repo_url, sizeof(dep.url) - 1);
        strncpy(dep.image_id, image_id, sizeof(dep.image_id) - 1);
        
        if (strlen(container_id) > 0) {
            strncpy(dep.container_id, container_id, sizeof(dep.container_id) - 1);
        } else {
            strncpy(dep.container_id, "interactive", sizeof(dep.container_id) - 1);
        }
        
        dep.port = 8080;
        dep.deployed_at = time(NULL);
        dep.version = version;
        dep.status = 1;
        
        generate_deployment_id(dep.id, sizeof(dep.id), versioned_name);
        
        if (show_progress) progress_update(&pb, 80, "writing to disk...");
        
        if (state_save(&dep) == 0) {
            log_info("Deployment saved: %s (v%d)", dep.id, dep.version);
            if (show_progress) progress_update(&pb, 100, "saved");
        } else {
            log_warn("Failed to save deployment state");
            if (show_progress) progress_update(&pb, 100, "save failed");
        }
        if (show_progress) progress_complete(&pb, NULL);
        
    } else {
        log_info("No Dockerfile found. Deployment finished without containerization.");
        if (show_progress) progress_spinner("No Dockerfile found", 1);
    }

    if (show_progress) {
        printf("\n%s✓%s Deployment complete!\n", COLOR_GREEN, COLOR_RESET);
    }
    
    return EXIT_SUCCESS;
}