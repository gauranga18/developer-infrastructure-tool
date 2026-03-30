#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "clone.h"
#include "utils.h"
#include "log.h"
#include "exit_codes.h"

int clone_project_to(const char *repo_url, const char *target_dir) {
    log_info("Cloning repository from URL: %s to %s", repo_url, target_dir);
    
    if (repo_url == NULL || strlen(repo_url) == 0) {
        log_error("Repository URL cannot be empty.");
        return EXIT_BAD_ARGS;
    }
    
    if (target_dir == NULL || strlen(target_dir) == 0) {
        log_error("Target directory cannot be empty.");
        return EXIT_BAD_ARGS;
    }
    
    const char *git_args[] = {
        "git",
        "clone",
        repo_url,
        target_dir,
        NULL
    };
    
    log_info("Running: git clone %s %s", repo_url, target_dir);
    int status = run_command_fork(git_args);
    
    if (status != 0) {
        log_error("Failed to clone repository: %s", repo_url);
        return EXIT_GENERIC;
    }
    
    log_info("Repository cloned successfully to: %s", target_dir);
    return EXIT_SUCCESS;
}