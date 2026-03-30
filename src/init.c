#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "log.h"
#include "init.h"
#include "exit_codes.h"
#include "templates.h"

static int create_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) {
        log_error("Failed to create %s: %s", path, strerror(errno));
        return -1;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

static int create_directory(const char *path) {
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        log_error("Failed to create %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

int init_project(const InitOptions *opts) {
    const char *project_name = opts->project_name;
    
    if (project_name == NULL || strlen(project_name) == 0) {
        log_error("Project name cannot be empty.");
        return EXIT_BAD_ARGS;
    }
    
    if (strchr(project_name, ' ') != NULL || strchr(project_name, '/') != NULL) {
        log_error("Project name cannot contain spaces or slashes.");
        return EXIT_BAD_ARGS;
    }
    
    // Create main project directory
    if (create_directory(project_name) != 0) {
        return EXIT_GENERIC;
    }
    
    // Create src directory
    char path[256];
    snprintf(path, sizeof(path), "%s/src", project_name);
    create_directory(path);
    
    // Create tests directory
    snprintf(path, sizeof(path), "%s/tests", project_name);
    create_directory(path);
    
    // Create main source file
    snprintf(path, sizeof(path), "%s/src/main.%s", project_name,
             opts->language == LANG_PYTHON ? "py" :
             opts->language == LANG_NODE ? "js" :
             opts->language == LANG_GO ? "go" :
             opts->language == LANG_RUST ? "rs" : "c");
    const char *main_content = get_main_template(opts->language, project_name);
    create_file(path, main_content);
    
    // Create test file
    snprintf(path, sizeof(path), "%s/tests/test_main.%s", project_name,
             opts->language == LANG_PYTHON ? "py" :
             opts->language == LANG_NODE ? "js" :
             opts->language == LANG_GO ? "go" :
             opts->language == LANG_RUST ? "rs" : "c");
    const char *test_content = get_test_template(opts->language);
    create_file(path, test_content);
    
    // Create Dockerfile
    snprintf(path, sizeof(path), "%s/Dockerfile", project_name);
    const char *dockerfile = get_dockerfile_template(opts->language);
    create_file(path, dockerfile);
    
    // Create .gitignore
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    const char *gitignore = get_gitignore_template(opts->language);
    create_file(path, gitignore);
    
    // Create .forge.yaml
    snprintf(path, sizeof(path), "%s/.forge.yaml", project_name);
    const char *yaml = get_forge_yaml_template(project_name, opts->language);
    create_file(path, yaml);
    
    // Create README.md
    snprintf(path, sizeof(path), "%s/README.md", project_name);
    const char *readme = get_readme_template(project_name, opts->language);
    create_file(path, readme);
    
    // Create .env.example
    snprintf(path, sizeof(path), "%s/.env.example", project_name);
    const char *env = get_env_example_template(opts->language);
    create_file(path, env);
    
    // Create CI config if requested
    if (opts->ci == CI_GITHUB) {
        snprintf(path, sizeof(path), "%s/.github/workflows/ci.yml", project_name);
        create_directory(path);
        char *p = strrchr(path, '/');
        if (p) *p = '\0';
        create_directory(path);
        *p = '/';
        const char *ci = get_ci_template(opts->ci, opts->language);
        create_file(path, ci);
    }
    
    log_info("Project '%s' initialized successfully with %s template!",
             project_name,
             opts->language == LANG_PYTHON ? "Python" :
             opts->language == LANG_NODE ? "Node.js" :
             opts->language == LANG_GO ? "Go" :
             opts->language == LANG_RUST ? "Rust" : "C");
    
    return EXIT_SUCCESS;
}