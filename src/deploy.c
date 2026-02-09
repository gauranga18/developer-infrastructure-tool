#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "deploy.h"
#include "clone.h"
#include "repo.h"
int deploy_repo(const char *repo_url, run_mode_t mode) {
    char project_name[128];

    printf("Deploying repository from URL: %s\n", repo_url);

    if (repo_extract_name(repo_url, project_name, sizeof(project_name)) != 0) {
        fprintf(stderr, "ERROR: Failed to extract name from repo URL\n");
        return 1;
    }

    printf("Project name detected: %s\n", project_name);

    if (clone_project(repo_url) != 0) {
        return 1;
    }

    if (chdir(project_name) != 0) {
        perror("ERROR: Failed to enter project directory");
        return 1;
    }

    if (access("Dockerfile", F_OK) == 0) {
        printf("Dockerfile found, building docker image...\n");

        if (system("docker build -t forge_app .") != 0) {
            fprintf(stderr, "Docker build failed\n");
            return 1;
        }

        printf("Docker image created successfully.\n");
        printf("Running docker container...\n");

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
            fprintf(stderr, "ERROR: Failed to run docker container\n");
            return 1;
        }

        return 0;   // ✅ correct exit after successful docker run
    }

    printf("No Dockerfile found. Deployment finished without containerization.\n");
    return 0;
}
