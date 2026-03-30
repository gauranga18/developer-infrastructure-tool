#include <stdio.h>
#include "help.h"

void print_help(const char *prog_name)
{
    printf("Forge — Developer Infrastructure Tool\n\n");

    printf("Usage:\n");
    printf("  %s init <project_name>\n", prog_name);
    printf("    --type <lang>   Language: python, node, go, rust, c (default: python)\n");
    printf("    --ci <type>     CI: github (creates GitHub Actions workflow)\n");
    printf("  %s deploy <repo_url> [options]\n", prog_name);
    printf("  %s cleanup [options]\n", prog_name);
    printf("  %s --list\n", prog_name);
    printf("  %s --status <project>\n", prog_name);
    printf("  %s --logs <project>\n", prog_name);
    printf("  %s --rollback <project>\n\n", prog_name);

    printf("Commands:\n");
    printf("  init              Initialize a new project directory\n");
    printf("  deploy            Clone and deploy a repository\n");
    printf("  cleanup           Remove unused deployments, containers, and images\n");
    printf("    --dry-run       Show what would be deleted without actually deleting\n");
    printf("    --keep N        Keep the last N versions of each project\n");
    printf("    --older-than N  Delete deployments older than N days\n");
    printf("    --prune-images  Remove unused Docker images\n");
    printf("    --all           Remove everything except the current deployment\n");
    printf("  --list            List all deployments\n");
    printf("  --status          Show status of a deployment\n");
    printf("  --logs            View container logs\n");
    printf("  --rollback        Rollback to previous version\n\n");

    printf("Global Flags:\n");
    printf("  -v                Show Forge version\n");
    printf("  -h/--help         Show this help message\n");
    printf("  -ver/--verbose    Enable debug logging\n");
    printf("  -q/--quiet        Only show errors, suppress info/warnings\n\n");

    printf("Deploy Options:\n");
    printf("  -i                Run container in interactive mode\n");
    printf("  -d                Run container in detached mode\n\n");
    
    printf("Remote Options:\n");
    printf("  ssh user@host <cmd> Execute forge command remotely via SSH\n");
    printf("  ssh user@host deploy <url> Deploy to remote server\n");
    printf("  ssh user@host --status <project> Check remote status\n\n");
    
    printf("Examples:\n");
    printf("  %s init demo\n", prog_name);
    printf("  %s deploy https://github.com/user/repo.git -d\n", prog_name);
    printf("  %s --status demo\n", prog_name);
    printf("  %s --logs demo\n", prog_name);
    printf("  %s --rollback demo\n", prog_name);
    printf("  %s cleanup --keep 3 --dry-run\n", prog_name);
    printf("  %s cleanup --older-than 7 --prune-images\n", prog_name);
}