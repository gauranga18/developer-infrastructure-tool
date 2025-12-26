#include <stdio.h>

void print_help(const char *prog_name)
{
    printf("Forge — Developer Infrastructure Tool\n\n");

    printf("Usage:\n");
    printf("  %s deploy <repo_url>\n\n", prog_name);

    printf("Commands:\n");
    printf("  deploy    Clone and deploy a repository\n");
    printf("  help      Show this help message\n\n");

    printf("Examples:\n");
    printf("  %s deploy https://github.com/user/repo.git\n", prog_name);
}