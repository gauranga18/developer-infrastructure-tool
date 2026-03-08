#ifndef STATE_H
#define STATE_H
#include <time.h>
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
Deployment *state_find_latest(const char *project);
Deployment **state_get_history(const char *project, int *count);
#endif