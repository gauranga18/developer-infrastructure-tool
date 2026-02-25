#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prepare_deploy_workspace.h"
#include "exit_codes.h"


int prepare_deploy_workspace(
    const char *project_name,
    char *out_path,
    size_t path_size
){
    if (project_name == NULL || strlen(project_name) == 0) {
        log_error("Project name cannot be empty.");
        return EXIT_BAD_ARGS;
    }

    if (out_path == NULL || path_size == 0) {
        log_error("Output path buffer is invalid.");
        return EXIT_BAD_ARGS;
    }

    const char *home = getenv("HOME");
    if (home == NULL) {
        log_error("HOME environment variable is not set.");
        return EXIT_BAD_ARGS;
    }

    log_info("HOME directory: %s", home);

    return EXIT_SUCCESS;
}
