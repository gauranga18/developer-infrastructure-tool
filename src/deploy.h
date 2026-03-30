#ifndef DEPLOY_H
#define DEPLOY_H

typedef enum{
    RUN_DEFAULT,
    RUN_INTERACTIVE,
    RUN_DETACHED
} run_mode_t;

int deploy_repo(const char *repo_url, run_mode_t mode, int offline_mode, int show_progress);

#endif