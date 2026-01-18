#include<stdio.h>
#include<string.h>
#include "clone.h"
#include "utils.h"
int clone_project(const char *repo_url){
    printf("Cloning repository from URL: %s\n", repo_url);
    if(repo_url == NULL || strlen(repo_url) == 0){
        fprintf(stderr, "ERROR: Repository URL cannot be empty. \n");
        return 1;
    }
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "git clone %s", repo_url);
    int status = run_command(cmd);
    if(status != 0){
        fprintf(stderr, "ERROR: Failed to clone repository from URL: %s", repo_url);
        return 1;
    }
    printf("Repository cloned successfully from URL: %s\n", repo_url);

    return 0;
}