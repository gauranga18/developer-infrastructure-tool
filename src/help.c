#include <stdio.h>

void print_help(const char *prog_name)
{
    printf("Forge — Developer Infrastructure Tool\n\n");

    printf("Usage:\n");
    printf("  %s init <project_name>\n", prog_name);
    printf("  %s deploy <repo_url> [options]\n\n", prog_name);

    printf("Commands:\n");
    printf("  init      Initialize a new project directory\n");
    printf("  deploy    Clone and deploy a repository\n");
    printf("  help      Show this help message\n\n");
    printf("  version   Show Forge version\n");

    printf("Options:\n");
    printf("  -i        Run container in interactive mode\n");
    printf("  -d        Run container in detached mode\n\n");

    printf("Examples:\n");
    printf("  %s init demo\n", prog_name);
    printf("  %s deploy https://github.com/user/repo.git\n", prog_name);
}
