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

// Helper to trim newline
static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

// Helper to capture image size in KB
static long get_image_size_kb(const char *image_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "docker image inspect --format '{{.Size}}' %s 2>/dev/null", image_name);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    char output[64] = {0};
    if (fgets(output, sizeof(output), fp) != NULL) {
        long bytes = atol(output);
        pclose(fp);
        return bytes / 1024;
    }
    pclose(fp);
    return 0;
}

// Helper to get image size from tar file
static long get_image_size_from_tar(const char *tar_path) {
    struct stat st;
    if (stat(tar_path, &st) == 0) {
        return st.st_size / 1024;
    }
    return 0;
}

// Helper to read git metadata from a directory
static void read_git_metadata(const char *repo_path, char *sha_out, char *branch_out, char *msg_out) {
    char meta_path[512];
    snprintf(meta_path, sizeof(meta_path), "%s/.forge-git-meta", repo_path);
    
    FILE *mf = fopen(meta_path, "r");
    if (mf) {
        char line[256];
        while (fgets(line, sizeof(line), mf)) {
            if (strncmp(line, "GIT_SHA=", 8) == 0) {
                strncpy(sha_out, line + 8, 63);
                trim_newline(sha_out);
            } else if (strncmp(line, "GIT_BRANCH=", 11) == 0) {
                strncpy(branch_out, line + 11, 127);
                trim_newline(branch_out);
            } else if (strncmp(line, "GIT_MESSAGE=", 12) == 0) {
                strncpy(msg_out, line + 12, 255);
                trim_newline(msg_out);
            }
        }
        fclose(mf);
    }
}

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
    char git_sha[64] = {0};
    char git_branch[128] = {0};
    char git_message[256] = {0};
    long image_size_kb = 0;
    char project_base[128];
    char versioned_name[256];
    char image_id[256] = "forge_app";
    char container_id[256] = "";
    int version;
    int lock_fd = -1;
    int total_steps = 4;
    int current_step = 0;
    ProgressBar pb;
    Spinner spinner;

    setup_signal_handlers();

    if (show_progress) {
        spinner_start(&spinner, "Starting deployment");
    }

    log_info("Deploying repository from URL: %s", repo_url);
    if (offline_mode) {
        log_info("Offline mode enabled - using cached assets only");
    }

    state_init();

    if (repo_extract_name(repo_url, project_base, sizeof(project_base)) != 0) {
        log_error("Failed to extract name from repo URL");
        if (show_progress) spinner_fail(&spinner, "Failed to extract repo name");
        return EXIT_GENERIC;
    }

    log_info("Project name detected: %s", project_base);
    if (show_progress) spinner_complete(&spinner, NULL);

    lock_fd = lock_project(project_base);
    if (lock_fd == -1) {
        log_error("Failed to lock project %s", project_base);
        return EXIT_GENERIC;
    }

    version = get_next_version(project_base);
    build_versioned_name(versioned_name, sizeof(versioned_name), project_base, version);
    log_info("Deploying as: %s (version %d)", versioned_name, version);

    struct stat st;
    if (stat(versioned_name, &st) == 0) {
        log_error("Directory %s already exists!", versioned_name);
        unlock_project(lock_fd);
        return EXIT_GENERIC;
    }

    if (update_next_version_locked(project_base, version) != 0) {
        log_error("Failed to update next version");
        unlock_project(lock_fd);
        return EXIT_GENERIC;
    }

    unlock_project(lock_fd);

    // --- STEP 1: CHECK CACHE OR CLONE ---
    current_step++;
    if (show_progress) {
        progress_init(&pb, "Cloning repository", current_step, total_steps);
    }

    const char *repo_cache = get_repo_cache_path(project_base);
    int repo_cached = (stat(repo_cache, &st) == 0);

    if (repo_cached) {
        log_info("Using cached repository from: %s", repo_cache);
        if (show_progress) {
            for (int p = 0; p <= 100; p++) {
                progress_update(&pb, p, "from cache");
                usleep(10000);
            }
        }
        
        if (copy_dir(repo_cache, versioned_name) != 0) {
            log_error("Failed to copy cached repository");
            if (show_progress) progress_fail(&pb, "copy failed");
            progress_destroy(&pb);
            return EXIT_GENERIC;
        }
        
        // 🔥 FIX 1: Read git metadata from cached repo
        read_git_metadata(repo_cache, git_sha, git_branch, git_message);
        if (git_sha[0]) {
            log_debug("Git metadata from cache: sha=%s branch=%s msg=%s", git_sha, git_branch, git_message);
        }
        
    } else {
        if (offline_mode) {
            log_error("Offline mode enabled but repository not cached. Run once online first.");
            if (show_progress) progress_fail(&pb, "not cached");
            progress_destroy(&pb);
            return EXIT_GENERIC;
        }
        
        if (show_progress) {
            for (int p = 0; p <= 30; p++) {
                progress_update(&pb, p, "cloning...");
                usleep(10000);
            }
        }
        
        if (clone_project_to(repo_url, versioned_name) != 0) {
            if (show_progress) progress_fail(&pb, "clone failed");
            progress_destroy(&pb);
            return EXIT_GENERIC;
        }
        
        // 🔥 FIX 2: Read git metadata from freshly cloned repo
        read_git_metadata(versioned_name, git_sha, git_branch, git_message);
        if (git_sha[0]) {
            log_debug("Git metadata from fresh clone: sha=%s branch=%s msg=%s", git_sha, git_branch, git_message);
        }
        
        if (show_progress) {
            for (int p = 30; p <= 80; p++) {
                progress_update(&pb, p, "caching...");
                usleep(10000);
            }
        }
        
        char cache_repo_parent[512];
        snprintf(cache_repo_parent, sizeof(cache_repo_parent), "%s/cache/repos", get_forge_state_dir());
        ensure_dir(cache_repo_parent);
        
        if (copy_dir(versioned_name, repo_cache) == 0) {
            log_info("Repository cached for offline use at: %s", repo_cache);
            // 🔥 FIX 3: Also copy the git metadata file to cache
            char src_meta[512], dst_meta[512];
            snprintf(src_meta, sizeof(src_meta), "%s/.forge-git-meta", versioned_name);
            snprintf(dst_meta, sizeof(dst_meta), "%s/.forge-git-meta", repo_cache);
            copy_file(src_meta, dst_meta);
        } else {
            log_warn("Failed to cache repository");
        }
        
        if (show_progress) {
            for (int p = 80; p <= 100; p++) {
                progress_update(&pb, p, "done");
                usleep(10000);
            }
        }
    }
    if (show_progress) progress_complete(&pb, NULL);
    if (show_progress) progress_destroy(&pb);

    if (chdir(versioned_name) != 0) {
        log_error("Failed to enter project directory: %s", versioned_name);
        return EXIT_GENERIC;
    }

    if (access("Dockerfile", F_OK) == 0) {
        // --- STEP 2: BUILD DOCKER IMAGE ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Building Docker image", current_step, total_steps);
        }
        
        char project_lower[128];
        strncpy(project_lower, project_base, sizeof(project_lower) - 1);
        project_lower[sizeof(project_lower) - 1] = '\0';
        str_tolower(project_lower);
        
        char image_name[256];
        snprintf(image_name, sizeof(image_name), "forge_%s", project_lower);
        
        const char *image_cache = get_image_cache_path(image_name);
        int image_cached = (stat(image_cache, &st) == 0);
        
        if (image_cached) {
            log_info("Using cached Docker image from: %s", image_cache);
            if (show_progress) {
                for (int p = 0; p <= 100; p++) {
                    progress_update(&pb, p, "loading from cache");
                    usleep(10000);
                }
            }
            
            // 🔥 FIX 4: Get image size from tar file directly
            image_size_kb = get_image_size_from_tar(image_cache);
            log_debug("Image size from cache tar: %ld KB", image_size_kb);
            
            char load_cmd[512];
            snprintf(load_cmd, sizeof(load_cmd), "docker load < %s 2>&1", image_cache);
            if (system(load_cmd) != 0) {
                log_error("Failed to load cached image");
                if (show_progress) progress_fail(&pb, "load failed");
                progress_destroy(&pb);
                return EXIT_GENERIC;
            }
            log_info("Docker image loaded from cache: %s", image_name);
        } else {
            if (offline_mode) {
                log_error("Offline mode enabled but Docker image not cached. Run once online first.");
                if (show_progress) progress_fail(&pb, "not cached");
                progress_destroy(&pb);
                return EXIT_GENERIC;
            }
            
            if (show_progress) {
                for (int p = 0; p <= 50; p++) {
                    progress_update(&pb, p, "building...");
                    usleep(10000);
                }
            }
            
            const char *build_args[] = {"docker", "build", "-t", image_name, ".", NULL};
            if (run_command_fork(build_args) != 0) {
                log_error("Docker build failed for image: %s", image_name);
                if (show_progress) progress_fail(&pb, "build failed");
                progress_destroy(&pb);
                return EXIT_GENERIC;
            }
            log_info("Docker image created successfully: %s", image_name);
            
            // 🔥 FIX 5: Get image size after build
            image_size_kb = get_image_size_kb(image_name);
            log_debug("Image size: %ld KB", image_size_kb);
            
            if (show_progress) {
                for (int p = 50; p <= 100; p++) {
                    progress_update(&pb, p, "saving to cache...");
                    usleep(10000);
                }
            }
            
            char cache_image_parent[512];
            snprintf(cache_image_parent, sizeof(cache_image_parent), "%s/cache/images", get_forge_state_dir());
            ensure_dir(cache_image_parent);
            
            unlink(image_cache);
            
            char save_cmd[512];
            snprintf(save_cmd, sizeof(save_cmd), "docker save %s -o %s 2>&1", image_name, image_cache);
            log_debug("Saving image: %s", save_cmd);
            
            if (system(save_cmd) == 0 && access(image_cache, F_OK) == 0) {
                log_info("Docker image cached for offline use at: %s", image_cache);
            } else {
                log_warn("Failed to cache Docker image");
            }
        }
        if (show_progress) progress_complete(&pb, NULL);
        if (show_progress) progress_destroy(&pb);
        
        strncpy(image_id, image_name, sizeof(image_id) - 1);

        // --- STEP 3: RUN DOCKER CONTAINER ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Starting container", current_step, total_steps);
            for (int p = 0; p <= 100; p++) {
                progress_update(&pb, p, "preparing...");
                usleep(5000);
            }
        }

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

        FILE *fp = popen(run_cmd, "r");
        if (!fp) {
            log_error("Failed to execute docker run");
            if (show_progress) progress_fail(&pb, "execution failed");
            progress_destroy(&pb);
            return EXIT_GENERIC;
        }

        char output[512] = "";
        if (fgets(output, sizeof(output), fp) != NULL) {
            output[strcspn(output, "\n")] = 0;
            
            if (strstr(output, "Error") != NULL || strstr(output, "error") != NULL) {
                log_error("Docker error: %s", output);
                if (show_progress) progress_fail(&pb, output);
                pclose(fp);
                progress_destroy(&pb);
                return EXIT_GENERIC;
            }
            
            if (strlen(output) > 0 && strchr(output, ' ') == NULL) {
                strncpy(container_id, output, sizeof(container_id) - 1);
                log_info("Container ID: %s", container_id);
            }
        }

        int status = pclose(fp);
        if (status != 0) {
            log_error("Docker run failed with exit code: %d", status);
            if (show_progress) progress_fail(&pb, "run failed");
            progress_destroy(&pb);
            return EXIT_GENERIC;
        }
        
        if (show_progress) {
            progress_complete(&pb, NULL);
            progress_destroy(&pb);
        }

        // --- STEP 4: SAVE DEPLOYMENT STATE ---
        current_step++;
        if (show_progress) {
            progress_init(&pb, "Saving deployment state", current_step, total_steps);
            for (int p = 0; p <= 100; p++) {
                progress_update(&pb, p, "writing to disk...");
                usleep(5000);
            }
        }

        Deployment dep = {0};
        
        strncpy(dep.project, versioned_name, sizeof(dep.project) - 1);
        strncpy(dep.project_base, project_base, sizeof(dep.project_base) - 1);
        strncpy(dep.url, repo_url, sizeof(dep.url) - 1);
        strncpy(dep.image_id, image_id, sizeof(dep.image_id) - 1);

        // 🔥 FIX 6: Save all metadata to deployment record
        strncpy(dep.git_sha, git_sha, sizeof(dep.git_sha) - 1);
        strncpy(dep.git_branch, git_branch, sizeof(dep.git_branch) - 1);
        strncpy(dep.git_message, git_message, sizeof(dep.git_message) - 1);
        dep.image_size_kb = image_size_kb;
        strncpy(dep.cache_path, repo_cache, sizeof(dep.cache_path) - 1);
        strncpy(dep.image_tar_path, image_cache, sizeof(dep.image_tar_path) - 1);
        
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
        
        if (state_save(&dep) == 0) {
            log_info("Deployment saved: %s (v%d)", dep.id, dep.version);
        } else {
            log_warn("Failed to save deployment state");
        }
        
        if (show_progress) {
            progress_complete(&pb, NULL);
            progress_destroy(&pb);
        }
        
    } else {
        log_info("No Dockerfile found. Deployment finished without containerization.");
    }

    if (show_progress) {
        printf("\n%s✓%s Deployment complete!\n", COLOR_GREEN, COLOR_RESET);
    }
    
    return EXIT_SUCCESS;
}