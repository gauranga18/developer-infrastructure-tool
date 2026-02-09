#include <stdio.h>
#include <string.h>
#include "help.h"
#include "init.h"
#include "deploy.h"
#include "version.h"

int main(int argc, char *argv[]) {

    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "help") == 0 ||
        strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        return 0;
    }

    /* ---------------- DEPLOY COMMAND ---------------- */
    if (strcmp(argv[1], "deploy") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s deploy <repo_url> [-i|-d]\n", argv[0]);
            return 1;
        }

        run_mode_t mode = RUN_DEFAULT;

        if (argc >= 4) {
            if (strcmp(argv[3], "-i") == 0) {
                mode = RUN_INTERACTIVE;
            } else if (strcmp(argv[3], "-d") == 0) {
                mode = RUN_DETACHED;
            } else {
                fprintf(stderr, "ERROR: Unknown option '%s'\n", argv[3]);
                return 1;
            }
        }

        return deploy_repo(argv[2], mode);
    }

    /* ---------------- INIT COMMAND ---------------- */
    if (strcmp(argv[1], "init") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s init <project_name>\n", argv[0]);
            return 1;
        }
        return init_project(argv[2]);
    }
//Version command 
if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "-v") == 0) {
    printf("Forge version %s\n", FORGE_VERSION);
    return 0;
}
    /* ---------------- UNKNOWN COMMAND ---------------- */
    fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
    printf("Run '%s help' for more information.\n", argv[0]);
    return 1;
}
