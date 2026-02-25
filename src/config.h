#ifndef CONFIG_H
#define CONFIG_H
#define FORGE_LOG_PATH "../logs/forge.log"

enum LogLevel{
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_WARN = 1,
	LOG_LEVEL_INFO = 2,
	LOG_LEVEL_DEBUG = 3
};


struct Config{
enum LogLevel log_level;
};
extern struct Config g_config;

#endif
