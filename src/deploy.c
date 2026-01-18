#include <stdio.h>
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
    return 0;
}