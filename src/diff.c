#include "diff.h"
#include "state.h"
#include "utils.h"
#include "log.h"
#include "exit_codes.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    int patch;          // --patch: show full unified diff
    int meta_only;      // --meta-only: skip git output
    int json_out;       // --json: machine-readable output
    char ver_a[32];     // explicit version or empty
    char ver_b[32];     // explicit version or empty
} DiffFlags;

// Helper to format timestamp
static void format_time_str(char *buf, size_t size, time_t t) {
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
}

// Helper to run git command and capture output
static int run_git_cmd(const char *repo_path, const char **args, char *output, size_t out_size) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git -C %s ", repo_path);
    
    for (int i = 0; args[i]; i++) {
        strcat(cmd, args[i]);
        strcat(cmd, " ");
    }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    
    if (output && out_size > 0) {
        size_t total = 0;
        while (fgets(output + total, out_size - total, fp)) {
            total = strlen(output);
            if (total >= out_size - 1) break;
        }
    }
    
    int status = pclose(fp);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// Helper to get commit count between two SHAs
static int get_commit_count(const char *repo_path, const char *sha_a, const char *sha_b) {
    char sha_range[130];
    snprintf(sha_range, sizeof(sha_range), "%s..%s", sha_a, sha_b);
    
    const char *args[] = {"rev-list", "--count", sha_range, NULL};
    char output[32] = {0};
    
    if (run_git_cmd(repo_path, args, output, sizeof(output)) == 0) {
        return atoi(output);
    }
    return 0;
}

// Helper to get git stat output
static char* get_git_stat(const char *repo_path, const char *sha_a, const char *sha_b) {
    static char output[4096];
    memset(output, 0, sizeof(output));
    
    const char *args[] = {"diff", "--stat", sha_a, sha_b, NULL};
    run_git_cmd(repo_path, args, output, sizeof(output));
    return output;
}

// Helper to stream full git patch
static int stream_git_patch(const char *repo_path, const char *sha_a, const char *sha_b) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git -C %s diff %s %s", repo_path, sha_a, sha_b);
    return system(cmd);
}

// Helper to load deployment by versioned name
static int load_deployment_by_version(const char *base_project, int version, Deployment *out) {
    char versioned_name[256];
    snprintf(versioned_name, sizeof(versioned_name), "%s-v%d", base_project, version);
    
    Deployment *dep = state_find_latest(versioned_name);
    if (!dep) return -1;
    
    memcpy(out, dep, sizeof(Deployment));
    free(dep);
    return 0;
}

// Helper to get latest two deployments by base name using state_get_by_base
static int get_latest_two_by_base(const char *base_project, Deployment *newest, Deployment *second) {
    int count;
    Deployment **history = state_get_by_base(base_project, &count);
    
    if (!history || count < 2) {
        if (history) {
            for (int i = 0; i < count; i++) free(history[i]);
            free(history);
        }
        return -1;
    }
    
    memcpy(newest, history[0], sizeof(Deployment));
    memcpy(second, history[1], sizeof(Deployment));
    
    for (int i = 0; i < count; i++) free(history[i]);
    free(history);
    
    return 0;
}

// Helper to parse diff flags
static void parse_diff_flags(DiffFlags *flags, int argc, char **argv) {
    memset(flags, 0, sizeof(DiffFlags));
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--patch") == 0) {
            flags->patch = 1;
        }
        else if (strcmp(argv[i], "--meta-only") == 0) {
            flags->meta_only = 1;
        }
        else if (strcmp(argv[i], "--json") == 0) {
            flags->json_out = 1;
        }
        else if (strcmp(argv[i], "-v") == 0 && i + 2 < argc) {
            strncpy(flags->ver_a, argv[i + 1], 31);
            strncpy(flags->ver_b, argv[i + 2], 31);
            i += 2;
        }
    }
}

