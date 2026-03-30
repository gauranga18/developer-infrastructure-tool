#ifndef STATE_H
#define STATE_H
#include <time.h>
#include <sys/types.h>

#define STATE_DIR "/.local/state/forge"
#define DEPLOYMENTS_DIR STATE_DIR "/deployments"
#define CURRENT_DIR STATE_DIR "/current"

typedef struct {
    char id[32];
    char project[64];
    char url[256];
    char image_id[128];
    char container_id[128];
    int port;
    time_t deployed_at;
    int version;
    int status;
} Deployment;

int state_init(void);
int state_save(const Deployment *dep);
int list_deployments(void);
int show_status(const char *project);
int rollback_deployment(const char *project);
Deployment *state_find_latest(const char *project);
Deployment **state_get_history(const char *project, int *count);

// Versioning functions
int get_next_version(const char *project);
int update_next_version(const char *project, int new_version);
void build_versioned_name(char *dest, size_t size, const char *project, int version);
void generate_deployment_id(char *id, size_t size, const char *project);

// Internal version - caller must hold lock
int update_next_version_locked(const char *project, int new_version);

// Locking functions
int lock_project(const char *project);
void unlock_project(int fd);
void get_lock_path(char *path, size_t size, const char *project);

#endif