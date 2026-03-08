#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "clone.h"
#include "utils.h"
#include "log.h"
#include "exit_codes.h"
int clone_project(const char *repo_url){
    log_info("Cloning repository from URL: %s", repo_url);
    if(repo_url == NULL || strlen(repo_url) == 0){
        log_error("Repository URL cannot be empty.");
        return EXIT_BAD_ARGS;
    }
    const char *git_args[]={
        "git",
        "clone",
        repo_url,
        NULL
    };
    log_info("Running git clone %s",repo_url);
    int status = run_command_fork(git_args);
    if(status != 0){
        log_error("Failed to clone repository from URL: %s", repo_url,status);
        return EXIT_GENERIC;
    }
    log_info("Repository cloned successfully from URL: %s\n", repo_url,status);
    return EXIT_SUCCESS;
}