#include <stdio.h>
#include <string.h>
#include "repo.h"
#include "exit_codes.h"
int repo_extract_name(
    const char *repo_url,
    char *out_name,
    size_t out_size
) {
    if (repo_url == NULL || strlen(repo_url) == 0) {
        return EXIT_GENERIC;
    }

    if (out_name == NULL || out_size == 0) {
        return EXIT_GENERIC;
    }

    const char *slash = strrchr(repo_url, '/');
    if (slash == NULL || *(slash + 1) == '\0') {
        return EXIT_GENERIC;
    }

    const char *repo_part = slash + 1;
    size_t name_length = strlen(repo_part);

    if (name_length > 4 &&
        strcmp(repo_part + name_length - 4, ".git") == 0) {
        name_length -= 4;
    }

    if (name_length + 1 > out_size) {
        return EXIT_GENERIC;
    }

    memcpy(out_name, repo_part, name_length);
    out_name[name_length] = '\0';

    return EXIT_SUCCESS;
}
