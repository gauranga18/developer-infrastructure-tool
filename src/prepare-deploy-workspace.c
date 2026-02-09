#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prepare_deploy_workspace.h"

int prepare_deploy_workspace(
    const char *project_name,
    char *out_path,
    size_t path_size
){
    if (project_name == NULL || strlen(project_name) == 0) {
        fprintf(stderr, "ERROR: Project name cannot be empty.\n");
        return 1;
    }

    if (out_path == NULL || path_size == 0) {
        fprintf(stderr, "ERROR: Output path buffer is invalid.\n");
        return 1;
    }

    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "ERROR: HOME environment variable is not set.\n");
        return 1;
    }

    printf("HOME directory: %s\n", home);

    return 0;
}
