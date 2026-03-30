#ifndef REMOTE_H
#define REMOTE_H

typedef struct {
    char *user;
    char *host;
    int port;
} RemoteTarget;

// Parse remote string like "user@host:port" or "user@host" or "host"
int parse_remote(const char *remote_str, RemoteTarget *target);

// Check if forge binary exists on remote
int remote_forge_exists(RemoteTarget *target);

// Copy forge binary to remote
int remote_copy_forge(RemoteTarget *target, const char *forge_path);

// Execute forge command remotely
int remote_execute(RemoteTarget *target, int argc, char **argv, int show_progress);

// SSH remote command handler
int handle_remote_command(int argc, char **argv, int show_progress);

#endif