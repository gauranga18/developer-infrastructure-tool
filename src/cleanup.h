#ifndef CLEANUP_H
#define CLEANUP_H

#include <time.h>
#include <stdbool.h>

typedef struct {
    int dry_run;
    int keep_versions;
    int older_than_days;
    int prune_images;
    int remove_all;
} CleanupOptions;

int run_cleanup(CleanupOptions *opts);

#endif