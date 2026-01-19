#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "deploy.h"
#include "clone.h"

int deploy_repo(const char *repo_url){
    // Placeholder for deployment logic
    printf("Deploying repository from URL: %s\n", repo_url);
    // Here you would add the actual deployment code, e.g., using git commands
int status = clone_project(repo_url);
if(status !=0){
    return 1;
}
if(access("Dockerfile", F_OK) == 0){
    printf("Dockerfile found, building docker image...\n");
    if(system("docker build -t forge_app .") !=0){
        fprintf(stderr, "Docker build failed. Skipping containerization \n");
        return 1;
    }
    printf("Docker image created successfully.\n");
    printf("Running docker container...\n");
    if(system("docker run -d forge_app")!=0){
        fprintf(stderr,"Failed to run docker container.\n");
        return 1;
    }
    else {
    printf("No Dockerfile found. Deployment finished without containerization.\n");
}
}
    return 0;
}