// Helper to print human-readable diff
static int print_diff_human(Deployment *a, Deployment *b, DiffFlags *flags) {
    printf("\n");
    printf("────────────────────────────────────────────────\n");
    printf("  Project : %s\n", a->project_base);
    printf("  Comparing: v%d  →  v%d\n", a->version, b->version);
    printf("────────────────────────────────────────────────\n\n");
    
    // Git section
    if (!flags->meta_only && a->git_sha[0] && b->git_sha[0]) {
        printf("  GIT\n  ───\n");
        
        if (strcmp(a->git_branch, b->git_branch) == 0) {
            printf("  Branch  : %s\n", a->git_branch);
        } else {
            printf("  Branch  : %s → %s\n", a->git_branch, b->git_branch);
        }
        
        printf("  SHA     : %.8s  →  %.8s\n", a->git_sha, b->git_sha);
        printf("  Message : \"%s\"\n", b->git_message);
        
        if (strcmp(a->git_sha, b->git_sha) == 0) {
            printf("\n  ℹ  Same commit — no code changes.\n");
        } else {
            int commits = get_commit_count(a->cache_path, a->git_sha, b->git_sha);
            printf("  Commits between: %d\n\n", commits);
            
            printf("  CODE CHANGES\n  ────────────\n");
            char *stat = get_git_stat(a->cache_path, a->git_sha, b->git_sha);
            if (stat && stat[0]) {
                char *line = strtok(stat, "\n");
                while (line) {
                    printf("  %s\n", line);
                    line = strtok(NULL, "\n");
                }
            }
        }
        printf("\n");
    } else if (!flags->meta_only && (!a->git_sha[0] || !b->git_sha[0])) {
        printf("  GIT\n  ───\n");
        printf("  ⚠  Git metadata not available (deployed before v4)\n\n");
    }
    
    // Metadata section
    printf("  METADATA\n  ────────\n");
    
    char time_a[32], time_b[32];
    format_time_str(time_a, sizeof(time_a), a->deployed_at);
    format_time_str(time_b, sizeof(time_b), b->deployed_at);
    printf("  Deployed  : %s  →  %s\n", time_a, time_b);
    
    if (a->image_size_kb > 0 && b->image_size_kb > 0) {
        long delta = b->image_size_kb - a->image_size_kb;
        printf("  Image size: %.1f MB  →  %.1f MB  (%+.1f MB)\n",
               a->image_size_kb / 1024.0,
               b->image_size_kb / 1024.0,
               delta / 1024.0);
    }
    
    printf("\n────────────────────────────────────────────────\n");
    if (!flags->patch && strcmp(a->git_sha, b->git_sha) != 0) {
        printf("  Run with --patch to see full diff\n");
    }
    printf("────────────────────────────────────────────────\n\n");
    
    // Full patch (streaming)
    if (flags->patch && a->git_sha[0] && b->git_sha[0] && strcmp(a->git_sha, b->git_sha) != 0) {
        printf("\n  FULL DIFF\n  ─────────\n\n");
        stream_git_patch(a->cache_path, a->git_sha, b->git_sha);
    }
    
    return 0;
}

// Helper to print JSON diff
static int print_diff_json(Deployment *a, Deployment *b) {
    printf("{\n");
    printf("  \"project\": \"%s\",\n", a->project_base);
    printf("  \"from\": {\n");
    printf("    \"version\": %d,\n", a->version);
    printf("    \"sha\": \"%s\",\n", a->git_sha);
    printf("    \"branch\": \"%s\",\n", a->git_branch);
    printf("    \"message\": \"%s\",\n", a->git_message);
    printf("    \"deployed_at\": %ld,\n", a->deployed_at);
    printf("    \"image_size_kb\": %ld\n", a->image_size_kb);
    printf("  },\n");
    printf("  \"to\": {\n");
    printf("    \"version\": %d,\n", b->version);
    printf("    \"sha\": \"%s\",\n", b->git_sha);
    printf("    \"branch\": \"%s\",\n", b->git_branch);
    printf("    \"message\": \"%s\",\n", b->git_message);
    printf("    \"deployed_at\": %ld,\n", b->deployed_at);
    printf("    \"image_size_kb\": %ld\n", b->image_size_kb);
    printf("  },\n");
    
    if (a->git_sha[0] && b->git_sha[0] && strcmp(a->git_sha, b->git_sha) != 0) {
        int commits = get_commit_count(a->cache_path, a->git_sha, b->git_sha);
        long size_delta = b->image_size_kb - a->image_size_kb;
        
        printf("  \"commits_between\": %d,\n", commits);
        printf("  \"image_size_delta_kb\": %ld,\n", size_delta);
        
        char *stat = get_git_stat(a->cache_path, a->git_sha, b->git_sha);
        if (stat) {
            printf("  \"stat\": \"%s\"\n", stat);
        } else {
            printf("  \"stat\": \"\"\n");
        }
    } else {
        printf("  \"commits_between\": 0,\n");
        printf("  \"image_size_delta_kb\": 0,\n");
        printf("  \"stat\": \"\"\n");
    }
    
    printf("}\n");
    return 0;
}

// ─── Main entry point ─────────────────────────────────────────────────────────

int cmd_diff(const char *project, int argc, char **argv) {
    DiffFlags flags;
    parse_diff_flags(&flags, argc, argv);
    
    Deployment a = {0};
    Deployment b = {0};
    int ret = 0;
    
    // Check if specific versions were provided
    if (flags.ver_a[0] && flags.ver_b[0]) {
        int version_a = atoi(flags.ver_a);
        int version_b = atoi(flags.ver_b);
        
        if (version_a == 0 || version_b == 0) {
            log_error("Invalid version numbers: %s %s", flags.ver_a, flags.ver_b);
            return EXIT_BAD_ARGS;
        }
        
        if (load_deployment_by_version(project, version_a, &a) != 0) {
            log_error("Version v%d not found for project: %s", version_a, project);
            return EXIT_NOT_FOUND;
        }
        if (load_deployment_by_version(project, version_b, &b) != 0) {
            log_error("Version v%d not found for project: %s", version_b, project);
            return EXIT_NOT_FOUND;
        }
    } else {
        // Get latest two deployments by base project name
        if (get_latest_two_by_base(project, &a, &b) != 0) {
            log_error("Need at least 2 deployments of '%s' to diff", project);
            return EXIT_GENERIC;
        }
    }
    
    if (flags.json_out) {
        ret = print_diff_json(&a, &b);
    } else {
        ret = print_diff_human(&a, &b, &flags);
    }
    
    return ret;
}