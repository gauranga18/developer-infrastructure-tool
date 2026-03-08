#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "log.h"
#include "init.h"
#include "exit_codes.h"

int init_project(const char *project_name) {
    if (project_name == NULL || strlen(project_name) == 0) {
        log_error("Project name cannot be empty.");
        return EXIT_BAD_ARGS;
    }
    
    if (strchr(project_name, ' ') != NULL || strchr(project_name, '/') != NULL) {
        log_error("Project name cannot contain spaces or slashes.");
        return EXIT_BAD_ARGS;
    }
    
    // Create main project directory
    if (mkdir(project_name, 0755) != 0) {
        if (errno == EEXIST) {
            log_error("Directory '%s' already exists.", project_name);
            return EXIT_GENERIC;
        } else {
            log_error("Failed to create directory '%s': %s", project_name, strerror(errno));
            return EXIT_GENERIC;
        }
    }
    
    // Create src directory
    char path[256];
    snprintf(path, sizeof(path), "%s/src", project_name);
    if (mkdir(path, 0755) != 0) {
        log_error("Failed to create %s: %s", path, strerror(errno));
        return EXIT_GENERIC;
    }
    
    // Create include directory
    snprintf(path, sizeof(path), "%s/include", project_name);
    if (mkdir(path, 0755) != 0) {
        log_error("Failed to create %s: %s", path, strerror(errno));
        return EXIT_GENERIC;
    }
    
    // Create build directory
    snprintf(path, sizeof(path), "%s/build", project_name);
    if (mkdir(path, 0755) != 0) {
        log_error("Failed to create %s: %s", path, strerror(errno));
        return EXIT_GENERIC;
    }
    
    // Create .forge.yaml
    snprintf(path, sizeof(path), "%s/.forge.yaml", project_name);
    FILE *yaml = fopen(path, "w");
    if (yaml == NULL) {
        log_error("Failed to create .forge.yaml: %s", strerror(errno));
        return EXIT_GENERIC;
    }
    fprintf(yaml, 
        "name: %s\n"
        "version: 1\n"
        "runtime: docker\n",
        project_name);
    fclose(yaml);
    
    // Create README.md
    snprintf(path, sizeof(path), "%s/README.md", project_name);
    FILE *readme = fopen(path, "w");
    if (readme == NULL) {
        log_error("Failed to create README.md: %s", strerror(errno));
        return EXIT_GENERIC;
    }
    fprintf(readme, "# %s\n\nProject initialized with Forge.\n", project_name);
    fclose(readme);
    
    // Create .gitignore
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    FILE *gitignore = fopen(path, "w");
    if (gitignore == NULL) {
        log_error("Failed to create .gitignore: %s", strerror(errno));
        return EXIT_GENERIC;
    }
    fprintf(gitignore, "build/\n*.log\n");
    fclose(gitignore);
    
    log_info("Project '%s' initialized successfully!", project_name);
    return EXIT_SUCCESS;
}