#ifndef CONFIG_H
#define CONFIG_H
#define FORGE_LOG_PATH "../logs/forge.log"
#define ENV_REGISTRY "FORGE_REGISTRY"
#define ENV_LOG_LEVEL "FORGE_LOG_LEVEL"

enum LogLevel{
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_WARN = 1,
	LOG_LEVEL_INFO = 2,
	LOG_LEVEL_DEBUG = 3
};
void load_env_defaults(void);

struct Config{
enum LogLevel log_level;
char *registry;
char*config_dir;
};
extern struct Config g_config;

#endif
