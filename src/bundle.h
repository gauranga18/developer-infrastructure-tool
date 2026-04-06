#ifndef BUNDLE_H
#define BUNDLE_H

typedef struct {
    char project[128];
    char version[32];       // "v4" or empty for latest
    char output_path[512];  // "" = auto-generate in cwd
    char ship_target[256];  // "user@host" or empty
    int no_binary;          // --no-binary flag
    int dry_run;            // --dry-run flag
} BundleFlags;

// Create a bundle
int cmd_bundle(const char *project, int argc, char **argv);

// Deploy from a bundle file
int deploy_from_bundle(const char *bundle_path, int detached);

#endif