#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "clone.h"
#include "utils.h"
#include "log.h"
#include "exit_codes.h"

// Helper to capture command output
static int run_command_capture(const char **argv, char *output, size_t out_size) {
    int pipefd[2];
    if (pipe(pipefd) == -1) return -1;
    
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        execvp(argv[0], (char **)argv);
        exit(127);
    }
    
    close(pipefd[1]);
    ssize_t n = read(pipefd[0], output, out_size - 1);
    if (n > 0) output[n] = '\0';
    close(pipefd[0]);
    
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// Helper to trim newline
static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

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
    
    // Capture git metadata for bundle feature
    char git_sha[64] = {0};
    char git_branch[128] = {0};
    char git_message[256] = {0};
    
    const char *sha_args[] = {"git", "-C", target_dir, "rev-parse", "HEAD", NULL};
    if (run_command_capture(sha_args, git_sha, sizeof(git_sha)) == 0) {
        trim_newline(git_sha);
        log_debug("Git SHA: %s", git_sha);
    }
    
    const char *branch_args[] = {"git", "-C", target_dir, "symbolic-ref", "--short", "HEAD", NULL};
    if (run_command_capture(branch_args, git_branch, sizeof(git_branch)) == 0) {
        trim_newline(git_branch);
        log_debug("Git branch: %s", git_branch);
    }
    
    const char *msg_args[] = {"git", "-C", target_dir, "log", "-1", "--pretty=%s", NULL};
    if (run_command_capture(msg_args, git_message, sizeof(git_message)) == 0) {
        trim_newline(git_message);
        log_debug("Git message: %s", git_message);
    }
    
    // Store metadata in a file for deploy.c to read
    char meta_path[512];
    snprintf(meta_path, sizeof(meta_path), "%s/.forge-git-meta", target_dir);
    FILE *mf = fopen(meta_path, "w");
    if (mf) {
        fprintf(mf, "GIT_SHA=%s\n", git_sha);
        fprintf(mf, "GIT_BRANCH=%s\n", git_branch);
        fprintf(mf, "GIT_MESSAGE=%s\n", git_message);
        fclose(mf);
        log_debug("Git metadata saved to %s", meta_path);
    }
    
    return EXIT_SUCCESS;
}