#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "deploy.h"
#include "clone.h"
#include "repo.h"
#include "log.h"
#include "exit_codes.h"
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

        if (system("docker build -t forge_app .") != 0) {
            log_error("Docker build failed");
            return EXIT_GENERIC;
        }

        log_info("Docker image created successfully.");
        log_info("Running docker container...");

        const char *run_cmd;

        switch (mode) {
            case RUN_INTERACTIVE:
                run_cmd = "docker run -it forge_app";
                break;
            case RUN_DETACHED:
                run_cmd = "docker run -d forge_app";
                break;
            default:
                run_cmd = "docker run forge_app";
        }

        if (system(run_cmd) != 0) {
            log_error("Failed to run docker container");
            return EXIT_GENERIC;
        }

        return 0;   //correct exit after successful docker run
    }

    log_info("No Dockerfile found. Deployment finished without containerization.\n");
    return EXIT_SUCCESS;
}
