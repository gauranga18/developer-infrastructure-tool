#include <stdio.h>
#include <string.h>
#include "utils.h"

int main(int argc, char *argv[]) {

    /* 1. Validate argument count */
    if (argc < 3) {
        printf("Usage: %s deploy <repo_url>\n", argv[0]);
        return 1;
    }

    /* 2. Validate command */
    if (strcmp(argv[1], "deploy") != 0) {
        printf("Unknown command: %s\n", argv[1]);
        printf("Usage: %s deploy <repo_url>\n", argv[0]);
        return 1;
    }

    /* 3. Extract repo URL */
    const char *repo_url = argv[2];

    /* 4. Build git clone command */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "git clone %s", repo_url);

    /* 5. Execute command */
    int ret = run_command(cmd);

    /* 6. Handle failure */
    if (ret != 0) {
        printf("Deployment failed :( \n");
        return 1;
    }

    printf("Repository cloned successfully :) \n");
    return 0;
}
