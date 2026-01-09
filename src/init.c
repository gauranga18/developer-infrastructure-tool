#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

#include "init.h"
int init_project(const char *project_name){
    if(project_name == NULL || strlen(project_name) == 0){
        fprintf(stderr,"ERROR: Project name cannot be empty. \n");
    return 1;
    }if(strchr(project_name, ' ') != NULL || strchr(project_name, '/') != NULL){
        fprintf(stderr,"ERROR: Project name cannot contain spaces or slashes. \n");
        return 1;
    }
    if(mkdir(project_name, 0755) != 0 ){
        if(errno == EEXIST){
            fprintf(stderr, "ERROR: Directory '%s' already exists. \n", project_name);
        return 1;
        }else{
            perror("mkdir failed");
        }
        return 1;
    }
    char path[256];
    snprintf(path, sizeof(path), "%s/src", project_name);
    if(mkdir(path,0755) !=0){
        perror("mkdir src failed ");
        return 1;
    }
    snprintf(path, sizeof(path), "%s/include", project_name);
    if(mkdir(path,0755) !=0){
        perror("mkdir include failed ");
        return 1;
    }
    snprintf(path, sizeof(path), "%s/build", project_name);
    if(mkdir(path,0755) !=0){
        perror("mkdir build failed");
        return 1;
    }
    snprintf(path, sizeof(path), "%s/README.md", project_name);
    FILE *readme = fopen(path, "w");
    if(readme == NULL){
        perror("README.md failed to create");
        return 1;
    }
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    FILE *gitignore = fopen(path, "w");
    if(gitignore == NULL){
        perror(".gitignore failed to create ");
        return 1;
    }
    fclose(readme);
    fclose(gitignore);
    printf("Project '%s' initialized successfully! \n", project_name);

    return 0;
}

