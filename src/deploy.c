#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "deploy.h"
#include "clone.h"
#include "repo.h"
#include "log.h"
#include "exit_codes.h"
#include "utils.h"
int deploy_repo(const char *repo_url, run_mode_t mode) {
    char project_name[128];

    log_info("Deploying repository from URL: %s", repo_url);

    if (repo_extract_name(repo_url, project_name, sizeof(project_name)) != 0) {
        log_error("Failed to extract name from repo URL");
        return EXIT_GENERIC;
    }

    log_info("Project name detected: %s", project_name);

    if (clone_project(repo_url) != 0) {
        return EXIT_GENERIC;
    }

    if (chdir(project_name) != 0) {
        log_error("Failed to enter project directory");
        return EXIT_GENERIC;
    }

    if (access("Dockerfile", F_OK) == 0) {
        log_info("Dockerfile found, building docker image...");
        const char *build_args[] = {"docker","build","-t","forge_app",".",NULL};
        if(run_command_fork(build_args)!=0){
            log_error("Docker build failed");
            return EXIT_GENERIC;
        }

        log_info("Docker image created successfully.");
        log_info("Running docker container...");

        const char *run_args[20];
        int i = 0;
        run_args[i++] = "docker";
        run_args[i++] = "run";

        switch (mode) {
            case RUN_INTERACTIVE:
                run_args[i++] = "-it";
                break;
            case RUN_DETACHED:
                run_args[i++] = "-d";
                break;
            case RUN_DEFAULT:
            default:
                break;
        }
        // Add container name 
        run_args[i++] = "--name";
        run_args[i++] = project_name;
        // Add image name 
        run_args[i++] = "forge_app";
        run_args[i] = NULL;

        if(run_command_fork(run_args)!=0){
            log_error("Failed to run docker container");
            return EXIT_GENERIC;
        }
    }

    log_info("No Dockerfile found. Deployment finished without containerization.\n");
    return EXIT_SUCCESS;
}
