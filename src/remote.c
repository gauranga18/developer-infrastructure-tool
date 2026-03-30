#include "remote.h"
#include "log.h"
#include "exit_codes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parse_remote(const char *remote_str, RemoteTarget *target) {
    char buffer[256];
    char *at_sign, *colon;
    
    strncpy(buffer, remote_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    target->user = NULL;
    target->port = 22;
    
    at_sign = strchr(buffer, '@');
    if (at_sign) {
        *at_sign = '\0';
        target->user = strdup(buffer);
        strcpy(buffer, at_sign + 1);
    }
    
    colon = strchr(buffer, ':');
    if (colon) {
        *colon = '\0';
        target->port = atoi(colon + 1);
        if (target->port <= 0 || target->port > 65535) {
            log_error("Invalid port number: %s", colon + 1);
            return -1;
        }
    }
    
    target->host = strdup(buffer);
    return 0;
}

int remote_forge_exists(RemoteTarget *target) {
    char cmd[512];
    int result;
    
    if (target->user) {
        snprintf(cmd, sizeof(cmd), "ssh -p %d %s@%s 'test -f /tmp/forge' 2>/dev/null",
                 target->port, target->user, target->host);
    } else {
        snprintf(cmd, sizeof(cmd), "ssh -p %d %s 'test -f /tmp/forge' 2>/dev/null",
                 target->port, target->host);
    }
    
    result = system(cmd);
    return (result == 0);
}

int remote_copy_forge(RemoteTarget *target, const char *forge_path) {
    char cmd[1024];
    
    log_info("Copying forge binary to remote server...");
    
    // Use cat + ssh to copy the binary (works better with password auth)
    if (target->user) {
        snprintf(cmd, sizeof(cmd), "cat %s | ssh -p %d %s@%s 'cat > /tmp/forge && chmod +x /tmp/forge' 2>&1",
                 forge_path, target->port, target->user, target->host);
    } else {
        snprintf(cmd, sizeof(cmd), "cat %s | ssh -p %d %s 'cat > /tmp/forge && chmod +x /tmp/forge' 2>&1",
                 forge_path, target->port, target->host);
    }
    
    if (system(cmd) != 0) {
        log_error("Failed to copy forge binary to remote");
        return -1;
    }
    
    log_info("Forge binary installed on remote at /tmp/forge");
    return 0;
}

int remote_execute(RemoteTarget *target, int argc, char **argv, int show_progress) {
    char cmd[2048];
    char remote_cmd[1024];
    char *forge_path = "/tmp/forge";
    
    remote_cmd[0] = '\0';
    for (int i = 0; i < argc; i++) {
        strcat(remote_cmd, argv[i]);
        if (i < argc - 1) strcat(remote_cmd, " ");
    }
    
    if (target->user) {
        snprintf(cmd, sizeof(cmd), "ssh -p %d %s@%s '%s %s' 2>&1",
                 target->port, target->user, target->host, forge_path, remote_cmd);
    } else {
        snprintf(cmd, sizeof(cmd), "ssh -p %d %s '%s %s' 2>&1",
                 target->port, target->host, forge_path, remote_cmd);
    }
    
    if (show_progress) {
        log_info("Executing remotely: %s", remote_cmd);
    }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        log_error("Failed to execute remote command");
        return -1;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

int handle_remote_command(int argc, char **argv, int show_progress) {
    if (argc < 2) {
        log_error("Usage: forge ssh user@host <command> [options]");
        return EXIT_BAD_ARGS;
    }
    
    char forge_path[512];
    ssize_t len = readlink("/proc/self/exe", forge_path, sizeof(forge_path) - 1);
    if (len == -1) {
        snprintf(forge_path, sizeof(forge_path), "./forge");
    } else {
        forge_path[len] = '\0';
    }
    
    RemoteTarget target;
    if (parse_remote(argv[1], &target) != 0) {
        return EXIT_BAD_ARGS;
    }
    
    log_info("Connecting to remote: %s@%s:%d", 
             target.user ? target.user : "current user", 
             target.host, target.port);
    
    if (!remote_forge_exists(&target)) {
        log_info("Forge not found on remote, copying...");
        if (remote_copy_forge(&target, forge_path) != 0) {
            return EXIT_GENERIC;
        }
    }
    
    int cmd_argc = argc - 2;
    char **cmd_argv = argv + 2;
    
    if (cmd_argc == 0) {
        log_error("No command specified for remote execution");
        return EXIT_BAD_ARGS;
    }
    
    return remote_execute(&target, cmd_argc, cmd_argv, show_progress);
}